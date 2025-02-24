// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>

struct Entity {
    uint8_t padding1[0x14];
    // X, Y, Z
    float position[3]; // Offset 0x14
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

void mainloop() {
	
    //Force OA to open console
    AllocConsole();
    static FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    entityList = *(Entity**)(baseAddress + ENTITY_LIST_PTR_OFFSET);

	while (true) {
        for (int i = 1; i < 16; i++) {
            Entity* entity = (Entity*)((DWORD)entityList + i * 0x840);
        
			//print entity team
			std::cout << "Entity " << i << " Team: " << (int)entity->team << std::endl;

			//print entity position
			std::cout << "Entity " << i << " Position: "
				<< "X = " << entity->position[0] << ", "
				<< "Y = " << entity->position[1] << ", "
				<< "Z = " << entity->position[2] << std::endl;

			//print entity angles
			std::cout << "Entity " << i << " Angles: "
				<< "Pitch = " << entity->angles[0] << ", "
				<< "Yaw = " << entity->angles[1] << std::endl;

			//print entity health
			std::cout << "Entity " << i << " Health: " << entity->health << std::endl;

         
        }
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
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

