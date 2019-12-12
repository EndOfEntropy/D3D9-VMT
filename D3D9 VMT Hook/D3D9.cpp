// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <windowsx.h>
#include <d3d9.h>
#include <D3dx9.h>

// include the Direct3D Library "d3d9.lib" and "D3dx9.lib" in Linker > Input > Additional dependencies

using namespace std;

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
ID3DXFont *pFont = NULL;	// the pointer to the created font
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);	// the color struct used to draw text

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		L"WindowClass",
		L"Our First Direct3D Program",
		WS_OVERLAPPEDWINDOW,
		300, 300,
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);

	// enter the main loop:
	MSG msg;

	while (TRUE)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		render_frame();
	}

	// clean up DirectX and COM
	cleanD3D();

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


// this is the function used to create a font
int CreateDrawingFont(void)
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

wstring GetHexAddress(unsigned int * add)
{
	stringstream ss;
	ss << hex << (add);
	string aStr = ss.str();
	wstring wStr(aStr.begin(), aStr.end());
	return wStr;
}

// this is the function used to draw text
int DrawingText()
{
	RECT Rect = { 200, 50, 300, 100  };
	wstring wStr = (L"d3ddev: ") + (GetHexAddress((unsigned int *)d3ddev));
	LPCWSTR pStr = wStr.c_str();
	pFont->DrawTextW(NULL, pStr, -1, &Rect, DT_CENTER | DT_NOCLIP, RED);

	unsigned int *pVTableBase = (unsigned int*)(*(unsigned int *)d3ddev);
	RECT Rect2 = { 200, 100, 300, 150 };
	wstring wStr2 = (L"VTable: ") + (GetHexAddress((unsigned int *)pVTableBase));
	LPCWSTR pStr2 = wStr2.c_str();
	pFont->DrawTextW(NULL, pStr2, -1, &Rect2, DT_CENTER | DT_NOCLIP, RED);

	unsigned int *pVTableFnc = (unsigned int *)(pVTableBase[42]);
	RECT Rect3 = { 200, 150, 300, 200 };
	wstring wStr3 = (L"EndScene: ") + (GetHexAddress((unsigned int *)pVTableFnc));
	LPCWSTR pStr3 = wStr3.c_str();
	pFont->DrawTextW(NULL, pStr3, -1, &Rect3, DT_CENTER | DT_NOCLIP, RED);

	return 1;
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D


	// create a device class using this information and the info from the d3dpp struct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	CreateDrawingFont();	// Create the font that we will be drawing with. Called only once
}


// this is the function used to render a single frame
void render_frame(void)
{
	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

	// do 3D rendering on the back buffer here
	DrawingText();	// Draw text on the 3D scene

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	pFont->Release();	// close and release the Font
	d3ddev->Release();    // close and release the 3D device
	d3d->Release();    // close and release Direct3D
}