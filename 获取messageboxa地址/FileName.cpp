#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	HMODULE hmodule = LoadLibraryA("user32.dll");
	if (hmodule == NULL) {
		printf("Failed to load user32.dll\n");
		return 1;
	}

	ULONG_PTR msg = (ULONG_PTR)GetProcAddress(hmodule, "MessageBoxA");
	if (msg == 0) {
		printf("Failed to get address of MessageBoxA\n");
		FreeLibrary(hmodule);
		return 1;
	}

	printf("Address of MessageBoxA: %p\n", (PVOID)msg);

	// Pause the program
	printf("Press Enter to continue...");
	getchar();

	FreeLibrary(hmodule);
	return 0;
}
