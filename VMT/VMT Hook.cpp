#include "Windows.h"
#include <stdio.h>

class Base
{
public:
	Base() { printf("-  Base::Base\n"); }
	virtual ~Base() { printf("-  Base::~Base\n"); }

	void A() { printf("-  Base::A\n"); }
	virtual void B() { printf("-  Base::B\n"); }
	virtual void C() { printf("-  Base::C\n"); }
};

void CreateConsole(void)
{
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		SetConsoleTitle(L"Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		printf("DLL loaded at %X\n", GetModuleHandle(NULL));
	}
}

void __fastcall VMTHookFnc(void *pEcx, void *pEdx)
{
	Base *pThisPtr = (Base *)pEcx;

	printf("In VMTHookFnc from DLL\n");
}

void HookVMT()
{
	unsigned int *pVTableBase = (unsigned int*)(0x004051c8); // Seems like you need the pointer to the VTable not just the pointer to the function
	unsigned int *pVTableFnc = (unsigned int *)((pVTableBase + 1));
	void* pHookFnc = (void*)VMTHookFnc;

	SIZE_T ulOldProtect = 0;
	(void)VirtualProtect(pVTableFnc, sizeof(void *), PAGE_EXECUTE_READWRITE, &ulOldProtect);
	memcpy(pVTableFnc, &pHookFnc, sizeof(void *));
	(void)VirtualProtect(pVTableFnc, sizeof(void *), ulOldProtect, &ulOldProtect);
}

int APIENTRY DllMain(HMODULE hModule, DWORD Reason, LPVOID Reserved)
{
    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
		CreateConsole();
		HookVMT();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return 1;
}