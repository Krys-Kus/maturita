#include "game.h"
#include <iostream>
#include <windows.h>


bool IsEnemy(int myTeam, int otherTeam) {
    // Ignorovat prázdné sloty (0)
    if (otherTeam == 0) {
        return false;
    }
    // Hráèe s jiným tým value je nepøítel
    return otherTeam != myTeam;
}
// Structure to store the player's position

// Function to read the player's position from memory
vec3_t ReadPosition(HANDLE hProcess, DWORD xAddress) {
    vec3_t position;
    // Read X, Y, and Z coordinates
    ReadProcessMemory(hProcess, (LPCVOID)xAddress, &position.x, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(xAddress + 4), &position.y, sizeof(float), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(xAddress + 8), &position.z, sizeof(float), nullptr);
    return position;
}
