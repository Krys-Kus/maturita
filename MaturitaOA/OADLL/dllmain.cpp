//this is the internal solution utlizing almost the same code for the aimbot as the external one but the wallhack is going to be built only here

#include "pch.h"
#include "vectors.h"
#include <iostream>
#include <Windows.h>
#include <gl/GL.h>

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
    unsigned char	weapon;        
    signed char	    forwardmove, rightmove, upmove;
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

bool aimbot_enabled = true;
bool triggerbot_enabled = false;
bool wallhack_enabled = true;

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

void drawBoundingBox(float x, float y, float width, float height, float red, float green, float blue) {
    // saves the current opengl satate
    glPushAttrib(GL_ALL_ATTRIB_BITS); // saves all attributes like color andline width
    glPushMatrix();  // saves the current transformation matrices 

    // set state to what we need
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);

    // draw a box outline
    glBegin(GL_LINE_LOOP);
    glColor3f(red, green, blue);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // restore the saved opengl state
    glPopMatrix();
    glPopAttrib();  
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
	if (GetAsyncKeyState(VK_F1) & 1) {
		aimbot_enabled = !aimbot_enabled;
	}

	if (!aimbot_enabled) {
		return;
	}
    
    // Variables to track the closest enemy
    float closestDistance = FLT_MAX; 
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
			directionToEnemy.z -= 15.0f; //offset the calculation a bit to aim at the chest instead of the head so we can still hit enemies if they crouch

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

        }
}

void triggerbot() {
    if (GetAsyncKeyState(0x46) & 1) { //for some reason F key and all other letters are stored as hexadecimals
        triggerbot_enabled = !triggerbot_enabled;
    }

    if (!triggerbot_enabled) {
        return;
    }

    usercmd->buttons |= (BUTTON_ANY | BUTTON_ATTACK);
    
}

//calling convention that will prevent the compiler from adding instrunctions at the start and end of the function that would mess with the registers
void __declspec(naked) CL_CreateCmd_hook() { 
    __asm {
        mov usercmd, eax
		PUSHFD //push saves the state of the registers
        PUSHAD
    }

	aimbot();
	triggerbot();

    __asm {
		POPAD //pop restores the state of the registers
        POPFD
        pop edi
        pop ebp
        ret 0x4
    }
}

void wallhack() {
    if (GetAsyncKeyState(VK_F2) & 1) {
        wallhack_enabled = !wallhack_enabled;
    }

    if (!wallhack_enabled) {
        return;
    }

    for (int i = 1; i < 16; i++) {
        Entity* entity = (Entity*)((DWORD)entityList + i * 0x840);
        if (entity->team == 0) {
            continue;
        }

        if (entity->health <= 0) {
            continue;
        }

        Vec3 directionToEntity = entity->position - player->position;

        float yawToEntity = CalculateYaw(directionToEntity) - player->angles[1];
        if (yawToEntity > 180) {
            yawToEntity -= 360;
        }
        else if (yawToEntity < -180) {
            yawToEntity += 360;
        }

        float pitchToEntity = CalculatePitch(directionToEntity) - player->angles[0];

        float FOV = 50.0f;

        float x = (-yawToEntity / FOV); //opengl uses 1 going up and to the left while open arena uses -1 going up and to the left
        float y = (-pitchToEntity / ((3.0f / 4.0f) * FOV)); //specific for the aspect ratio 4:3

         
        float scale = 100.0f / (directionToEntity.length() + 50.0f);
		
        float width = 0.5f * scale;
		float height = 2.0f * width;

		//rectangles were drawn even if a player was out of screen so this checks if the player is on the screen
        //since the pitch and yaw ToEntiy are translated into a coordinate system ranging from -1 to 1
        if (x + width/2 < -1 || x - width/2 > 1 || y + height*0.15 < -1 || y - height*0.85 > 1) {
            continue;
        }

		std::cout << "Entity " << i << " Position: "
			<< "X = " << entity->position.x << ", "
			<< "Y = " << entity->position.y << ", "
			<< "Z = " << entity->position.z << std::endl;

		std::cout << "Player " << i << " Position: " 
            << "X = " << player->position.x << ", "
            << "Y = " << player->position.y << ", "
            << "Z = " << player->position.z << std::endl;

		std::cout << yawToEntity << std::endl;
		std::cout << pitchToEntity << std::endl;
		

        //x and y are the position of player's head by default and boudning box is scaled from the left bottom corner to the top right corner so we have to adjust
        if (entity->team == player->team) {
			drawBoundingBox(x - width / 2, y - height * 0.85, width, height, 0.0, 0.0, 1.0); 
        }
        else {
            drawBoundingBox(x - width / 2, y - height * 0.85, width, height, 1.0, 0.0, 0.0);
        }

    }
}

DWORD SDL_GL_SwapBuffersJumpBackAddress = 0x123;
void __declspec(naked) SDL_GL_SwapBuffers_hook() {
    __asm {
        PUSHFD
        PUSHAD
    }

	wallhack();

    __asm {
        POPAD
        POPFD
        push ebp
        mov ebp, esp
        sub esp, 0x8
        jmp[SDL_GL_SwapBuffersJumpBackAddress]
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

    void* SDL_GL_SwapBuffers = GetProcAddress(GetModuleHandleA("SDL.dll"), "SDL_GL_SwapBuffers");

    installHook(SDL_GL_SwapBuffers, SDL_GL_SwapBuffers_hook, &SDL_GL_SwapBuffersJumpBackAddress, 6);
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

