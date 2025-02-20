#pragma once
#include <iostream>
#include <windows.h>
#include <psapi.h>

template <typename T>
T ReadMemory(HANDLE hProcess, DWORD address) {
    T value;
    ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
};

uintptr_t GetBaseAddress(DWORD processId);