#include <windows.h>
#include <aclapi.h>
#pragma comment(lib, "Advapi32.lib")


void SetFilePermissions(LPCWSTR filePath)
{
	// 定义 SID 和 ACL
	PSID pSid = NULL;
	PACL pAcl = NULL;
	EXPLICIT_ACCESS ea = { 0 };
	SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;

	// 为 SYSTEM 账户创建 SID
	AllocateAndInitializeSid(&authority, 1, SECURITY_LOCAL_SYSTEM_RID,
		0, 0, 0, 0, 0, 0, 0, &pSid);

	// 配置 ACL 条目，允许 SYSTEM 读写
	ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea.Trustee.ptstrName = (LPTSTR)pSid;

	// 创建 ACL
	SetEntriesInAcl(1, &ea, NULL, &pAcl);

	// 应用安全描述符
	SetNamedSecurityInfo((LPWSTR)filePath, SE_FILE_OBJECT,
		DACL_SECURITY_INFORMATION, NULL, NULL, pAcl, NULL);

	// 清理
	if (pSid) FreeSid(pSid);
	if (pAcl) LocalFree(pAcl);
}

int main()
{
	SetFilePermissions(L"C:\\path\\to\\your\\driver.sys");
	return 0;
}
