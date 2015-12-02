// An un-epic 4k demo.

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>

#include "UnEpic4k.h"

// -------------------------------------------------------------------------------------------------------- Defines 

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define ABS(X) ((X) < 0 ? (-X) : (X))

// Pixels between each square and the next:

#define SQUARE_THICKNESS 30

// A distance from the center of the window that is guaranteed
// to (at least) reach the border in any direction. We add one pixel
// to account for odd resolutions:

#define BORDER_DISTANCE (1 + (MAX(WINDOW_WIDTH, WINDOW_HEIGHT) / 2))

// Window center:

#define CENTER_X (WINDOW_WIDTH / 2)
#define CENTER_Y (WINDOW_HEIGHT / 2)

// The animation loops for two full squares of different colors
// in either direction:

#define REVERSE_DISPLACEMENT 0

#if REVERSE_DISPLACEMENT
  #define DISPLACEMENT_START (SQUARE_THICKNESS * 2)
  #define DISPLACEMENT_END 0
  #define DISPLACEMENT_STEP -1
#else
  #define DISPLACEMENT_START 0
  #define DISPLACEMENT_END (SQUARE_THICKNESS * 2)
  #define DISPLACEMENT_STEP 1
#endif

// -------------------------------------------------------------------------------------------------------- Life-cycle events 

// Message handler
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
}

bool Initialize() {

  // no WinMain so get the module handle
  g_hInstance = GetModuleHandle(NULL);
  if (g_hInstance == NULL)
    return false;

  // register the window class
  WNDCLASS wc;

  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = WINDOW_TITLE;

  if (RegisterClass(&wc) == 0)
    return false;

  // create the window:
  g_hWnd = CreateWindow(
    WINDOW_TITLE,                                             // class name
    WINDOW_TITLE,                                             // title
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // style
    CW_USEDEFAULT, CW_USEDEFAULT,                             // position
    CW_USEDEFAULT, CW_USEDEFAULT,                             // size
    NULL,                                                     // no parent
    NULL,                                                     // no menu
    g_hInstance,                                              // instance
    NULL                                                      // no special
    );

  if (g_hWnd == NULL)
    return false;

  // setup double buffering:
  g_hDC = GetDC(g_hWnd);
  if (g_hDC == NULL)
    return false;

  g_hMemDC = CreateCompatibleDC(g_hDC);
  if (g_hMemDC == NULL)
    return false;

  g_hMemBMP = CreateCompatibleBitmap(g_hDC, WINDOW_WIDTH, WINDOW_HEIGHT);
  if (g_hMemBMP == NULL)
    return false;

  SelectObject(g_hMemDC, g_hMemBMP);

  // Setup colors
  g_hColorGreen = CreateSolidBrush(RGB(180, 253, 11));
  g_hColorPink = CreateSolidBrush(RGB(254, 1, 154));
  g_hColorRed = CreateSolidBrush(RGB(255, 7, 58));
  g_hColorYellow = CreateSolidBrush(RGB(255, 255, 0));

  g_hColorBlack = CreateSolidBrush(RGB(0, 0, 0));

  if (
    g_hColorGreen == NULL
    || g_hColorPink == NULL
    || g_hColorRed == NULL
    || g_hColorBlack == NULL
    || g_hColorYellow == NULL
    ) {
    return false;
  }

  return true;
}

void Shutdown(UINT uExitCode) {

  // Release resources
  if (g_hColorRed != NULL)
    DeleteObject(g_hColorRed);

  if (g_hColorPink != NULL)
    DeleteObject(g_hColorPink);

  if (g_hColorGreen != NULL)
    DeleteObject(g_hColorGreen);

  if (g_hColorYellow != NULL)
    DeleteObject(g_hColorYellow);

  if (g_hColorBlack != NULL)
    DeleteObject(g_hColorBlack);

  if (g_hMemBMP != NULL)
    DeleteObject(g_hMemBMP);

  if (g_hMemDC != NULL)
    DeleteDC(g_hMemDC);

  if (g_hDC != NULL)
    ReleaseDC(g_hWnd, g_hDC);

  // destroy the window and unregister the class:
  if (g_hWnd != NULL)
    DestroyWindow(g_hWnd);

  WNDCLASS wc;
  if (GetClassInfo(g_hInstance, WINDOW_TITLE, &wc) != 0)
    UnregisterClass(WINDOW_TITLE, g_hInstance);

  // without WinMainCRTStartup() we must exit the process ourselves:
  ExitProcess(uExitCode);
}

