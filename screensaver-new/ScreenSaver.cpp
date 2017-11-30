//Barebones screensaver example from the-generalist.com

#include <windows.h>
#include <scrnsave.h>
#pragma comment (lib, "scrnsavw.lib")
#pragma comment (lib, "comctl32.lib")

#define TIMER 1

//Required Function
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			//Put your initialization code here.
			//This will be called on start up.
			SetTimer( hWnd, TIMER, 10, NULL );
			return(0);
 
		case WM_DESTROY:
			//Put you cleanup code here.
			//This will be called on close.
			KillTimer( hWnd, TIMER );
			return(0);

		case WM_TIMER:
			//Put your drawing code here
			//This is called every 10 ms
			return(0);
	}

	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

//Required Function
BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, 
									   WPARAM wParam, LPARAM lParam)
{
	//Put your configuration interface code here
	return(FALSE);
}

//Required Function
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
	return(TRUE);
}

