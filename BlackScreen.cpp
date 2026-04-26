#include <windows.h>

#include <stdio.h>

#if !defined(TARGET_EXE_NAME)
  #define TARGET_EXE_NAME "BF1942.exe"
#endif


//
// BlackScreen: 0x400
//
LRESULT
WINAPI
WindowProc(
  HWND hWnd,
  UINT Msg,
  WPARAM wParam,
  LPARAM lParam )
{
  return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static LARGE_INTEGER perfCounterStart;
static LARGE_INTEGER perfFrequency;

//
// BlackScreen.exe: 0x410
//
double
getElapsedTime()
{
  static bool isFirstCall = true;


  if ( isFirstCall )
  {
    isFirstCall = false;
    QueryPerformanceCounter(&perfCounterStart);
    QueryPerformanceFrequency(&perfFrequency);
  }

  LARGE_INTEGER perfCounter;
  QueryPerformanceCounter(&perfCounter);

  return static_cast <double> (
    perfCounter.QuadPart - perfCounterStart.QuadPart)
    / perfFrequency.QuadPart;
}

//
// BlackScreen.exe: 0x480
//
bool
restoreSavedDisplayMode()
{
  const char* savedDisplayModePath = "cache\\orgDisplayMode.dat";

  FILE* stream = fopen(savedDisplayModePath, "r" );

  if ( stream != NULL )
  {
    DEVMODE devMode;
    fread(&devMode, sizeof(devMode), 1, stream);

    fclose(stream);
    unlink(savedDisplayModePath); // deletes file

    ChangeDisplaySettings(&devMode, CDS_UPDATEREGISTRY);

    return true;
  }

  return false;
}

//
// BlackScreen.exe: 0x4F0
//
int
WINAPI
WinMain(
  HINSTANCE hInstance,
  HINSTANCE,
  LPSTR lpCmdLine,
  int )
{
  unsigned int processId;
  if ( sscanf(lpCmdLine, "%u ", &processId) != 1 )
  {
    printf("could not get args error\n");
    return 0;
  }

  char* argsStart = lpCmdLine;

  for ( ; *argsStart != '\0' && *argsStart != ' ';
        ++argsStart )
    ;

  if ( *argsStart == ' ' )
    ++argsStart;


  WNDCLASSEX wc;
  ZeroMemory( &wc, sizeof(wc) );

  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = LoadIcon(wc.hInstance, IDI_APPLICATION);
  wc.hIconSm = 0;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = CreateSolidBrush( RGB(0, 0, 0) );
  wc.lpszClassName = "black";

  RegisterClassEx(&wc);


  DEVMODE devMode;
  EnumDisplaySettings(
    NULL, ENUM_CURRENT_SETTINGS, &devMode );

  int displayWidth = devMode.dmPelsWidth;
  int displayHeight = devMode.dmPelsHeight;

  HWND hWnd = CreateWindowEx(
    WS_EX_APPWINDOW,
    "black", "",
    WS_POPUP,
    0, 0,
    displayWidth,
    displayHeight,
    NULL,
    NULL,
    GetModuleHandle(NULL),
    NULL );

  ShowWindow(hWnd, SW_SHOW);
  SetFocus(hWnd);

  SetWindowPos(
    hWnd, HWND_TOPMOST,
    0, 0, 0, 0,
    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );


  HANDLE hImage = LoadImage(
    NULL,
    "00000000.256",
    IMAGE_BITMAP,
    0, 0,
    LR_LOADFROMFILE );

  if ( hImage != INVALID_HANDLE_VALUE && hImage != NULL )
  {
    HDC hdcDst = GetDC(hWnd);
    HDC hdcSrc = CreateCompatibleDC(hdcDst);

    SelectObject(hdcSrc, hImage);

    BitBlt( hdcDst,
      displayWidth / 2 - 320,
      displayHeight / 2 - 240,
      640, 480,
      hdcSrc,
      0, 0,
      SRCCOPY );

    DeleteObject(hImage);
    ReleaseDC(hWnd, hdcDst);
  }


  UpdateWindow(hWnd);

  SetCursor(NULL);
  ShowCursor(FALSE);


  HANDLE hProcess = OpenProcess(
    PROCESS_ALL_ACCESS,
    TRUE,
    processId );

  getElapsedTime();

  TerminateProcess(hProcess, 1);
  Sleep(1000);

  DWORD exitCode;
  while ( GetExitCodeProcess(hProcess, &exitCode) != 0 )
  {
    if ( exitCode != STATUS_PENDING )
      break;

    TerminateProcess(hProcess, 1);
    Sleep(1000);
  }

  CloseHandle(hProcess);


  CHAR cmdLine[1000];
  sprintf(cmdLine, TARGET_EXE_NAME " %s", argsStart);

  STARTUPINFO si = {0};
  ZeroMemory( &si, sizeof(si) );
  PROCESS_INFORMATION pi = {0};
  si.cb = sizeof(si);

  BOOL processCreated = CreateProcess(
    NULL, cmdLine,
    NULL, NULL,
    FALSE, CREATE_NEW_PROCESS_GROUP,
    NULL, NULL,
    &si, &pi );

  if ( processCreated != FALSE )
  {
    CloseHandle(pi.hThread);
    Sleep(3000);

    for ( int elapsedSeconds = 1;
          elapsedSeconds <= 60;
          ++elapsedSeconds )
    {
      GetExitCodeProcess(pi.hProcess, &exitCode);

      if ( exitCode != STATUS_PENDING )
        break;

      HKEY hKey;
      if (  ERROR_SUCCESS == RegOpenKeyEx(
              HKEY_CURRENT_USER,
              "SOFTWARE\\Battlefield 1942\\Initiated",
              0, KEY_READ, &hKey ) )
      {
        RegCloseKey(hKey);

        if (  ERROR_SUCCESS == RegOpenKeyEx(
                HKEY_CURRENT_USER,
                "SOFTWARE\\Battlefield 1942",
                0, KEY_READ, &hKey ) )
        {
          RegDeleteKey(hKey, "Initiated");
          RegCloseKey(hKey);
        }

        Sleep(2000);
        return 0;
      }

      Sleep(1000);
    }
  }

  restoreSavedDisplayMode();
  return 0;
}
