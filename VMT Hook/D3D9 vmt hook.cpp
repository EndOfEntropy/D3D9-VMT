// include the basic windows header files and the Direct3D header file
#include "Tools.h"
#include <Tlhelp32.h>
#include <string>
#include <stdio.h>
#include <d3d9.h>
#include <D3dx9.h>

// include the Direct3D Library "d3d9.lib" and "D3dx9.lib" in Linker > Input > Additional dependencies

using namespace std;

// global declarations
LPDIRECT3DDEVICE9 d3ddev;	// the pointer to the device class
unsigned int * EndSceneAddress;	// the pointer to VTable[42] (EndScene address)
ID3DXFont *pFont = NULL;	// the pointer to the created font
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);	// the color struct used to draw text
typedef HRESULT(__stdcall * f_EndScene)(IDirect3DDevice9 * pDevice);
LRESULT __stdcall NewWndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
static LONG OldWndProc;
f_EndScene pTrp_EndScene;
PCHAR mPattern = "\xFF\x52\x40\xA1\x00\x00\x00\x00"; //FF 52 40 A1 ?? ?? ?? ?? (FF 52 40 A1 08 CE 40 00)
PCHAR mMask = "xxxx????";



void CreateConsole(void)
{
	if (AllocConsole()) {
		freopen("CONOUT$", "w", stdout);
		SetConsoleTitle(L"Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		printf("DLL loaded at %X\n", GetModuleHandle(NULL));
	}
}


int FindDevice()
{
	DWORD hD3D = { 0 };
	
	while (!hD3D)
	{
		hD3D = (DWORD)GetModuleHandle(0); // If EndScene => "d3d9.dll"
		printf("Process base addr: %X\n", hD3D);
	}

	PBYTE sig = (PBYTE)FindSignature((PBYTE)hD3D, (DWORD)hD3D+0xB000, (PCHAR)mPattern, (PCHAR)mMask); // d3d9_test.exe + ".text" = 0xB000; d3d9.dll size = 0x171000

	if (sig)
	{
		unsigned int * ppDevice = { 0 };
		memcpy(&ppDevice, (unsigned int *)(sig + 4), 4);
		d3ddev = (LPDIRECT3DDEVICE9)(*ppDevice);		//printf("d3ddev: %X\n", d3ddev);	
		unsigned int * VTable = (unsigned int *)(*(unsigned int *)d3ddev);		//printf("VTable: %X\n", VTable);
		EndSceneAddress = &VTable[42];		//printf("EndScene address: %X\n", *EndSceneAddress);
		pTrp_EndScene = (f_EndScene)VTable[42];		//Save it to return to later in our EndScence hook
	}
	else
	{
		printf("Signature not found : %X\n", sig);
	}

	return 1;
}

// this is the function used to create a font
int CreateDrawingFont()
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height = 24;
	fontDesc.Width = 0;
	fontDesc.Weight = 0;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy_s(fontDesc.FaceName, L"Times New Roman");

	if (d3ddev == NULL)
	{
		cout << "Device to create font on is NULL. \n";
		return 0;
	}

	D3DXCreateFontIndirect(d3ddev, &fontDesc, &pFont);
	return 1;
}


// this is the function used to draw text
int DrawingText()
{
	RECT Rect = { 100, 100, 200, 200 };
	pFont->DrawTextW(NULL, L"Hello, World!", -1, &Rect, DT_CENTER | DT_NOCLIP, RED);
	return 1;
}

HRESULT __stdcall EndSceneHook(IDirect3DDevice9 * pDevice)
{
	//printf("In EndScene (%X)\n", EndSceneHook);
	DrawingText();

	return pTrp_EndScene(pDevice);		// return *This pointer to the original EndScene address
}


int HookVMT()
{
	
	while (*EndSceneAddress != (unsigned int)EndSceneHook)		// For some reason the hook has to be placed twice
	{
		void* pHookFnc = (void*)EndSceneHook;

		DWORD OldProtections = 0;
		VirtualProtect(EndSceneAddress, sizeof(void *), PAGE_EXECUTE_READWRITE, &OldProtections);
		memcpy(EndSceneAddress, &pHookFnc, sizeof(void *));
		VirtualProtect(EndSceneAddress, sizeof(void *), OldProtections, &OldProtections);

		Sleep(50);
	}

	return 1;
}


// WndProc message interception
LRESULT __stdcall NewWndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_INSERT:
			// MessageBoxW(0, L"Test", L"Yay!", MB_OK);
			HookVMT();		// For some reason the hook has to be placed twice. Because we intercept the messages no reset happen while hooking, thus Insert has to be pressed twice

			break;
		}
		break;
	}
	return CallWindowProc((WNDPROC)OldWndProc, Hwnd, Message, wParam, lParam);
}

int SetWdnProc()
{
	D3DDEVICE_CREATION_PARAMETERS d3dpp;
	d3ddev->GetCreationParameters(&d3dpp);

	OldWndProc = SetWindowLongPtr(d3dpp.hFocusWindow, GWL_WNDPROC, (LONG_PTR)NewWndProc);

	return 1;
}


int __stdcall DllMain(HMODULE hModule, DWORD Reason, LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		CreateConsole();
		FindDevice();
		CreateDrawingFont();
		SetWdnProc();		// CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookVMT, 0, 0, 0);
		//HookVMT();

		break;
	case DLL_PROCESS_DETACH:
		pFont->Release();	// close and release the Font
		d3ddev->Release();    // close and release the 3D device

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return 1;
}