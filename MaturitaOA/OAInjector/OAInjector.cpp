#include <iostream>
#include <Windows.h>

int main()
{
	const char* dll_path = "C:\\Users\\User\\source\\repos\\maturita\\MaturitaOA\\Debug\\OADLL.dll";

	// Get handle from window name
	HWND hwnd = FindWindowA(NULL, "OpenArena");
	if (hwnd == NULL) {
		std::cout << "Window not found\n";
		return 1;
	}
	else
	{
		std::cout << "Window handle: " << hwnd << "\n";
	}

	// Get process ID from window handle
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

	// Get handle to process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		std::cout << "Process handle not found\n";
		return 1;
	}
	else {
		std::cout << "Process handle: " << hProcess << "\n";
	}

	// Get handle to kernel32.dll
	HMODULE kernel32_handle = GetModuleHandleA("kernel32.dll");
	if (kernel32_handle == NULL) {
		std::cout << "Kernel32 handle not found\n";
		return 1;
	}
	else {
		std::cout << "Kernel32 handle: " << kernel32_handle << "\n";
	}

	// Get address of LoadLibraryA
	LPVOID load_library_address = (LPVOID)GetProcAddress(kernel32_handle, "LoadLibraryA");
	if (load_library_address == NULL) {
		std::cout << "LoadLibraryA address not found\n";
		return 1;
	}
	else {
		std::cout << "LoadLibraryA address: " << load_library_address << "\n";
	}

	// Allocate memory in process
	LPVOID arg = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(dll_path) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (arg == NULL) {
		std::cout << "Memory not allocated\n";
		return 1;
	}
	else {
		std::cout << "Memory allocated: " << arg << "\n";
	}

	// Write path to memory
	WriteProcessMemory(hProcess, arg, dll_path, strlen(dll_path) + 1, NULL);

	// Create remote thread
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_address, arg, 0, NULL);
	if (hThread == NULL) {
		std::cout << "Thread not created\n";
		return 1;
	}
	else {
		std::cout << "Thread created: " << hThread << "\n";
	}

	// Wait for thread to finish
	WaitForSingleObject(hThread, INFINITE);

	// Free memory
	VirtualFreeEx(hProcess, arg, 0, MEM_RELEASE);

	// Close handle to process
	CloseHandle(hProcess);

	// Close handle to thread
	CloseHandle(hThread);

	std::cout << "Injected\n";
	return 0;
}