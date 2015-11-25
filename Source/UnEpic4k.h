// An un-epic 4k demo.

// Window properties:
#define WINDOW_TITLE TEXT("UnEpic 4k Demo")
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Minimum amount of milliseconds for each update cycle:
#define STEP_RATE 13

// Globals:
HINSTANCE g_hInstance;
HWND g_hWnd;
HDC g_hDC;
HDC g_hMemDC;
HBITMAP g_hMemBMP;

// Brushes:
HBRUSH g_hColor1;
HBRUSH g_hColor2;
HBRUSH g_hColorBlack;

// Function prototypes:

// Message handler
LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Setup window, resources, globals, etc
bool Initialize();

// Cleanup resources and exit process
void Shutdown(UINT uExitCode);

void UpdateScene0();

// Main update loop
void Loop();

// Resize the window
void ResizeClientWindow(HWND hWnd, UINT uWidth, UINT uHeight);

// Entry point (main)
void EntryPoint ();