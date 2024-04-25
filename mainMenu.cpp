#include <windows.h>
#include <string>
#include <ctime>
#include <cmath>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

const int BUTTON_WIDTH = 100;
const int BUTTON_HEIGHT = 50;
const int ARROW_SIZE = 20;

HWND buttonHWND;
HWND hwndText;
HWND radioCursor;
HWND radioButton;
HWND radioDefault;
int clickCount = 0;
int currentMode = 0;

void AddText(const std::wstring& text) {
    SendMessage(hwndText, EM_SETSEL, -1, -1);
    SendMessage(hwndText, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>((text + L"\r\n").c_str()));
}

void ReturnCursorToCenter(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    POINT center = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
    ClientToScreen(hwnd, &center);
    SetCursorPos(center.x, center.y);
}

void MoveCursorRandomX() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int newX = rand() % screenWidth;
    SetCursorPos(newX, screenHeight / 2); // Появление курсора на высоте центра экрана
}

void MoveButtonRandomPosition(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int maxX = rect.right - BUTTON_WIDTH;
    int maxY = rect.bottom - BUTTON_HEIGHT;
    int newX = rand() % maxX;
    int newY = rand() % maxY;
    SetWindowPos(buttonHWND, HWND_TOP, newX, newY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_SHOWWINDOW);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"WinAPI Button Test",          // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Create button
    buttonHWND = CreateWindow(
        L"BUTTON",                      // Predefined class; Unicode assumed 
        L"MyButton",                    // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,                             // x position 
        125,                            // y position 
        BUTTON_WIDTH,                   // Button width
        BUTTON_HEIGHT,                  // Button height
        hwnd,                           // Parent window
        NULL,                           // No menu.
        (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE),
        NULL                            // Pointer not needed.
    );

    hwndText = CreateWindowEx(
        0, L"EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 200, 350, 80,
        hwnd, NULL, hInstance, NULL
    );

    if (hwndText == NULL) {
        return 0;
    }

    // Create radio buttons
    radioCursor = CreateWindow(
        L"BUTTON", L"Cursor X Random",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        10, 10, 150, 30,
        hwnd, NULL, hInstance, NULL
    );

    radioButton = CreateWindow(
        L"BUTTON", L"Button Random Position",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        10, 50, 150, 30,
        hwnd, NULL, hInstance, NULL
    );

    radioDefault = CreateWindow(
        L"BUTTON", L"Default",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
        10, 90, 150, 30,
        hwnd, NULL, hInstance, NULL
    );

    if (radioCursor == NULL || radioButton == NULL || radioDefault == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == BN_CLICKED && (HWND)lParam == buttonHWND) {
            clickCount++;
            AddText(L"Click #" + std::to_wstring(clickCount));

            POINT cursorPos;
            GetCursorPos(&cursorPos);
            ScreenToClient(hwnd, &cursorPos);

            RECT buttonRect;
            GetClientRect(buttonHWND, &buttonRect);

            double targetX = buttonRect.left + (buttonRect.right - buttonRect.left) / 2.0;
            double targetY = buttonRect.top + (buttonRect.bottom - buttonRect.top) / 2.0;

            double distance = sqrt(pow(cursorPos.x - targetX, 2) + pow(cursorPos.y - targetY, 2));
            double timeReaction = double(clock()) / CLOCKS_PER_SEC;

            std::wstring result = L"Reaction Time: " + std::to_wstring(timeReaction) + L" seconds\r\n";
            result += L"Fitts' Law Time: " + std::to_wstring(log2(2 * distance / BUTTON_WIDTH + 1)) + L" seconds";

            AddText(result);

            // Установка таймера для возвращения курсора в центр экрана через 2-3 секунды
            SetTimer(hwnd, 1, 2000 + rand() % 1001, NULL);
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            ReturnCursorToCenter(hwnd);
            KillTimer(hwnd, 1);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
        if (clickCount < 10) {
            // Выбор режима в зависимости от выбранной радио-кнопки
            if (SendMessage(radioCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                MoveCursorRandomX();
            }
            else if (SendMessage(radioButton, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                MoveButtonRandomPosition(hwnd);
            }
            else {
                // Возврат курсора в центр, если выбран режим по умолчанию
                ReturnCursorToCenter(hwnd);
            }
        }
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
