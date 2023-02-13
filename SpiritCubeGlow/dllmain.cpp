#include "pch.h"
#include "callbacks.h"
#include <iostream>

unsigned int gamma = 30;  // combo override value
unsigned int glow = 40;   // combo override value

unsigned int base;
unsigned int CubeGammaInjection_JMP_back;
unsigned int CubeGlowInjection_JMP_back;

__declspec(naked) void __declspec(dllexport) CubeGammaInjection() {
	__asm {
		push ebx;
		mov ebx, [gamma];
		movd xmm0, ebx;
		pop ebx;
		jmp[CubeGammaInjection_JMP_back];
	}
}

__declspec(naked) void __declspec(dllexport) CubeGlowInjection() {
	__asm {
		push ebx;
		mov ebx, [glow];
		movd xmm0, ebx;
		pop ebx;
		jmp[CubeGlowInjection_JMP_back];
	}
}

void WriteJMP(BYTE* location, BYTE* newFunction) {
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	location[0] = 0xE9; // jmp
	*((DWORD*)(location + 1)) = (DWORD)(((UINT32)newFunction - (UINT32)location) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}

void WriteBytes(BYTE* location, BYTE bytes[], int size) {
	DWORD dwOldProtection;
	VirtualProtect(location, size, PAGE_EXECUTE_READWRITE, &dwOldProtection);
	for (int i = 0; i < size; i++)
		location[i] = bytes[i];
	VirtualProtect(location, size, dwOldProtection, &dwOldProtection);
}

bool __stdcall HandleChatEvent(wchar_t msg[], unsigned int msg_size) {
	unsigned int custom_gamma;
	unsigned int custom_glow;
	if (swscanf_s(msg, L"/gamma %u", &custom_gamma) == 1) {
		gamma = custom_gamma;
		return true;
	}
	else if (swscanf_s(msg, L"/glow %u", &custom_glow) == 1) {
		glow = custom_glow;
		return true;
	}
	return false;
}

DWORD WINAPI RegisterCallbacks() {
	RegisterCallback("RegisterChatEventCallback", HandleChatEvent);
	return 0;
}

extern "C" __declspec(dllexport) bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	base = (unsigned int)GetModuleHandle(NULL);
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		CubeGammaInjection_JMP_back = base + 0xB5DBC;
		WriteJMP((BYTE*)(base + 0xB5DB7), (BYTE*)&CubeGammaInjection);
		CubeGlowInjection_JMP_back = base + 0xB018B;
		WriteJMP((BYTE*)(base + 0xB0186), (BYTE*)&CubeGlowInjection);
		BYTE unlimitedSpiritCubes[] = { 0x90, 0x90, 0x90, 0x90, 0x90 }; // nop out call to item count dec func
		WriteBytes((BYTE*)(base + 0x188394), unlimitedSpiritCubes, 5);
		BYTE disable16SpiritCubeLimit[] = { 0x20 }; // flip byte representing limit from 16 to 32 (true max)
		WriteBytes((BYTE*)(base + 0xC6A48), disable16SpiritCubeLimit, 1);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
		break;
	}
	return true;
}