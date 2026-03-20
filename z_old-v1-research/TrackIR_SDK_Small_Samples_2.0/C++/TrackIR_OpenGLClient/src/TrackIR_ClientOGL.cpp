// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// * Description:
// *
// *    Main OpenGL application. Renders a first person camera inside a sphere similar
// *    to that in the TrackIR application. The camera changes with the head position 
// *    of the user.
// *
// *******************************************************************************

#define WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include "StdAfx.h"
#include "TrackIR_ClientOGL.h"
#include "GLRenderer.h"
#include "NPClientWraps.h"

#define NP_DEVELOPER_ID         1000        //== Developer ID =============-- 

unsigned long FrameSignature	= 0;
unsigned long StaleFrames		= 0;
char szVersion[MAX_PATH]		= {""};
char szStatus[MAX_PATH]			= {""};

using namespace NPClient;

// TrackIR Functions
bool TrackIR_Enhanced_Init();
void TrackIR_Enhanced_Shutdown();
NPRESULT TrackIR_HandleTrackIRData();

// Application globals
HINSTANCE hInst;
HWND hWnd;
glRenderer* g_pRenderer = NULL;
int LastMouseDownX = 0;
int LastMouseDownY = 0;
const int Timer_ID = 100;
CString gcsDLLPath = "";

