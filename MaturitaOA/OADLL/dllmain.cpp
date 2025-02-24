// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include "vectors.h"

#define PLAYER_WRITABLE_ANGLES_OFFSET 0x8821F8

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

void mainloop() {

    //Force OA to open console
    AllocConsole();
    static FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    entityList = *(Entity**)(baseAddress + ENTITY_LIST_PTR_OFFSET);
    player = entityList + 0;

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

