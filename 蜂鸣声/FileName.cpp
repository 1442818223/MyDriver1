#include <windows.h>
#include <pdh.h>
#include <stdio.h>

#pragma comment(lib, "Pdh.lib")  // 自动链接 Pdh.lib

int main() {
	PDH_HQUERY query;
	PDH_HCOUNTER counter;
	PDH_FMT_COUNTERVALUE counterVal;
	const wchar_t* processName = L"QQMusic";  // 需要监控的进程名称 (不带扩展名)

	// 创建查询
	if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) {
		printf("Failed to open query.\n");
		return 1;
	}

	// 构建进程 CPU 使用率计数器路径
	wchar_t counterPath[256];
	swprintf(counterPath, 256, L"\\Process(%s)\\%% Processor Time", processName);

	// 添加进程 CPU 使用率计数器
	if (PdhAddCounter(query, counterPath, 0, &counter) != ERROR_SUCCESS) {
		printf("Failed to add counter for process %ls.\n", processName);
		PdhCloseQuery(query);
		return 1;
	}

	// 初始收集性能数据
	PdhCollectQueryData(query);

	// 无限循环，每秒收集并打印进程 CPU 使用率
	while (1) {
		// 等待 1 秒
		Sleep(1000);

		// 再次收集性能数据
		PdhCollectQueryData(query);

		// 获取格式化的 CPU 使用率数据
		if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
			
			double cpuUsage = counterVal.doubleValue;

			printf("%ls CPU Usage: %.2f%%\n", processName, cpuUsage);
		}
		else {
			printf("Failed to get formatted counter value.\n");
		}
	}

	// 关闭查询 (此行永远不会执行，因为程序处于无限循环中)
	PdhCloseQuery(query);

	return 0;
}
