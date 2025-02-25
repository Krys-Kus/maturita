#include <iostream>
#include <windows.h>


bool IsEnemy(int myTeam, int otherTeam);

struct vec3_t {
    float x, y, z;
};

vec3_t ReadPosition(HANDLE hProcess, DWORD xAddress);
