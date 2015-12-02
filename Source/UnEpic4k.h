// An un-epic 4k demo.

// Window properties:
#define WINDOW_TITLE TEXT("UnEpic 4k Demo")
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Minimum amount of milliseconds for each update cycle:
#define STEP_RATE 13

#define STEPS_SCENE_LENGTH 200 
#define STEPS_START 1
#define STEPS_SCENE_1 (STEPS_START + 20) 
#define STEPS_SCENE_2 (STEPS_SCENE_1 + STEPS_SCENE_LENGTH) 
#define STEPS_SCENE_3 (STEPS_SCENE_2 + STEPS_SCENE_LENGTH) 
#define STEPS_SCENE_4 (STEPS_SCENE_3 + STEPS_SCENE_LENGTH) 
#define STEPS_END (STEPS_SCENE_4 + STEPS_SCENE_LENGTH) 

// Globals
HINSTANCE g_hInstance;
HWND g_hWnd;

// Display dc handle
HDC g_hDC;

// Back buffer dc handle
HDC g_hMemDC;

HBITMAP g_hMemBMP;

// Brush handles
HBRUSH g_hColorGreen;
HBRUSH g_hColorPink;
HBRUSH g_hColorRed;
HBRUSH g_hColorYellow;
HBRUSH g_hColorBlack;

// Function prototypes:

// Message handler
LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Setup window, resources, globals, etc
bool Initialize();

// Cleanup resources and exit process
void Shutdown(UINT uExitCode);

void UpdateScene1();

// Main update loop
void Loop();

// Resize the window
void ResizeClientWindow(HWND hWnd, UINT uWidth, UINT uHeight);

// Entry point (main)
void EntryPoint ();