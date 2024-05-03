#include <windows.h>
#include <string>
#include <ctime>
#include <cmath>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

const int BUTTON_WIDTH = 100;
const int BUTTON_HEIGHT = 50;

HWND buttonHWND;
HWND hwndText;
HWND comboBoxMode;
int clickCount = 0;
int selIndex;
bool isButtonPressed = false;
clock_t buttonPressTime;
POINT cursorPos;
RECT buttonRect;

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
    buttonPressTime = clock();
    
    GetCursorPos(&cursorPos);
    ScreenToClient(hwnd, &cursorPos);
    GetWindowRect(buttonHWND, &buttonRect);
}

void MoveCursorRandomX(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int screenWidth = rect.right - rect.left; 
    int screenHeight = rect.bottom - rect.top; 
    int newX = rect.left + rand() % screenWidth; 
    int newY = rect.top + screenHeight / 2; 
    SetCursorPos(newX, newY); 
    buttonPressTime = clock();

    GetCursorPos(&cursorPos);
    ScreenToClient(hwnd, &cursorPos);
    GetWindowRect(buttonHWND, &buttonRect);
}

void MoveCursorRandomPosition(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    RECT buttonRect;
    GetWindowRect(buttonHWND, &buttonRect); 

    int maxX = rect.right - BUTTON_WIDTH;
    int maxY = rect.bottom - BUTTON_HEIGHT;

    int newX, newY;
    do {
        newX = rand() % maxX;
        newY = rand() % maxY;
    } while (newX >= buttonRect.left && newX <= buttonRect.right && newY >= buttonRect.top && newY <= buttonRect.bottom);

    SetCursorPos(rect.left + newX, rect.top + newY);
    buttonPressTime = clock();
}

double DistanceBetweenPoints(POINT point1, POINT point2) {
    double dx = (double)(point2.x - point1.x);
    double dy = (double)(point2.y - point1.y);
    return sqrt(dx * dx + dy * dy);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        0,                             
        CLASS_NAME,                    
        L"WinAPI Button Test",         
        WS_OVERLAPPEDWINDOW,            
        0, 0, 800, 500, // CW_USEDEFAULT, CW_USEDEFAULT
        NULL,      
        NULL,       
        hInstance,  
        NULL        
    );

    if (hwnd == NULL) {
        return 0;
    }

    buttonHWND = CreateWindow(
        L"BUTTON",                     
        L"Клик",                   
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  
        10,                             
        400,                             
        BUTTON_WIDTH,                   
        BUTTON_HEIGHT,                 
        hwnd,                           
        NULL,                           
        (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE),
        NULL                           
    );

    hwndText = CreateWindowEx(
        0, L"EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        550, 50, 200, 400,
        hwnd, NULL, hInstance, NULL
    );

    if (hwndText == NULL) {
        return 0;
    }

    comboBoxMode = CreateWindow(
        L"COMBOBOX", L"",
        WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
        550, 10, 200, 300,
        hwnd, NULL, hInstance, NULL
    );

    if (comboBoxMode == NULL) {
        return 0;
    }

    SendMessage(comboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Режим 1 (фиксированный)");
    SendMessage(comboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Режим 2 (по оси Х)");
    SendMessage(comboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Режим 3 (в рандомном месте)");

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc); 
        HBRUSH hBrush = CreateSolidBrush(RGB(200, 220, 255)); 
        FillRect(hdc, &rc, hBrush); 
        DeleteObject(hBrush);
        return (LRESULT)1; 
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == BN_CLICKED && (HWND)lParam == buttonHWND) {
            clickCount++;
            AddText(L"Нажатие #" + std::to_wstring(clickCount));
            double reactionTime = (double)(clock() - buttonPressTime) / CLOCKS_PER_SEC;

            /*POINT cursorPos;
            GetCursorPos(&cursorPos); 
            ScreenToClient(hwnd, &cursorPos); 

            RECT buttonRect;
            GetWindowRect(buttonHWND, &buttonRect); */

            POINT buttonCenter = {
                buttonRect.left + (buttonRect.right - buttonRect.left) / 2,
                buttonRect.top + (buttonRect.bottom - buttonRect.top) / 2
            };

            double distanceToCenter = DistanceBetweenPoints(cursorPos, buttonCenter);

            double fittsTime = 50 + 150 * log2((double)(distanceToCenter / BUTTON_WIDTH) + 1);
            if (selIndex != 2)
            {
                AddText(L"Время реакции: " + std::to_wstring(reactionTime) + L" сек. (Формула Фиттса: " + std::to_wstring(fittsTime) + L" сек.)");
            }
            else
                AddText(L"Время реакции: " + std::to_wstring(reactionTime) + L" сек.");
            isButtonPressed = true;
            buttonPressTime = clock();
            SetTimer(hwnd, 1, 2000 + rand() % 1001, NULL);
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            KillTimer(hwnd, 1);
            if (isButtonPressed) {
                isButtonPressed = false;
                int selectedIndex = SendMessage(comboBoxMode, CB_GETCURSEL, 0, 0);
                selIndex = selectedIndex;
                switch (selectedIndex) {
                case 0:
                    ReturnCursorToCenter(hwnd);
                    break;
                case 1:
                    MoveCursorRandomX(hwnd);
                    break;
                case 2:
                    MoveCursorRandomPosition(hwnd);
                    break;
                default:
                    break;
                }
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
