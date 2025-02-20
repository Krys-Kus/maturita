#include <iostream>
#include <windows.h>


bool IsEnemy(int myTeam, int otherTeam);

// Structure to store the player's position
struct vec3_t {
    float x, y, z;
};

// Function to read the player's position from memory
vec3_t ReadPosition(HANDLE hProcess, DWORD xAddress);
