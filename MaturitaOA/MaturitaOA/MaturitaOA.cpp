#include <iostream>
#include <cmath>
#include <windows.h>
#include <tlhelp32.h>
#include "game.h"
#include "memory.h"   



// Function to calculate the distance between two positions
float CalculateDistance(const vec3_t& pos1, const vec3_t& pos2) {
	return std::sqrt(
		std::pow(pos2.x - pos1.x, 2) +
		std::pow(pos2.y - pos1.y, 2) +
		std::pow(pos2.z - pos1.z, 2)
	);
}

// Function to calculate the directional vector from pos1 to pos2
vec3_t CalculateDirectionalVector(const vec3_t& pos1, const vec3_t& pos2) {
	vec3_t direction;
	direction.x = pos2.x - pos1.x;
	direction.y = pos2.y - pos1.y;
	direction.z = pos2.z - pos1.z;
	return direction;
}

// Function to normalize a vector (convert it to a unit vector)
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

// Function to calculate the yaw angle from a directional vector
float CalculateYaw(const vec3_t& direction) {
	// Yaw is the angle in the X-Y plane (horizontal plane)
	return std::atan2(direction.y, direction.x) * (180.0f / M_PI); // Convert radians to degrees
}

// Function to calculate the pitch angle from a directional vector
float CalculatePitch(const vec3_t& direction) {
	// Pitch is the angle in the vertical plane
	float horizontalDistance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	return std::atan2(-direction.z, horizontalDistance) * (180.0f / M_PI); // Convert radians to degrees


}

// Function to clamp a value between a minimum and maximum
float Clamp(float value, float min, float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

#define MouseSens 7.0f

// Global variable to keep track of the toggle state
bool isCtrlPressed = false;

// Function to simulate mouse movement
void MoveMouse(float deltaX, float deltaY) {
	// Clamp the deltas to the range [-10, 10]
	deltaX = Clamp(deltaX, -45.0f, 45.0f);
	deltaY = Clamp(deltaY, -22.5f, 22.5f);

	// Convert the deltas to integers (mouse_event uses integers)
	int mouseDeltaX = static_cast<int>(deltaX);
	int mouseDeltaY = static_cast<int>(deltaY);

	// Simulate mouse movement
	mouse_event(MOUSEEVENTF_MOVE, mouseDeltaX * MouseSens, mouseDeltaY * MouseSens, 0, 0);

	// Check if the right mouse button is pressed
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		// Toggle the state of the left control key
		isCtrlPressed = !isCtrlPressed;

		// Simulate left control key press or release based on the toggle state
		if (isCtrlPressed) {
			keybd_event(VK_LCONTROL, 0, 0, 0); // Press left control key
		}
		else {
			keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0); // Release left control key
		}

		// Wait for the right mouse button to be released to avoid multiple toggles
		while (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
			Sleep(10);
		}
	}
}



