// An un-epic 4k demo.

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>

#include "UnEpic4k.h"

// -------------------------------------------------------------------------------------------------------- DEFINES 

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

// -------------------------------------------------------------------------------------------------------- WINDOW HELPER FUNCTIONS

// Resize the window with respect to the client area
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

// -------------------------------------------------------------------------------------------------------- GRAPHICS HELPER FUNCTIONS

inline void ClearScene() {
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

short intensities[WINDOW_HEIGHT * WINDOW_WIDTH];

void initIntensities() {
  // Set default intensity to zero
  for (int i = 0; i < WINDOW_HEIGHT * WINDOW_WIDTH; ++i) {
    intensities[i] = RGB(0, 0, 0);
  }
}

const int MAX_INTENSITY = 255;
DWORD firePalette[MAX_INTENSITY];

// Build a pallet of 'fire' colors that can be mapped to an intensity value
void buildPalette() {
  for (int i = 0; i < MAX_INTENSITY; ++i) {
    firePalette[i] = RGB(0, 0, 0);
  }

  // black to blue
  for (int i = 0; i < 10; ++i) {
    int percent = (i * 100) / 10;
    int ratio = percent * MAX_INTENSITY / 100;
    firePalette[i] = RGB(0, 0, ratio);
  }

  // blue to red
  for (int i = 10; i < 50; ++i) {
    int percent = ((i - 10) * 100) / (50 - 10);
    int ratio = percent * MAX_INTENSITY / 100;
    firePalette[i] = RGB(ratio, 0, MAX_INTENSITY - ratio);
  }

  // red to yellow
  for (int i = 50; i < 150; ++i) {
    int percent = ((i - 50) * 100) / (150 - 50);
    int ratio = percent * MAX_INTENSITY / 100;
    firePalette[i] = RGB(MAX_INTENSITY, ratio, 0);
  }

  // yellow to white
  for (int i = 150; i < 255; ++i) {
    int percent = ((i - 150) * 100) / (MAX_INTENSITY - 150);
    int ratio = percent * MAX_INTENSITY / 100;
    firePalette[i] = RGB(MAX_INTENSITY, MAX_INTENSITY, ratio);
  }
}

HBRUSH fireBrushes[MAX_INTENSITY];
void buildBrushes() {
  for (int i = 0; i < MAX_INTENSITY; ++i) {
    fireBrushes[i] = CreateSolidBrush(firePalette[i]);
  }
};

void destroyBrushes() {
  for (int i = 0; i < MAX_INTENSITY; ++i) {
    if (fireBrushes[i]) {
      DeleteObject(fireBrushes[i]);
    }
  }
};

#define NUM_BLOCKS_WIDTH 75 // # blocks
#define NUM_BLOCKS_HEIGHT 75 // # blocks
#define BLOCK_WIDTH (WINDOW_WIDTH / NUM_BLOCKS_WIDTH) // px
#define BLOCK_HEIGHT (WINDOW_HEIGHT / NUM_BLOCKS_HEIGHT) // px
#define BLOCK_GAP (BLOCK_WIDTH / 100) // px

inline void DrawBlock(int x, int y, HBRUSH brush) {
  RECT block = {
    (x * BLOCK_WIDTH + 1),
    (y * BLOCK_HEIGHT + 1),
    ((x + 1) * BLOCK_WIDTH - 1),
    ((y + 1) * BLOCK_HEIGHT - 1)
  };
  FillRect(g_hMemDC, &block, brush);
}

void DrawFireBlock(int x, int y, int intensity) {
  DrawBlock(x, y, fireBrushes[intensity]);
}

// -------------------------------------------------------------------------------------------------------- RANDOM NUMBER GENERATOR

#define MAX_RAND 65502;
unsigned short lfsr = 0xACE1u;
unsigned bit;

// Galois linear feedback shift register
unsigned rand() {
  bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
  return lfsr = (lfsr >> 1) | (bit << 15);
}

// -------------------------------------------------------------------------------------------------------- MUSIC HELPER FUNCTIONS

struct SongStep {
  DWORD note;
  short weight;
};

const int SCORE_LENGTH = 100;
SongStep score[SCORE_LENGTH];

#define NOTE_ON 0x00000090

void initScore() {
  // Set defaults
  for (int i = 0; i < SCORE_LENGTH; ++i) {
    SongStep s;
    s.note = 0x0;
    s.weight = 1;
    score[i] = s;
  }

  for (int i = 1; i < 30; ++i) {
    int volume = i * 10;
    int tone = (i + 60);
    DWORD cmd = NOTE_ON | (tone << 8) | (volume << 16);
    score[i] = { cmd, (short)(volume / 5) + 10 };
  }

  for (int i = 61; i < SCORE_LENGTH; ++i) {
    int volume = (i - 60) * 10;
    int tone = (150 - i);
    DWORD cmd = NOTE_ON | (tone << 8) | (volume << 16);
    score[i] = { cmd, (short)(volume / 5) + 10 };
  }
};

void sendMIDIEvent(HMIDIOUT hMidiOut, DWORD data) {
  midiOutShortMsg(hMidiOut, data);
};

void sendMIDIEvent(BYTE bStatus, BYTE bData1, BYTE bData2) {
  union {
    DWORD 		dwData;
    BYTE		bData[4];
  } dwMsg;

  dwMsg.bData[0] = bStatus;
  dwMsg.bData[1] = bData1; // First MIDI data byte
  dwMsg.bData[2] = bData2; // Second MIDI data byte
  dwMsg.bData[3] = 0;

  midiOutShortMsg(g_hMidiOut, dwMsg.dwData);
}

// -------------------------------------------------------------------------------------------------------- Life-cycle events 

// Window Procedure - Message handler for messages from the window's message queue
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
}