// Application functions
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
CString GetDllLocation();

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	MSG msg;
	hInst = hInstance;

	// Create a Win32 Window
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRACKIRCLIENTOGL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TRACKIRCLIENTOGL);
	wcex.lpszClassName	= _T("TrackIRClientOGL");
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&wcex);
	hWnd = CreateWindow(_T("TrackIRClientOGL"), _T("TrackIRClientOGL"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
						CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 0;

	// Create OpenGL renderer/context
	g_pRenderer = new glRenderer();
	bool bSuccess = g_pRenderer->Initialize(hWnd);
	glSphere* pSphere = new glSphere();
	g_pRenderer->m_RenderList3D.push_back(pSphere);

	// Create TrackIR Interface
	bSuccess = TrackIR_Enhanced_Init();
	if(!bSuccess)
	{
		strcpy_s(szVersion, "TrackIR Not Detected");
		strcpy_s(szStatus, "Not Tracking");
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Normal Mode: Create timer to poll TrackIR for data
	// note: Win32 Timer resolution tends to be about 15 msecs
	SetTimer(hWnd, Timer_ID, 10, (TIMERPROC) NULL);
	
	// Win32 Message Loop
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRACKIRCLIENTOGL));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

// Message handler for Win32 window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			TrackIR_Enhanced_Shutdown();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
		if(wParam == Timer_ID)
		{
			NPRESULT result = TrackIR_HandleTrackIRData();
			g_pRenderer->Render();
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if(g_pRenderer)
			g_pRenderer->Render();
		EndPaint(hWnd, &ps);
		break;
	case WM_ERASEBKGND:
		break;
	case WM_SIZE:
		if(g_pRenderer)
			g_pRenderer->ResizeWindow(LOWORD(lParam),HIWORD(lParam));	// LoWord=Width, HiWord=Height
		break;
	case WM_LBUTTONDOWN:
		LastMouseDownX = LOWORD(lParam);
		LastMouseDownY = HIWORD(lParam);
		break;
	case WM_MOUSEWHEEL:
		if(g_pRenderer)
		{
			// zoom the viewpoint
			float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			float fNearLimitZ = -50.0f;
			float fFarLimitZ = 2000.0f;
			float fScale = 10.0f;
			float nNotches = -1.0f * (float)zDelta/ (float)WHEEL_DELTA;
			float fNewZ = (float) g_pRenderer->m_pView->EyeDistance + (nNotches * fScale);
			if( (fNewZ <= fFarLimitZ) && (fNewZ >= fNearLimitZ) )
			{
				g_pRenderer->m_pView->EyeDistance = fNewZ;
				g_pRenderer->m_bTrackingView = false;
				g_pRenderer->Render();
			}
		}
		break;
	case WM_MOUSEMOVE:
		if(wParam & MK_LBUTTON)
		{
			// orbit the viewpoint
			int x = LOWORD(lParam); 
			int y = HIWORD(lParam); 
			float fYawScale = 0.3f;
			float fPitchScale = 0.3f;
			float fYawChange = (x - LastMouseDownX) * fYawScale;
			float fPitchChange = (y - LastMouseDownY) * fPitchScale;
			g_pRenderer->m_pView->EyeAzimuth += fYawChange;
			g_pRenderer->m_pView->EyeElevation += fPitchChange;
			g_pRenderer->Render();
			LastMouseDownX = x;
			LastMouseDownY = y;
		}
		break;
	case WM_DESTROY:
		NP_UnregisterWindowHandle();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// [TrackIR Enhanced SDK]
// Initialization procedure (uses the DLL wrapper module NPClientWraps.cpp/.h)
bool TrackIR_Enhanced_Init()
{
	NPRESULT result;

	// Zero TrackIR SDK Related counters
	FrameSignature = 0;
	StaleFrames = 0;

	// Locate the TrackIR Enhanced DLL
	gcsDLLPath = GetDllLocation();
	result = NPClient_Init( gcsDLLPath );
	// Initialize the the TrackIR Enhanced DLL
	if( NP_OK != result )
	{
		MessageBox(hWnd, _T("Error initializing NPClient interface"), _T("TrackIR Client"), MB_OK);
		return false;
	}

	// Register your applications Window Handle 
	result = NP_RegisterWindowHandle(hWnd);
	if( NP_OK != result )
	{
		MessageBox(hWnd, _T("NPClient : Error registering window handle"), _T("TrackIR Client"), MB_OK);
		return false;
	}

	// Query for the NaturalPoint TrackIR software version
	unsigned short wNPClientVer;
	result = NP_QueryVersion( &wNPClientVer );
	if( NP_OK == result )
		sprintf_s(szVersion, "%d.%d", wNPClientVer >> 8, wNPClientVer & 0x00FF);
	else
		MessageBox(hWnd, _T("NPClient : Error querying NaturalPoint software version"), _T("TrackIR Client"), MB_OK);


	// Choose the Axes that you want tracking data for

	unsigned int DataFields = 0;
	DataFields |= NPPitch;
	DataFields |= NPYaw;
	DataFields |= NPRoll;
	DataFields |= NPX;
	DataFields |= NPY;
	DataFields |= NPZ;

	// Register the Axes selection with the TrackIR Enhanced interface
	NP_RequestData(DataFields);


	// It is *required* that your application registers the Developer ID 
	// assigned by NaturalPoint!
	// NOTE : The title of your project must show up 
	// in the list of supported titles shown in the Profiles
	// tab of the TrackIR software, if it does not then the
	// TrackIR software will *not* transmit data to your
	// application. If your title is not present in the list, 
	// you may need to have the TrackIR software perform a
	// game list update (to download support for new Developer IDs)
	// using the menu item under the "Help" or "Update" menu.
	NP_RegisterProgramProfileID(NP_DEVELOPER_ID);

	// Stop the cursor
	result = NP_StopCursor();
	if (result != NP_OK)
		MessageBox(hWnd, _T("NPCient : Error stopping cursor"), _T("TrackIR Client"), MB_OK);
	
	// Request that the TrackIR software begins sending Tracking Data
	result = NP_StartDataTransmission();
	if (result != NP_OK)
		MessageBox(hWnd, _T("Error starting data transmission"), _T("TrackIR Client"), MB_OK);

	return true;  
}

CString GetDllLocation()
{
	HKEY pKey = NULL;
	LPTSTR pszValue;
	DWORD dwSize;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\NaturalPoint\\NATURALPOINT\\NPClient Location"), 0, KEY_READ, &pKey) != ERROR_SUCCESS)
	{
		MessageBox(hWnd, _T("DLL Location key not present"), _T("TrackIR Client"), MB_OK);
		return "Error";
		
	}
	if (RegQueryValueEx(pKey, _T("Path"), NULL, NULL, NULL, &dwSize) != ERROR_SUCCESS)
	{
		MessageBox(hWnd, _T("Path value not present"), _T("TrackIR Client"), MB_OK);
		return "Error";
	}
	pszValue = (LPTSTR)malloc(dwSize);
	if (pszValue == NULL)
	{
		MessageBox(hWnd, _T("Insufficient memory"), _T("TrackIR Client"), MB_OK);
		return "Error";
	}
	if (RegQueryValueEx(pKey, _T("Path"), NULL, NULL, (LPBYTE)pszValue, &dwSize) != ERROR_SUCCESS)
	{
		::RegCloseKey(pKey);
		MessageBox(hWnd, _T("Error reading location key"), _T("TrackIR Client"), MB_OK);
		return "Error";
	}
	else
	{
		::RegCloseKey(pKey);
	
		
		CString LValue = pszValue;
		return LValue;
	}
}

// [TrackIR Enhanced SDK]
// Shutdown procedure
void TrackIR_Enhanced_Shutdown()
{
	// Request that the TrackIR software stop sending Tracking Data
	NP_StopDataTransmission();

	// Un-register your applications Windows Handle
	NP_UnregisterWindowHandle();
}

// [TrackIR Enhanced SDK]
// Poll TrackIR Enhanced for new data (called by the timer event in this application)
NPRESULT TrackIR_HandleTrackIRData()
{
	
	// Query the TrackIR Enhanced Interface for the latest data
	TRACKIRDATA tid;
	NPRESULT result = NP_GetData( &tid );


	// If the call succeeded, then we have data to process
	if( NP_OK == result )
	{
		// Make sure the remote interface is active
		if (tid.Status == NPSTATUS_REMOTEACTIVE)
		{
			// Compare the last frame signature to the current one if 
			// they are not the same then the data is new
			if (FrameSignature != tid.FrameSignature)
			{

				// In your own application, this is where you would utilize
				// the Tracking Data for View Control / etc.
				if(g_pRenderer)
				{
					// convert TIR translation values from TIR units to centimeters
					double x = ( tid.X / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
					double y = ( tid.Y / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;
					double z = ( tid.Z / NP_MAX_VALUE ) * NP_MAX_TRANSLATION;

					// convert TIR rotation values from TIR units to degrees
					double yaw = ( tid.Yaw / NP_MAX_VALUE ) * NP_MAX_ROTATION;
					double pitch = ( tid.Pitch / NP_MAX_VALUE ) * NP_MAX_ROTATION;
					double roll = ( tid.Roll / NP_MAX_VALUE ) * NP_MAX_ROTATION;

					// update 3D viewpoint
					g_pRenderer->tirData.x = x;
					g_pRenderer->tirData.y = y;
					g_pRenderer->tirData.z = z;
					g_pRenderer->tirData.yaw = yaw;
					g_pRenderer->tirData.pitch = pitch;
					g_pRenderer->tirData.roll = roll;
				}

				FrameSignature = tid.FrameSignature;
				StaleFrames = 0;
				sprintf_s(szStatus, "Tracking");
			}
			else
			{
				// Either there is no tracking data, the user has
				// paused the trackIR, or the call happened before
				// the TrackIR was able to update the interface
				// with new data
				if (StaleFrames > 30)
				{
					sprintf_s(szStatus, "No New Data.  Paused or Not Tracking?");
				}
				else
				{
					StaleFrames++;
					sprintf_s(szStatus, "No New Data for %d frames", StaleFrames);
				}
				result = NP_ERR_NO_DATA;
			}
		}
		else
		{
			// The user has set the device out of trackIR Enhanced Mode
			// and into Mouse Emulation mode with the hotkey
			result = NP_ERR_NO_DATA;
		}
	} 


	return result;
} 