// -------------------------------------------------------------------------------------------------------- DRAWING HELPER FUNCTIONS

#define BLOCK_WIDTH (WINDOW_WIDTH / 50)
#define BLOCK_HEIGHT (WINDOW_HEIGHT / 50)

inline void DrawBlock(int x, int y) {
  RECT block = {
    x * BLOCK_WIDTH,
    y * BLOCK_HEIGHT,
    (x + 1) * BLOCK_WIDTH ,
    (y + 1) * BLOCK_HEIGHT
  };
  FillRect(g_hMemDC, &block, g_hColorGreen);
}

void ClearScene() {
  RECT r = {
    0,
    0,
    WINDOW_WIDTH,
    WINDOW_HEIGHT
  };
  FillRect(g_hMemDC, &r, g_hColorBlack);
}

// ex: LOGBRUSH before = getCurrentBrush();
LOGBRUSH getCurrentBrush() {
  HGDIOBJ hbrush = GetCurrentObject(g_hMemDC, OBJ_BRUSH);
  LOGBRUSH lb;
  GetObject(hbrush, sizeof(LOGBRUSH), &lb);
  return lb;
}

// -------------------------------------------------------------------------------------------------------- SCENE 1 - Title

#define TEXT_WIDTH 120

void UpdateScene1() {
  RECT r = {
    CENTER_X - TEXT_WIDTH,
        CENTER_Y - 10,
        CENTER_X + TEXT_WIDTH,
        CENTER_Y + 10
  };
  DrawText(g_hMemDC, TEXT("#####     UnEpic 4K Demo     #####"), -1, &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

// -------------------------------------------------------------------------------------------------------- SCENE 2 - Triangle Pulse

#define NUM_TRIANGLES 10
#define TRIANGLE_WIDTH (WINDOW_WIDTH / (NUM_TRIANGLES - 2))
#define SCENE_2_DISPLACEMENT_END (WINDOW_HEIGHT / 2)
#define SCENE_2_DISPLACEMENT_STEP 5
int scene2Displacement = 1; 

void UpdateScene2() {
  ClearScene();

  int percentComplete = (scene2Displacement * 100) / SCENE_2_DISPLACEMENT_END;
  bool colorToggle = false;
  bool directionToggle = false;
  int tilt = (SCENE_2_DISPLACEMENT_END >> 1) - scene2Displacement >> 2;
  int shift = percentComplete - 50;
  for (int i = 0; i < NUM_TRIANGLES; ++i) {

    directionToggle = !directionToggle;
    int height = scene2Displacement * (directionToggle ? 1 : -1);

    POINT trianglePts[3] = {
      { (TRIANGLE_WIDTH * i) + shift, CENTER_Y + tilt }, 
      { (TRIANGLE_WIDTH * i) + shift + (TRIANGLE_WIDTH >> 1), CENTER_Y + height }, 
      { (TRIANGLE_WIDTH * i) + shift + 1, CENTER_Y - tilt }
    };
    colorToggle = !colorToggle;
    SelectObject(g_hMemDC, (colorToggle) ? g_hColorPink : g_hColorGreen);
    Polygon(g_hMemDC, trianglePts, 3);
  }

  // reset?
  scene2Displacement += SCENE_2_DISPLACEMENT_STEP;
  if (scene2Displacement >= SCENE_2_DISPLACEMENT_END)
    scene2Displacement = DISPLACEMENT_START;
}

// -------------------------------------------------------------------------------------------------------- SCENE 3 - Boxes

int displacement = DISPLACEMENT_START;

void UpdateScene3() {
  // starting at the largest square that reaches the border distance
  // draw smaller and smaller squares until we get to the center:
  for (int i = 1, size = BORDER_DISTANCE; ; i++, size -= SQUARE_THICKNESS) {
    int offset = size + displacement;

    // stop at the center:
    if (offset < 1)
      break;

    // GDI allows us to draw outside the window, but let's be polite...
    RECT r = {
        MAX(CENTER_X - offset, 0),
        MAX(CENTER_Y - offset, 0),
        MIN(CENTER_X + offset, WINDOW_WIDTH),
        MIN(CENTER_Y + offset, WINDOW_HEIGHT),
    };

    FillRect(g_hMemDC, &r, i % 2 == 0 ? g_hColorGreen : g_hColorRed);
  }

  // back to the start?
  displacement += DISPLACEMENT_STEP;
  if (displacement == DISPLACEMENT_END)
    displacement = DISPLACEMENT_START;
}

// -------------------------------------------------------------------------------------------------------- SCENE 4 - Fuzz 

#define SCENE_4_DISPLACEMENT_END (WINDOW_HEIGHT / 2)
#define SCENE_4_DISPLACEMENT_STEP 5
int scene4Displacement = 1; 

void UpdateScene4() {
  int percentComplete = (scene4Displacement * 100) / SCENE_4_DISPLACEMENT_END;

  // reset?
  scene4Displacement += SCENE_4_DISPLACEMENT_STEP;
  if (scene4Displacement >= SCENE_4_DISPLACEMENT_END)
    scene4Displacement = DISPLACEMENT_START;
}

// -------------------------------------------------------------------------------------------------------- MAIN LOOP 

void Loop() {
  MSG msg;
  bool done = false;
  long step = 0;

  while (!done) {
    DWORD dwStart = timeGetTime();
    ++step;

    // Poll windows events
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
      if (msg.message == WM_QUIT)
        done = true;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    step = STEPS_SCENE_4 - 1;

    // Update 
    if (step == STEPS_START) {
      ClearScene();
    } else if (step < STEPS_SCENE_1) {
      UpdateScene1();
    } else if (step == STEPS_SCENE_1) {
      ClearScene();
    } else if (step < STEPS_SCENE_2) {
      UpdateScene2();
    } else if (step == STEPS_SCENE_2) {
      ClearScene();
    } else if (step < STEPS_SCENE_3) {
      UpdateScene3();
    } else if (step < STEPS_SCENE_3) {
      UpdateScene4();
    } else if (step == STEPS_SCENE_3) {
      step = 0; // reset
    }

    // Copy the back buffer to dc
    BitBlt(g_hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);

    // Sleep until next step
    DWORD dwDelta = timeGetTime() - dwStart;
    if (dwDelta < STEP_RATE) {
      Sleep(STEP_RATE - dwDelta);
    }
  }
}

// A helper to resize the window with respect to the client area
void ResizeClientWindow(HWND hWnd, UINT uWidth, UINT uHeight) {
  RECT rcClient, rcWindow;

  GetClientRect(hWnd, &rcClient);
  GetWindowRect(hWnd, &rcWindow);

  MoveWindow(hWnd,
    rcWindow.left,
    rcWindow.top,
    uWidth + (rcWindow.right - rcWindow.left) - rcClient.right,
    uHeight + (rcWindow.bottom - rcWindow.top) - rcClient.bottom,
    FALSE);
}

void EntryPoint() {
  if (!Initialize()) {
    MessageBox(NULL, "Initialization failed.", "Error", MB_OK | MB_ICONERROR);
    Shutdown(1);
  }

  ResizeClientWindow(g_hWnd, WINDOW_WIDTH, WINDOW_HEIGHT);
  ShowWindow(g_hWnd, SW_SHOW);
  Loop();
  Shutdown(0);
}