//This is the external solution for aimbot and wallhack, I had to abondon this since the wallhack was way too complicated to even at it's most basic parts

#include <iostream>
#include <cmath>
#include <windows.h>
#include <tlhelp32.h>
#include "game.h"
#include "memory.h"   

float CalculateDistance(const vec3_t& pos1, const vec3_t& pos2) {
	return std::sqrt(
		std::pow(pos2.x - pos1.x, 2) +
		std::pow(pos2.y - pos1.y, 2) +
		std::pow(pos2.z - pos1.z, 2)
	);
}

vec3_t CalculateDirectionalVector(const vec3_t& pos1, const vec3_t& pos2) {
	vec3_t direction;
	direction.x = pos2.x - pos1.x;
	direction.y = pos2.y - pos1.y;
	direction.z = pos2.z - pos1.z;
	return direction;
}

vec3_t NormalizeVector(const vec3_t& vec) {
	float length = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	vec3_t normalized;
	normalized.x = vec.x / length;
	normalized.y = vec.y / length;
	normalized.z = vec.z / length;
	return normalized;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float CalculateYaw(const vec3_t& direction) {
	return std::atan2(direction.y, direction.x) * (180.0f / M_PI); // Convert radians to degrees
}

float CalculatePitch(const vec3_t& direction) {
	float horizontalDistance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	return std::atan2(-direction.z, horizontalDistance) * (180.0f / M_PI);


}

// Needed to clamp the mouse movement in order to stop the camera from spinning uncontrollably
float Clamp(float value, float min, float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

#define MouseSens 7.0f
bool isCtrlPressed = false;


void MoveMouse(float deltaX, float deltaY) {
	// Clamp the deltas to the range [-10, 10]
	deltaX = Clamp(deltaX, -45.0f, 45.0f);
	deltaY = Clamp(deltaY, -22.5f, 22.5f);

	// mouse_event uses integers so the deltas must be converted, i didn't realise this for a while
	int mouseDeltaX = static_cast<int>(deltaX);
	int mouseDeltaY = static_cast<int>(deltaY);

	// This just simulates the mouse movement i inputed
	mouse_event(MOUSEEVENTF_MOVE, mouseDeltaX * MouseSens, mouseDeltaY * MouseSens, 0, 0);

	
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		isCtrlPressed = !isCtrlPressed;

		if (isCtrlPressed) {
			keybd_event(VK_LCONTROL, 0, 0, 0); 
		}
		else {
			keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0); 
		}

		while (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
			Sleep(10);
		}
	}
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

int main()
{
	DWORD processID = 0;
	HWND hwnd = FindWindowA(nullptr, "OpenArena");
	if (hwnd == nullptr) {
		std::cerr << "OpenArena window not found!" << std::endl;
		return 1;
	}
	GetWindowThreadProcessId(hwnd, &processID);

	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
	if (hProcess == nullptr) {
		std::cerr << "Failed to open process!" << std::endl;
		return 1;
	}

	uintptr_t moduleBase = GetModuleBaseAddress(processID, L"openarena.exe");
	if (!moduleBase) {
		std::cerr << "Failed to get module base address!" << std::endl;
		return 1;
	}

	DWORD team_ptr = 0;
	ReadProcessMemory(hProcess, (void*)(moduleBase + 0x01B4BB44), &team_ptr, sizeof(team_ptr), nullptr);
	team_ptr = team_ptr + 0x104;
	std::cout << "team Pointer: 0x" << std::hex << team_ptr << std::endl;
	DWORD team = 0;
	ReadProcessMemory(hProcess, (void*)team_ptr, &team, sizeof(team), nullptr);
	std::cout << "team Value: " << std::dec << team << std::endl;

	// I didn't end up using the health address and values in the external solution
	DWORD health_ptr = 0;
	ReadProcessMemory(hProcess, (void*)(moduleBase + 0x01B4BB38), &health_ptr, sizeof(health_ptr), nullptr);
	health_ptr = health_ptr + 0x2DC;
	std::cout << "team Pointer: 0x" << std::hex << health_ptr << std::endl;
	DWORD health = 0;
	ReadProcessMemory(hProcess, (void*)health_ptr, &health, sizeof(health), nullptr);
	std::cout << "team Value: " << std::dec << health << std::endl;

	
	//DWORD baseAddress = GetBaseAddress(processID, "openarena.exe"); //this was replaced by ReadProcessMemory
	DWORD baseTeamAddress = team_ptr; 
	DWORD offsetBetweenBots = 0x840; //Offset between player structures in memory

	DWORD xAddress = baseTeamAddress + (-0xF0); 
	DWORD yAddress = xAddress + 4; 
	DWORD zAddress = yAddress + 4; 

	DWORD pitchAddress = baseTeamAddress + (-0x6C); 
	DWORD yawAddress = pitchAddress + 4; 

	int myTeam = ReadMemory<int>(hProcess, baseTeamAddress);
	std::cout << "Your Team Value: " << myTeam << std::endl;

	// It only scans for the next 15 players, because 16 is the default maximum players in a match
	for (int i = 0; i < 15; i++) {
		DWORD botTeamAddress = baseTeamAddress + (i)*offsetBetweenBots;
		int teamValue = ReadMemory<unsigned char>(hProcess, botTeamAddress);
		int botTeamValue = ReadMemory<int>(hProcess, botTeamAddress);

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

	while (true) {

		//This was handy for stopping my mouse from moving when alt-tabbed from openarena
		if (GetForegroundWindow() != hwnd) {
			Sleep(1000);
			continue;
		}

		vec3_t position = ReadPosition(hProcess, xAddress);

		std::cout << "Player Position: "
			<< "X = " << position.x << ", "
			<< "Y = " << position.y << ", "
			<< "Z = " << position.z << std::endl;

		float pitch = ReadMemory<float>(hProcess, pitchAddress);
		float yaw = ReadMemory<float>(hProcess, yawAddress);
		std::cout << "Pitch: " << pitch << ", Yaw: " << yaw << std::endl;

		// Variables to track the closest enemy
		float closestDistance = FLT_MAX; 
		int closestEnemyIndex = -1;
		vec3_t closestEnemyPosition = { 0, 0, 0 };

		// This is the same thing as before the while loop, this just prints it every frame. Needed it for some troubleshooting
		for (int i = 0; i < 15; i++) {
			
			DWORD botTeamAddress = baseTeamAddress + (i + 1) * offsetBetweenBots;
			int botTeamValue = ReadMemory<int>(hProcess, botTeamAddress);

			if (IsEnemy(myTeam, botTeamValue)) {
				std::cout << "Bot " << (i + 1) << " is an ENEMY! Team Value: " << botTeamValue << std::endl;
			}
			else if (botTeamValue != 0) {
				std::cout << "Bot " << (i + 1) << " is an ALLY. Team Value: " << botTeamValue << std::endl;
				continue;
			}
			else {
				std::cout << "Bot " << (i + 1) << " is an EMPTY SLOT. Team Value: " << botTeamValue << std::endl;
				continue;
			}

			// Calculate the position address for enemies
			DWORD botPositionAddress = xAddress + (i + 1) * offsetBetweenBots;

			vec3_t botPosition = ReadPosition(hProcess, botPositionAddress);

			float distance = CalculateDistance(position, botPosition);

			// Check if this is the closest enemy
			if (distance < closestDistance) {
				closestDistance = distance;
				closestEnemyIndex = i + 1;
				closestEnemyPosition = botPosition;
			}

			std::cout << "Enemy " << (i + 1) << " Position: "
				<< "X = " << botPosition.x << ", "
				<< "Y = " << botPosition.y << ", "
				<< "Z = " << botPosition.z << std::endl;
		}

		if (closestEnemyIndex != -1) {

			vec3_t directionToEnemy = CalculateDirectionalVector(position, closestEnemyPosition);

			float yawToEnemy = CalculateYaw(directionToEnemy);
			float pitchToEnemy = CalculatePitch(directionToEnemy);

			std::cout << "Closest Enemy: Bot " << closestEnemyIndex << std::endl;
			std::cout << "Closest Enemy Position: "
				<< "X = " << closestEnemyPosition.x << ", "
				<< "Y = " << closestEnemyPosition.y << ", "
				<< "Z = " << closestEnemyPosition.z << std::endl;
			std::cout << "Yaw to Enemy: " << yawToEnemy << " degrees" << std::endl;
			std::cout << "Pitch to Enemy: " << pitchToEnemy << " degrees" << std::endl;

			float deltaYaw = yawToEnemy - yaw;   
			if (deltaYaw > 180) { //the clamping wasn't enough to stop the camera spinning, this smoothed it out completely
				deltaYaw -= 360;
			}
			else if (deltaYaw < -180) {
				deltaYaw += 360;
			}

			float deltaPitch = pitchToEnemy - pitch; 
			if (abs(deltaYaw) < 45 && abs(deltaPitch) < 30) {
				MoveMouse(-deltaYaw, deltaPitch);
			}
		}
		else {
			std::cout << "No enemies found!" << std::endl;
		}
		Sleep(10); 
	}
	CloseHandle(hProcess);
	return 0;
}