// Get module base address
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

	// Otevření procesu
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
	if (hProcess == nullptr) {
		std::cerr << "Failed to open process!" << std::endl;
		return 1;
	}

	// Get module base address
	uintptr_t moduleBase = GetModuleBaseAddress(processID, L"openarena.exe");
	if (!moduleBase) {
		std::cerr << "Failed to get module base address!" << std::endl;
		return 1;
	}

	// read health value
	DWORD team_ptr = 0;
	ReadProcessMemory(hProcess, (void*)(moduleBase + 0x01B4BB44), &team_ptr, sizeof(team_ptr), nullptr);
	team_ptr = team_ptr + 0x104;
	std::cout << "team Pointer: 0x" << std::hex << team_ptr << std::endl;
	DWORD team = 0;
	ReadProcessMemory(hProcess, (void*)team_ptr, &team, sizeof(team), nullptr);
	std::cout << "team Value: " << std::dec << team << std::endl;

	// Čtení paměti
	//DWORD baseAddress = GetBaseAddress(processID, "openarena.exe");
	DWORD baseTeamAddress = team_ptr; // Adresa týmu
	DWORD offsetBetweenBots = 0x840; //Offset mezi boty

	// Calculate the X-coordinate address relative to the player team address
	DWORD xAddress = baseTeamAddress + (-0xF0); // X-coordinate address (offset -240)
	DWORD yAddress = xAddress + 4; // Y-coordinate address (4 bytes after X)
	DWORD zAddress = yAddress + 4; // Z-coordinate address (4 bytes after Y)

	// Calculate pitch and yaw addresses
	DWORD pitchAddress = baseTeamAddress + (-0x6C); // Pitch address (offset -0x6C)
	DWORD yawAddress = pitchAddress + 4; // Yaw address (4 bytes after pitch)



	// Read your team value
	int myTeam = ReadMemory<int>(hProcess, baseTeamAddress);
	std::cout << "Your Team Value: " << myTeam << std::endl;

	// Skenovat paměť pro 15 botů/hráčů, základní maximální počet hráčů v OA je 16
	for (int i = 0; i < 15; i++) {
		DWORD botTeamAddress = baseTeamAddress + (i)*offsetBetweenBots;
		int teamValue = ReadMemory<unsigned char>(hProcess, botTeamAddress);
		int botTeamValue = ReadMemory<int>(hProcess, botTeamAddress);

		// Zjistit, zda je bot enemy, ally nebo prázdný slot
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

	// Continuously read and print the player's position
	while (true) {
		// Read the player's position
		vec3_t position = ReadPosition(hProcess, xAddress);


		// Print the player's position
		std::cout << "Player Position: "
			<< "X = " << position.x << ", "
			<< "Y = " << position.y << ", "
			<< "Z = " << position.z << std::endl;

		// Read pitch and yaw values
		float pitch = ReadMemory<float>(hProcess, pitchAddress);
		float yaw = ReadMemory<float>(hProcess, yawAddress);
		std::cout << "Pitch: " << pitch << ", Yaw: " << yaw << std::endl;


		// Variables to track the closest enemy
		float closestDistance = FLT_MAX; // Initialize with a large value
		int closestEnemyIndex = -1;
		vec3_t closestEnemyPosition = { 0, 0, 0 };



		// Scan for the next 15 players
		for (int i = 0; i < 15; i++) {
			// Calculate the team address for the current player
			DWORD botTeamAddress = baseTeamAddress + (i + 1) * offsetBetweenBots;
			int botTeamValue = ReadMemory<int>(hProcess, botTeamAddress);

			// Check if the player is an enemy
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


			// Calculate the position address for the current enemy
			DWORD botPositionAddress = xAddress + (i + 1) * offsetBetweenBots;

			// Read the enemy's position
			vec3_t botPosition = ReadPosition(hProcess, botPositionAddress);

			// Calculate the distance to the enemy
			float distance = CalculateDistance(position, botPosition);


			// Check if this is the closest enemy
			if (distance < closestDistance) {
				closestDistance = distance;
				closestEnemyIndex = i + 1;
				closestEnemyPosition = botPosition;

			}


			// Print the enemy's position
			std::cout << "Enemy " << (i + 1) << " Position: "
				<< "X = " << botPosition.x << ", "
				<< "Y = " << botPosition.y << ", "
				<< "Z = " << botPosition.z << std::endl;

		}


		// Print the closest enemy's information
		if (closestEnemyIndex != -1) {
			// Calculate the directional vector to the closest enemy
			vec3_t directionToEnemy = CalculateDirectionalVector(position, closestEnemyPosition);


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

			// Calculate the required mouse movement
			float deltaYaw = yawToEnemy - yaw;   // Horizontal movement
			float deltaPitch = pitchToEnemy - pitch; // Vertical movement
			if (abs(deltaYaw) < 45 && abs(deltaPitch) < 30) {
			// Simulate mouse movement
			MoveMouse(-deltaYaw, deltaPitch);
			}

			


		}
		else {
			std::cout << "No enemies found!" << std::endl;
		}

		// Wait for a short time before updating again
		Sleep(10); // Adjust the delay (in milliseconds) as needed

	}



	// Clean up
	CloseHandle(hProcess);
	return 0;
}

