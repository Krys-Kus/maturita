// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "vectors.h"
#include <iostream>
#include <Windows.h>

#define PLAYER_WRITABLE_ANGLES_OFFSET 0x8821F8

// usercmd_t is sent to the server each client frame: https://github.com/OpenArena/legacy/blob/3db79b091ce1d950d9cdcac0445a2134f49a6fc7/engine/openarena-engine-source-0.8.8/code/qcommon/q_shared.h#L1121
typedef struct usercmd_s {
    int				serverTime;
    int				angles[3];
    int 			buttons;
    unsigned char			weapon;        
    signed char	forwardmove, rightmove, upmove;
} usercmd_t;

struct Entity {
    uint8_t padding1[0x14];
    // X, Y, Z
    Vec3 position; // Offset 0x14
    uint8_t padding2[0x84 - 0x14 - sizeof(position)];
    int health; // Offset 0x84
    uint8_t padding3[0x98 - 0x84 - sizeof(health)];
    // Pitch, Yaw
    float angles[2]; // Offset 0x98
    uint8_t padding4[0x104 - 0x98 - sizeof(angles)];
    uint8_t team; // Offset 0x104
};

const DWORD baseAddress = 0x00400000;
const DWORD ENTITY_LIST_PTR_OFFSET = 0x01B4BB44;
Entity* entityList;
Entity* player;

usercmd_t* usercmd;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Function to calculate the yaw angle from a directional vector
float CalculateYaw(const Vec3& direction) {
    // Yaw is the angle in the X-Y plane (horizontal plane)
    return std::atan2(direction.y, direction.x) * (180.0f / M_PI); // Convert radians to degrees
}

// Function to calculate the pitch angle from a directional vector
float CalculatePitch(const Vec3& direction) {
    // Pitch is the angle in the vertical plane
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    return std::atan2(-direction.z, horizontalDistance) * (180.0f / M_PI); // Convert radians to degrees


}

void installHook(void* target, void* hook, DWORD* jumpBackAddress, int size) {
    DWORD oldProtect;
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    //32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
    uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

    //to fill out the last 4 bytes of jmpInstruction, we need the offset between 
    //the payload function and the instruction immediately AFTER the jmp instruction
    DWORD relAddr = (DWORD)hook - ((DWORD)target + sizeof(jmpInstruction));

    if (jumpBackAddress != NULL) {
        *jumpBackAddress = (DWORD)target + size;
    }

    memcpy(jmpInstruction + 1, &relAddr, 4);

    //install the hook
    memcpy(target, jmpInstruction, sizeof(jmpInstruction));

	VirtualProtect(target, 5, oldProtect, &oldProtect);
}

void __declspec(naked) CL_CreateCmd_hook() {
    __asm {
        mov usercmd, eax
        PUSHFD
        PUSHAD
    }

    __asm {
        POPAD
        POPFD
        pop edi
        pop ebp
        ret 0x4
    }
}

void mainloop() {

    //Force OA to open console
    AllocConsole();
    static FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    entityList = *(Entity**)(baseAddress + ENTITY_LIST_PTR_OFFSET);
    player = entityList + 0;

    // At the end of CL_CreateCmd, before the new usercmd is returned
    void* CL_CreateCmd_end = (void*)(baseAddress + 0x00010722);

	installHook(CL_CreateCmd_end, CL_CreateCmd_hook, nullptr, 5);

    while (true) {

        // Variables to track the closest enemy
        float closestDistance = FLT_MAX; // Initialize with a large value
        int closestEnemyIndex = -1;
        Vec3 closestEnemyPosition = { 0, 0, 0 };

        for (int i = 1; i < 16; i++) {
            Entity* entity = (Entity*)((DWORD)entityList + i * 0x840);
			if (entity->team == 0) {
				continue;
			}
			if (entity->team == player->team) {
				continue;
			}

            //print entity team
            std::cout << "Entity " << i << " Team: " << (int)entity->team << std::endl;

            //print entity position
            std::cout << "Entity " << i << " Position: "
                << "X = " << entity->position.x << ", "
                << "Y = " << entity->position.y << ", "
                << "Z = " << entity->position.z << std::endl;

            //print entity angles
            std::cout << "Entity " << i << " Angles: "
                << "Pitch = " << entity->angles[0] << ", "
                << "Yaw = " << entity->angles[1] << std::endl;

            //print entity health
            std::cout << "Entity " << i << " Health: " << entity->health << std::endl;



            float distance = entity->position.distance(player->position);

            // Check if this is the closest enemy
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEnemyIndex = i;
                closestEnemyPosition = entity->position;

                std::cout << "Closest Enemy: " << i << std::endl;
            }
        }

        // Print the closest enemy's information
        if (closestEnemyIndex != -1) {
            // Calculate the directional vector to the closest enemy
            Vec3 directionToEnemy = closestEnemyPosition - player->position;

            // Calculate yaw and pitch angles
            float yawToEnemy = CalculateYaw(directionToEnemy);
            float pitchToEnemy = CalculatePitch(directionToEnemy);

            std::cout << "Closest Enemy: Bot " << closestEnemyIndex << std::endl;
            std::cout << "Closest Enemy Position: "
                << "X = " << closestEnemyPosition.x << ", "
                << "Y = " << closestEnemyPosition.y << ", "
                << "Z = " << closestEnemyPosition.z << std::endl;
            std::cout << "Yaw to Enemy: " << yawToEnemy << " degrees" << std::endl;
            std::cout << "Pitch to Enemy: " << pitchToEnemy << " degrees" << std::endl;

            *(float*)(0x400000 + PLAYER_WRITABLE_ANGLES_OFFSET) += pitchToEnemy - player->angles[0];
            player->angles[0] = pitchToEnemy;
            *(float*)(0x400000 + PLAYER_WRITABLE_ANGLES_OFFSET + 0x4) += yawToEnemy - player->angles[1];
            player->angles[1] = yawToEnemy;
   
        }
    }
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainloop, nullptr, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

