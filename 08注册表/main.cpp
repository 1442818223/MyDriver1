#include <ntddk.h>

// 定义要创建的注册表键路径
#define REGISTRY_PATH L"\\Registry\\Machine\\Software\\MyApp"  //对应HKEY_LOCAL_MACHINE\SOFTWARE\MyApp
// 定义要创建的值的名称和数据
#define VALUE_NAME L"version"
#define VALUE_DATA 12

VOID UnloadDriver(PDRIVER_OBJECT Pdriver) {
    DbgPrintEx(77, 0, "卸载成功\n");
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
    pDriver->DriverUnload = UnloadDriver;

        NTSTATUS status;
    UNICODE_STRING registryPath;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE registryKey;
    UNICODE_STRING valueName;
    ULONG valueData;
    PKEY_VALUE_PARTIAL_INFORMATION  pKeyinfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, 0x10, 'info');
    ULONG queryLen;
    ULONG relqueryLen;

    // 初始化注册表路径
    RtlInitUnicodeString(&registryPath, REGISTRY_PATH);
    // 初始化对象属性                                              大小写不敏感
    InitializeObjectAttributes(&objectAttributes, &registryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
    // 创建或打开注册表键
    status = ZwCreateKey(&registryKey,//指向用于接收创建的键的句柄的指针。
        KEY_ALL_ACCESS, //指定对新创建的键的访问权限。
        &objectAttributes,//指向 OBJECT_ATTRIBUTES 结构的指针，其中包含有关键的对象属性，例如键名、对象名和安全描述符等。
        0, NULL,
        REG_OPTION_NON_VOLATILE,//非易失性,表示创建了一直在
        NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

/*
写注册表项
*/
    // 初始化值名称
    RtlInitUnicodeString(&valueName, VALUE_NAME);
    // 设置整数值数据
    valueData = VALUE_DATA;
    // 写入整数值到注册表
    status = ZwSetValueKey(registryKey, &valueName, 0, REG_DWORD, &valueData, sizeof(valueData));
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "set successfully!\n");
    }

/*
读注册表项
*/
// 初次查询获取储存所需的缓冲区大小
    status = ZwQueryValueKey(registryKey,//注册表键的句柄
        &valueName, //注册表值的名称
        KeyValuePartialInformation, //要查询的信息类别,此处为:查询指定值的部分信息
        pKeyinfo,//接收查询结果的缓冲区指针
        sizeof(KEY_VALUE_PARTIAL_INFORMATION),
        &queryLen);                                       //返回实际查询结果的长度
    DbgPrintEx(77, 0, "[statu1]:%d\n", status);
    DbgPrintEx(77, 0, "[query1]%d\n", *(PUCHAR)(pKeyinfo->Data));
    // 释放防止内存泄露
    ExFreePoolWithTag(pKeyinfo, 'info');

    // 申请符合所需大小的内存块
    PKEY_VALUE_PARTIAL_INFORMATION pKeyinfo2 = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, queryLen, 'info');
    if (pKeyinfo)
    {
        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)//缓冲区过小或过大
        {
            relqueryLen = queryLen;
            status = ZwQueryValueKey(registryKey,
                &valueName,
                KeyValuePartialInformation,
                pKeyinfo2,
                relqueryLen,
                &queryLen);
            DbgPrintEx(77, 0, "[statu2]:%d\n", status);
            if (NT_SUCCESS(status))
            {
                DbgPrintEx(77, 0, "[query2]%d\n", *(PUCHAR)(pKeyinfo2->Data));
            }
            else
            {
                DbgPrintEx(77, 0, "Wrong!");
            }

        }
    }

/*
删注册表项
*/
    status = ZwDeleteKey(registryKey);
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Handle deleted!\n");
    }



    // 关闭注册表键
    status = ZwClose(registryKey);
    if (NT_SUCCESS(status))
    {
        DbgPrintEx(77, 0, "Handle closed!\n");
    }

    return 0;
}