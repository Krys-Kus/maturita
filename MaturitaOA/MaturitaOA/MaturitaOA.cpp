// Maturita_OA.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>

// Pomocná funkce pro čtení paměti
template <typename T>
T ReadMemory(HANDLE hProcess, DWORD address) {
    T value;
    ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}
bool IsEnemy(int myTeam, int otherTeam) {
    // Ignorovat prázdné sloty (0)
    if (otherTeam == 0) {
        return false;
    }
    // Hráče s jiným tým value je nepřítel
    return otherTeam != myTeam;
}

int main()
{
    //td::cout << "Hello World!\n";

    DWORD processID = 0;
    HWND hwnd = FindWindowA(nullptr, "OpenArena");
    if (hwnd == nullptr) {
        std::cerr << "OpenArena window not found!" << std::endl;
        return 1;
    }
    GetWindowThreadProcessId(hwnd, &processID);

    // Otevření procesu
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    if (hProcess == nullptr) {
        std::cerr << "Failed to open process!" << std::endl;
        return 1;
    }

    // Čtení paměti
    DWORD baseTeamAddress = 0x0D97B4A8; // Adresa týmu
    DWORD offsetBetweenBots = 0x840; //Offset mezi boty

    // Read your team value
    int myTeam = ReadMemory<int>(hProcess, baseTeamAddress);
    std::cout << "Your Team Value: " << myTeam << std::endl;


    // Skenovat paměť pro 15 botů/hráčů, základní maximální počet hráčů v OA je 16
    for (int i = 0; i < 15; i++) {
        DWORD botTeamAddress = baseTeamAddress + (i)*offsetBetweenBots;
        int teamValue = ReadMemory<unsigned char>(hProcess, botTeamAddress);
        int botTeamValue = ReadMemory<int>(hProcess, botTeamAddress);

        // Zjistit, zda je bot enemy, ally nebo prázdný slot
        if (IsEnemy(myTeam, botTeamValue)) {
            std::cout << "Bot " << (i + 1) << " is an ENEMY! Team Value: " << botTeamValue << std::endl;
        }
        else if (botTeamValue != 0) {
            std::cout << "Bot " << (i + 1) << " is an ALLY. Team Value: " << botTeamValue << std::endl;
        }
        else {
            std::cout << "Bot " << (i + 1) << " is an EMPTY SLOT. Team Value: " << botTeamValue << std::endl;
        }
    }


    // Clean up
    CloseHandle(hProcess);
    return 0;
}




// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file