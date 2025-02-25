#include <iostream>
#include <Windows.h>

int main()
{
	const char* dll_path = "C:\\Users\\User\\source\\repos\\maturita\\MaturitaOA\\Debug\\OADLL.dll";

	HWND hwnd = FindWindowA(NULL, "OpenArena");
	if (hwnd == NULL) {
		std::cout << "Window not found\n";
		return 1;
	}
	else
	{
		std::cout << "Window handle: " << hwnd << "\n";
	}

	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0) {
		std::cout << "Process ID not found\n";
		return 1;
	}
	else
	{
		std::cout << "Process ID: " << pid << "\n";
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		std::cout << "Process handle not found\n";
		return 1;
	}
	else {
		std::cout << "Process handle: " << hProcess << "\n";
	}

	HMODULE kernel32_handle = GetModuleHandleA("kernel32.dll");
	if (kernel32_handle == NULL) {
		std::cout << "Kernel32 handle not found\n";
		return 1;
	}
	else {
		std::cout << "Kernel32 handle: " << kernel32_handle << "\n";
	}

	LPVOID load_library_address = (LPVOID)GetProcAddress(kernel32_handle, "LoadLibraryA");
	if (load_library_address == NULL) {
		std::cout << "LoadLibraryA address not found\n";
		return 1;
	}
	else {
		std::cout << "LoadLibraryA address: " << load_library_address << "\n";
	}

	LPVOID arg = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL) {
		std::cout << "Memory not allocated\n";
		return 1;
	}
	else {
		std::cout << "Memory allocated: " << arg << "\n";
	}

	WriteProcessMemory(hProcess, arg, dll_path, strlen(dll_path) + 1, NULL);

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, arg, 0, NULL);
	if (hThread == NULL) {
		std::cout << "Thread not created\n";
		return 1;
	}
	else {
		std::cout << "Thread created: " << hThread << "\n";
	}

	WaitForSingleObject(hThread, INFINITE);

	VirtualFreeEx(hProcess, arg, 0, MEM_RELEASE);

	CloseHandle(hProcess);

	CloseHandle(hThread);

	std::cout << "Injected\n";
	return 0;
}