#include <windows.h>
#include <pdh.h>
#include <stdio.h>

#pragma comment(lib, "Pdh.lib")  // �Զ����� Pdh.lib

int main() {
	PDH_HQUERY query;
	PDH_HCOUNTER counter;
	PDH_FMT_COUNTERVALUE counterVal;
	const wchar_t* processName = L"QQMusic";  // ��Ҫ��صĽ������� (������չ��)

	// ������ѯ
	if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) {
		printf("Failed to open query.\n");
		return 1;
	}

	// �������� CPU ʹ���ʼ�����·��
	wchar_t counterPath[256];
	swprintf(counterPath, 256, L"\\Process(%s)\\%% Processor Time", processName);

	// ��ӽ��� CPU ʹ���ʼ�����
	if (PdhAddCounter(query, counterPath, 0, &counter) != ERROR_SUCCESS) {
		printf("Failed to add counter for process %ls.\n", processName);
		PdhCloseQuery(query);
		return 1;
	}

	// ��ʼ�ռ���������
	PdhCollectQueryData(query);

	// ����ѭ����ÿ���ռ�����ӡ���� CPU ʹ����
	while (1) {
		// �ȴ� 1 ��
		Sleep(1000);

		// �ٴ��ռ���������
		PdhCollectQueryData(query);

		// ��ȡ��ʽ���� CPU ʹ��������
		if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
			
			double cpuUsage = counterVal.doubleValue;

			printf("%ls CPU Usage: %.2f%%\n", processName, cpuUsage);
		}
		else {
			printf("Failed to get formatted counter value.\n");
		}
	}

	// �رղ�ѯ (������Զ����ִ�У���Ϊ����������ѭ����)
	PdhCloseQuery(query);

	return 0;
}
