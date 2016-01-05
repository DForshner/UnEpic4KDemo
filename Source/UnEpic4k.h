// An un-epic 4k demo.

// Window properties:
#define WINDOW_TITLE TEXT("UnEpic 4k Demo")
#define WINDOW_WIDTH 640 
#define WINDOW_HEIGHT 480 

// Minimum amount of milliseconds for each update cycle:
#define STEP_RATE 13

#define STEPS_START 1
#define STEPS_SCENE_LENGTH 500 
#define STEPS_TITLE_END (STEPS_START + STEPS_TITLE_LENGTH) 
#define STEPS_TITLE_LENGTH 50 
#define STEPS_SCENE_END (STEPS_TITLE_END + STEPS_SCENE_LENGTH) 

// Music Notes
#define C_NOTE_ON_ 0x00403C90
#define C_NOTE_OFF 0x00003C90
#define D_NOTE_ON_ 0x00403E90
#define D_NOTE_OFF 0x00003E90
#define E_NOTE_ON_ 0x00404090
#define E_NOTE_OFF 0x00004090
#define F_NOTE_ON_ 0x00404190
#define F_NOTE_OFF 0x00004190
#define G_NOTE_ON_ 0x00404390
#define G_NOTE_OFF 0x00004390

// Globals
HINSTANCE g_hInstance; // Handle to the program's executable module
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

HMIDIOUT g_hMidiOut;

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