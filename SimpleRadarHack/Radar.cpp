#include <Windows.h>	// Used for DWORD variables. Isn't ideal.
#include "TlHelp32.h"	// Used for getting process / module.

/**
* getModule function.
* Return a module from a process.
*/
DWORD getModule(const char* moduleName, int iPid)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, iPid);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);
	do {
		if (!strcmp(mEntry.szModule, (LPSTR)moduleName)) {
			CloseHandle(hModule);
			return (DWORD)mEntry.hModule;
		}
	} while (Module32Next(hModule, &mEntry));
	return (DWORD)0;
}

/**
* Main function.
* Declare and set module and offset variables. Then loop through players
* and force true to the m_bSpotted offset of enemy entities.
*/
int main(int argc, char* argv[])
{
	// Variables for the module.
	DWORD dwPid;
	HWND hWindow = FindWindowA(0, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hWindow, &dwPid);
	HANDLE hHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);
	DWORD dwClient = getModule("client_panorama.dll", dwPid);

	// Variables for offsets and hack.
	DWORD dwEntityList = 0x4D43AC4;		// List of entities offset.
	DWORD dwLocalPlayer = 0xD2FB94;		// Local player offset.
	DWORD m_iTeamNum = 0xF4;			// Team number offset.
	DWORD dwCurrentEntity;				// Current entity of the entity list.
	DWORD localPlayer;					// Local player.
	DWORD m_bSpotted = 0x93D;			// Entity spotted offset.
	int entityTeam = 0;					// Entity's team number.
	int localTeam = 0;					// Local player's team number.
	char cStopHack = 0;					// Hack kill switch.
	bool bSpotted = true;				// Value written to m_bSpotted offset.

	ReadProcessMemory(hHandle, (LPVOID)(dwClient + dwLocalPlayer), &localPlayer, sizeof(localPlayer), NULL);
	// Hack loop.
	while (cStopHack != 1)
	{
		// Pressing F1 stops the hack.
		// The only way to start it again is to close and re-open it.
		if (GetAsyncKeyState(VK_F1) & 1)
		{
			cStopHack = 1;
		}

		// Loop through entities. Players entities are 0 to 64.
		for (int i = 0; i < 64; i++)
		{
			// Read entity i from the list and add it to dwCurrentEntity.
			ReadProcessMemory(hHandle, (LPVOID)(dwClient + dwEntityList + i * 0x10), &dwCurrentEntity, sizeof(dwCurrentEntity), 0);

			// Check if entity exists.
			if (dwCurrentEntity)
			{
				// Read teams for local player and entity.
				ReadProcessMemory(hHandle, (LPVOID)(dwCurrentEntity + m_iTeamNum), &entityTeam, sizeof(entityTeam), NULL);
				ReadProcessMemory(hHandle, (LPVOID)(localPlayer + m_iTeamNum), &localTeam, sizeof(localTeam), NULL);

				// Check if entity is enemy.
				if (entityTeam != localTeam)
				{
					// Write true to the m_bSpotted offset for the entity.
					WriteProcessMemory(hHandle, (LPVOID)(dwCurrentEntity + m_bSpotted), &bSpotted, sizeof(bSpotted), NULL);
				}
			}
		}
		Sleep(1);
	}
	return 0;
}