WNDCLASS CreateWindowClass()
{
  WNDCLASS wc; // Stores info about type of window we are creating

  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

  // Setup window procedure that controls our window
  wc.lpfnWndProc = WndProc;

  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = WINDOW_TITLE;

  return wc;
}

HWND CreateMainWindow() {
  return CreateWindow(
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
}

bool Initialize() {

  // no WinMain so get the module handle
  g_hInstance = GetModuleHandle(NULL);
  if (g_hInstance == NULL) {
    return false;
  }

  // Register window class
  WNDCLASS wc = CreateWindowClass();
  if (RegisterClass(&wc) == 0) {
    return false;
  }

  // Create the window
  g_hWnd = CreateMainWindow();
  if (g_hWnd == NULL) {
    return false;
  }

  // setup double buffering:
  g_hDC = GetDC(g_hWnd);
  if (g_hDC == NULL) {
    return false;
  }

  g_hMemDC = CreateCompatibleDC(g_hDC);
  if (g_hMemDC == NULL) {
    return false;
  }

  g_hMemBMP = CreateCompatibleBitmap(g_hDC, WINDOW_WIDTH, WINDOW_HEIGHT);
  if (g_hMemBMP == NULL) {
    return false;
  }

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

  // Setup Midi device
  UINT numDevs = midiOutGetNumDevs();	// Get the number of devices
  UINT curDevice;
  MIDIOUTCAPS devCaps;
  LPMIDIOUTCAPS lpCaps = &devCaps;
  midiOutGetDevCaps(curDevice, lpCaps, sizeof(MIDIOUTCAPS)); // Get device capabilities for first device (curDevice == 0)
  int midiOpenResult = midiOutOpen(&g_hMidiOut, curDevice, /*(DWORD) hwnd*/(DWORD)NULL, (DWORD)NULL, /*CALLBACK_WINDOW*/ (DWORD)NULL);
  if (midiOpenResult != MMSYSERR_NOERROR) {
    return false; // Failed to open first device
  }

  initIntensities();

  buildPalette();

  initScore();

  buildBrushes();

  return true;
}

// Release resources and destroy window
void Shutdown(UINT uExitCode) {

  if (g_hColorRed != NULL) {
    DeleteObject(g_hColorRed);
  }

  if (g_hColorPink != NULL) {
    DeleteObject(g_hColorPink);
  }

  if (g_hColorGreen != NULL) {
    DeleteObject(g_hColorGreen);
  }

  if (g_hColorYellow != NULL) {
    DeleteObject(g_hColorYellow);
  }

  if (g_hColorBlack != NULL) {
    DeleteObject(g_hColorBlack);
  }

  if (g_hMemBMP != NULL) {
    DeleteObject(g_hMemBMP);
  }

  if (g_hMemDC != NULL) {
    DeleteDC(g_hMemDC);
  }

  if (g_hDC != NULL) {
    ReleaseDC(g_hWnd, g_hDC);
  }

  if (g_hMidiOut) {
    midiOutClose(g_hMidiOut);
  }

  // destroy the window and unregister the class:
  if (g_hWnd != NULL) {
    DestroyWindow(g_hWnd);
  }

  WNDCLASS wc;
  if (GetClassInfo(g_hInstance, WINDOW_TITLE, &wc) != 0) {
    UnregisterClass(WINDOW_TITLE, g_hInstance);
  }

  destroyBrushes();

  // without WinMainCRTStartup() we must exit the process ourselves:
  ExitProcess(uExitCode);
}

// -------------------------------------------------------------------------------------------------------- Draw title

#define TEXT_WIDTH 120

void UpdateTitle() {
  RECT r = {
    CENTER_X - TEXT_WIDTH,
        CENTER_Y - 10,
        CENTER_X + TEXT_WIDTH,
        CENTER_Y + 10
  };
  DrawText(g_hMemDC, TEXT("#####     UnEpic 4K Demo     #####"), -1, &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

// -------------------------------------------------------------------------------------------------------- Draw fire

#define DISPLACEMENT_END (WINDOW_HEIGHT / 2)
int displacement = 1;
unsigned max = 0;

void UpdateScene(short weight) {
  int percentComplete = (displacement * 100) / DISPLACEMENT_END;

  // Seed bottom row
  int lastRowStart = NUM_BLOCKS_WIDTH * (NUM_BLOCKS_HEIGHT - 1);
  for (int x = 0; x < NUM_BLOCKS_WIDTH; ++x) {
    unsigned r = (rand() * 100) / MAX_RAND;
    intensities[lastRowStart + x] = (r < percentComplete) ? MAX_INTENSITY - 1: 0;
  }

  // Simulate fire moving upwards from bottom to top
  for (int y = NUM_BLOCKS_HEIGHT - 2; y > 0; --y) {
    for (int x = 0; x < NUM_BLOCKS_WIDTH; ++x) {
      short lowerCenter = intensities[(y + 1) * NUM_BLOCKS_WIDTH + x];
      short center = intensities[(y) * NUM_BLOCKS_WIDTH + x];
      short left = (x != 0) ? intensities[(y) * NUM_BLOCKS_WIDTH + (x - 1)] : 0;
      short right = (x != NUM_BLOCKS_WIDTH - 1) ? intensities[(y) * NUM_BLOCKS_WIDTH + (x + 1)] : 0;
      short avgTemp = (left + center + lowerCenter + right) / 4;

      if (avgTemp > 1) {
        avgTemp -= 1; // Decay
      }

      intensities[(y * NUM_BLOCKS_WIDTH) + x] = avgTemp;
    }
  }

  // Draw fire 
  for (int x = 0; x < NUM_BLOCKS_WIDTH; ++x) {
    for (int y = 0; y < NUM_BLOCKS_HEIGHT; ++y) {
      short intensity = intensities[y * NUM_BLOCKS_WIDTH + x];
      DrawFireBlock(x, y, intensity);
    }
  }

  displacement += weight;
  if (displacement >= DISPLACEMENT_END) {
    displacement = 1;
  }
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

    SongStep current = score[step % SCORE_LENGTH];

    // Play next note in score
    if (current.note != 0x0) {
      sendMIDIEvent(g_hMidiOut, current.note);
    }

    // Update 
    if (step == STEPS_START) {
      ClearScene();
    } else if (step < STEPS_TITLE_END) {
      UpdateTitle();
    } else if (step == STEPS_TITLE_END) {
      ClearScene();
    } else if (step < STEPS_SCENE_END) {
      UpdateScene(current.weight);
    } else if (step == STEPS_SCENE_END) {
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