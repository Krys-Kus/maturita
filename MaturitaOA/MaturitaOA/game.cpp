#include "game.h"
#include <iostream>
#include <windows.h>


bool IsEnemy(int myTeam, int otherTeam) {
    if (otherTeam == 0) {
        return false;
    }
    return otherTeam != myTeam;
}

vec3_t ReadPosition(HANDLE hProcess, DWORD xAddress) {
    vec3_t position;
    ReadProcessMemory(hProcess, (LPCVOID)xAddress, &position.x, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(xAddress + 4), &position.y, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(xAddress + 8), &position.z, sizeof(float), nullptr);
    return position;
}
