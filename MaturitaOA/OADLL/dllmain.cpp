//this is the internal solution utlizing almost the same code for the aimbot as the external one but the wallhack is going to be built only here

#include "pch.h"
#include "vectors.h"
#include <iostream>
#include <Windows.h>

#define ENTITY_LIST_PTR_OFFSET 0x01B4BB44
#define PLAYER_WRITABLE_ANGLES_OFFSET 0x8821F8

#define    ANGLE2SHORT(x) ((int)((x)*65536/360) & 65535)
#define    SHORT2ANGLE(x) ((x)*(360.0/65536))

#define    BUTTON_ATTACK 1
#define    BUTTON_ANY 2048

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

Entity* entityList;
Entity* player;

usercmd_t* usercmd;

bool should_fire = false;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float CalculateYaw(const Vec3& direction) {
    return std::atan2(direction.y, direction.x) * (180.0f / M_PI); // Convert radians to degrees
}

float CalculatePitch(const Vec3& direction) {
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    return std::atan2(-direction.z, horizontalDistance) * (180.0f / M_PI); 


}

void installHook(void* target, void* hook, DWORD* jumpBackAddress, int size) {
    DWORD oldProtect;
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    //32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
    uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

    //to fill out the last 4 bytes of jmpInstruction, we need the offset between the payload function and the instruction immediately AFTER the jmp instruction
    DWORD relAddr = (DWORD)hook - ((DWORD)target + sizeof(jmpInstruction));

    if (jumpBackAddress != NULL) {
        *jumpBackAddress = (DWORD)target + size;
    }

    memcpy(jmpInstruction + 1, &relAddr, 4);

    //install the hook
    memcpy(target, jmpInstruction, sizeof(jmpInstruction));

	VirtualProtect(target, 5, oldProtect, &oldProtect);
}

void aimbot() { 
    
    // Variables to track the closest enemy
    float closestDistance = FLT_MAX; 
    int closestEnemyIndex = -1;
    Vec3 closestEnemyPosition = { 0, 0, 0 };

    if (GetAsyncKeyState(VK_LCONTROL) & 1) {
        should_fire = !should_fire;
    }
    
    for (int i = 1; i < 16; i++) {
            Entity* entity = (Entity*)((DWORD)entityList + i * 0x840);
			if (entity->team == 0) {
				continue;
			}
			if (entity->team == player->team) {
				continue;
			}
			if (entity->health <= 0) {
				continue;
			}

            std::cout << "Entity " << i << " Team: " << (int)entity->team << std::endl;

            std::cout << "Entity " << i << " Position: "
                << "X = " << entity->position.x << ", "
                << "Y = " << entity->position.y << ", "
                << "Z = " << entity->position.z << std::endl;

            std::cout << "Entity " << i << " Angles: "
                << "Pitch = " << entity->angles[0] << ", "
                << "Yaw = " << entity->angles[1] << std::endl;

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

        if (closestEnemyIndex != -1) {
           
            Vec3 directionToEnemy = closestEnemyPosition - player->position;

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

            usercmd->angles[0] = ANGLE2SHORT(*(float*)(0x400000 + PLAYER_WRITABLE_ANGLES_OFFSET));
            usercmd->angles[1] = ANGLE2SHORT(*(float*)(0x400000 + PLAYER_WRITABLE_ANGLES_OFFSET + 0x4));

            if (should_fire) {
                usercmd->buttons |= (BUTTON_ANY | BUTTON_ATTACK);
            }
   
        }
}

//calling convention that will prevent the compiler from adding instrunctions at the start and end of the function that would mess with the registers
void __declspec(naked) CL_CreateCmd_hook() { 
    __asm {
        mov usercmd, eax
		PUSHFD //push saves the state of the registers
        PUSHAD
    }

	aimbot();

    __asm {
		POPAD //pop restores the state of the registers
        POPFD
        pop edi
        pop ebp
        ret 0x4
    }
}

void hack() {

    //This forces open arena to open its own console window where all the output is printed
    AllocConsole();
    static FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    entityList = *(Entity**)(baseAddress + ENTITY_LIST_PTR_OFFSET);
    player = entityList + 0;

    // At the end of CL_CreateCmd, before the new usercmd is returned
    void* CL_CreateCmd_end = (void*)(baseAddress + 0x00010722);

	installHook(CL_CreateCmd_end, CL_CreateCmd_hook, nullptr, 5);

}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)hack, nullptr, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

