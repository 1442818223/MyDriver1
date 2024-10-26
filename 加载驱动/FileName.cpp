#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 获取当前工作目录
	char currentPath[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, currentPath) == 0) {
		printf("获取当前目录失败\n");
		return 1;
	}

	// 构造驱动程序的完整路径
	char driverPath[MAX_PATH];
	snprintf(driverPath, sizeof(driverPath), "%s\\12356.sys", currentPath); // 假设驱动程序文件名为 12356.sys
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 停止服务的命令
	const char* stopCmd = "sc stop MyDriver";  // 修改为 MyDriver
	// 删除服务的命令
	const char* deleteCmd = "sc delete MyDriver"; // 修改为 MyDriver

	// 执行停止服务命令
	int resultStop = system(stopCmd);
	if (resultStop != 0) {
		printf("停止服务失败\n");
	}

	// 执行删除服务命令
	int resultDelete = system(deleteCmd);
	if (resultDelete != 0) {
		printf("删除服务失败\n");
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 创建服务命令
	char createCmd[512];
	snprintf(createCmd, sizeof(createCmd), "sc create MyDriver binPath= \"%s\" type= kernel start= demand", driverPath);
	int resultCreate = system(createCmd);
	if (resultCreate != 0) {
		printf("创建服务失败\n");
		return resultCreate;
	}

	// 启动服务命令
	const char* startCmd = "sc start MyDriver";
	int resultStart = system(startCmd);
	if (resultStart != 0) {
		printf("启动服务失败\n");
		return resultStart;
	}

	printf("驱动程序已成功加载并启动。\n");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HKEY hKey;
	LONG result;

	// 打开服务的注册表项
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\Services\\MyDriver"),
		0,
		KEY_SET_VALUE,
		&hKey);

	if (result != ERROR_SUCCESS) {
		printf("无法打开注册表项，错误代码：%ld\n", result);
		return 1;
	}

	// 设置 Start 值为 1
	DWORD startValue = 1; // 1 表示自动启动
	result = RegSetValueEx(hKey,
		TEXT("Start"),
		0,
		REG_DWORD,
		(const BYTE*)&startValue,
		sizeof(startValue));

	if (result != ERROR_SUCCESS) {
		printf("无法设置 Start 值，错误代码：%ld\n", result);
		RegCloseKey(hKey);
		return 1;
	}

	printf("Start 值已成功设置为 1。\n");

	// 关闭注册表项
	RegCloseKey(hKey);


	system("pause");
	return 0;
}
/*
@echo off

rem 切换到脚本所在的目录
cd /d "%~dp0"

rem 检查服务是否存在
sc query MyDriver >nul 2>&1
if %errorlevel%==0 (
	echo 驱动程序服务已存在，正在停止和删除服务...

	rem 停止服务（如果存在）
	sc stop MyDriver

	rem 删除已存在的服务（如果存在）
	sc delete MyDriver
) else (
	echo 驱动程序服务不存在，无需卸载。
)

rem 创建驱动程序服务
sc create MyDriver binPath= "%~dp012356.sys" type= kernel start= demand

rem 启动服务
sc start MyDriver

pause
*/