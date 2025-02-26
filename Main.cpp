#include <iostream>
#include <windows.h>
#include <string>
#include <gdiplus.h> // Include for GDI+ (Handles JPEG and PNG)
#include <random>
#include <tchar.h>
#pragma comment (lib,"Gdiplus.lib")

using namespace std;

/* ||      CONST          || */

const int COLOMN = 10;
const int ROWS = 10;
const int cellSize = 26;
const int windowShift = 25;
const int RadiusMine = 10;

const int NumMine = 10;

/* ||      CONST          || */

/* || PROGRAMMING TOOLS    ||*/

struct Plot {
    int CountMine = 0;
    bool Mine = false;
    bool ClickOrNo = false;
    bool MinMark = false;
};

Plot BoardGame[ROWS][COLOMN];


bool FlagComplBoard = false;

HDC hdc;
HBITMAP hBitmap = NULL;

/* || PROGRAMMING TOOLS ||*/

/* ||       GAME        ||*/

int ChoiseLine = -1; //Choise Plit
int ChoiseColomn = -1;

const int countEmptyPlit = 90;
int countPlayPlot = 0;

/* ||       GAME        ||*/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ComplectationBoard();
void GameOver();
int CheckWin();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MineSweeperClass";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Create Window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"MineSweeper",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        //size Window
        CW_USEDEFAULT, CW_USEDEFAULT, COLOMN * (cellSize + 10), ROWS * (cellSize + 10),

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void DrawBoard(HDC hdc, HWND hwnd) {

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLOMN; x++) {
            RECT cellRect;
            cellRect.left = x * cellSize + windowShift;
            cellRect.right = (x + 1) * cellSize + windowShift;
            cellRect.top = y * cellSize + windowShift;
            cellRect.bottom = (y + 1) * cellSize + windowShift;
            FillRect(hdc, &cellRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH)); //Light Grey
            FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));//Black grid

            if (y == ChoiseLine && x == ChoiseColomn) {
                FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH)); //Red
            }

            if (BoardGame[y][x].MinMark == true) {
                HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0)); // Yellow
                FillRect(hdc, &cellRect, yellowBrush);
                DeleteObject(yellowBrush); // Important to delete
            }
            if (BoardGame[y][x].ClickOrNo == true) {

                if (BoardGame[y][x].Mine == true) {
                    //The code calls for an elipse.
                    HBRUSH checkerBrush = CreateSolidBrush(RGB(128, 0, 0));
                    HGDIOBJ oldBrush = SelectObject(hdc, checkerBrush);

                    //The code makes sure that values can be referenced.
                    int centerY = cellRect.top + cellSize / 2;
                    int centerX = cellRect.left + cellSize / 2;

                    //the down part of the checker
                    Ellipse(hdc, centerX - RadiusMine, centerY - RadiusMine, centerX + RadiusMine, centerY + RadiusMine);

                    SelectObject(hdc, oldBrush);
                    DeleteObject(checkerBrush);

                    continue;
                }

                if (BoardGame[y][x].CountMine > 0) {//Show Plot;
                    FillRect(hdc, &cellRect, (HBRUSH)GetStockObject(WHITE_BRUSH)); //Light Grey
                    FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));//Black grid

                    SetBkMode(hdc, TRANSPARENT); // Make background transparent for text
                    TCHAR numStr[3];
                    _stprintf_s(numStr, _T("%d"), BoardGame[y][x].CountMine);

                    // Choose text color based on number of mines
                    COLORREF textColor;
                    switch (BoardGame[y][x].CountMine) {
                    case 1: textColor = RGB(0, 0, 255); break;   // Blue
                    case 2: textColor = RGB(0, 128, 0); break; // Green
                    case 3: textColor = RGB(255, 0, 0); break;   // Red
                    case 4: textColor = RGB(0, 0, 128); break;  // Dark Blue
                    case 5: textColor = RGB(128, 0, 0); break;  // Dark Red
                    case 6: textColor = RGB(0, 128, 128); break; // Teal
                    case 7: textColor = RGB(0, 0, 0); break;      // Black
                    case 8: textColor = RGB(128, 128, 128); break; // Gray
                    default: textColor = RGB(0, 0, 0); break;
                    }
                    SetTextColor(hdc, textColor);
                    TextOut(hdc, cellRect.left + 5, cellRect.top + 5, numStr, _tcslen(numStr));
                }
                else {
                    FillRect(hdc, &cellRect, (HBRUSH)GetStockObject(WHITE_BRUSH)); //Light Grey
                    FrameRect(hdc, &cellRect, (HBRUSH)GetStockObject(BLACK_BRUSH));//Black grid
                }
            }
        }
    }
}

void RevealEmptyCells(int row, int col);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int sX, sY;
    switch (uMsg) {
    case WM_SIZE:
        sX = LOWORD(lParam);
        sY = LOWORD(wParam);
        break; // Add break here

    case WM_CREATE:
        ComplectationBoard();
        break; // Add break here

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        DrawBoard(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break; // Add break here
    }

    case WM_LBUTTONDOWN: {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        int col = (xPos - windowShift) / cellSize;
        int row = (yPos - windowShift) / cellSize;

        // Validate row and col to prevent out-of-bounds access
        if (col >= 0 && col < COLOMN && row >= 0 && row < ROWS) {

            // Check if the cell is already clicked. If so, then don't update.
            if (!BoardGame[row][col].ClickOrNo) {

                // Update the previous, and set clicked to true;
                if (ChoiseLine != -1 && ChoiseColomn != -1) {
                    BoardGame[ChoiseLine][ChoiseColomn].ClickOrNo = true;
                }

                ChoiseLine = row;
                ChoiseColomn = col;

                //If the cell is a mine
                if (BoardGame[row][col].Mine == true) {
                    GameOver(); //Game is over!
                }
                else {
                    //Reveal adjacent cells.
                    RevealEmptyCells(row, col);
                    countPlayPlot++;
                }
            }

            InvalidateRect(hwnd, NULL, TRUE); // Force a redraw
        }
        break; // Add Break here
    }

    case WM_RBUTTONDOWN: {
        int xPos = LOWORD(lParam);  // horizontal position of cursor
        int yPos = HIWORD(lParam);  // vertical position of cursor

        int col = (xPos - windowShift) / cellSize;
        int row = (yPos - windowShift) / cellSize;
        if (BoardGame[row][col].MinMark == false) { BoardGame[row][col].MinMark = true; }
        else { BoardGame[row][col].MinMark = false; }
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


//Pictures

HBITMAP LoadBitmapFromFile(const WCHAR* filename) {
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(filename);
    if (bitmap == nullptr) {
        return nullptr;
    }

    Gdiplus::Status status = bitmap->GetLastStatus();
    if (status != Gdiplus::Ok) {
        delete bitmap;
        return nullptr;
    }

    HBITMAP hBitmap = NULL; // Local hBitmap to return
    status = bitmap->GetHBITMAP(Gdiplus::Color::Transparent, &hBitmap);
    if (status != Gdiplus::Ok) {
        delete bitmap;
        return nullptr;
    }

    delete bitmap;
    return hBitmap;
}

int CountMineAreal(int row, int col) {
    int countMine = 0;

    for (int y = row - 1; y <= row + 1; y++) {
        if (y == -1) { y = row; }//чтобы не выходило за пределы
        if (y == 10) { continue; } //не проверяем
        for (int x = col - 1; x <= col + 1; x++) {
            if (x == -1) { x = 0; }
            if (x == 10) { continue; } //не проверяем
            if (y == row && x == col) { continue; }
            else {
                if (BoardGame[y][x].Mine == true) {
                    countMine++;
                }
            }
        }
    }

    return countMine;
}

void GameOver() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLOMN; x++) {
            BoardGame[y][x].ClickOrNo = true;
        }
    }
}

int CheckWin() {
    if (countPlayPlot == countEmptyPlit) {
        return 1;
    }
    else {
        return 0;
    }
}

void ComplectationBoard() {
    random_device rd; // Get a random seed from the OS
    mt19937 gen(rd());   // Mersenne Twister engine
    uniform_int_distribution<> rowDist(0, ROWS - 1); // Distribution for row indices
    uniform_int_distribution<> colDist(0, COLOMN - 1); // Distribution for col indices

    int minesPlaced = 0;
    while (minesPlaced < NumMine) {
        int row = rowDist(gen);
        int col = colDist(gen);

        if (BoardGame[row][col].Mine == false) {
            BoardGame[row][col].Mine = true;
            minesPlaced++;
        }
    }

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLOMN; x++) {
            if (BoardGame[y][y].Mine == true) { continue; }
            else {
                BoardGame[y][x].CountMine = CountMineAreal(y, x);
            }
        }
    }

    FlagComplBoard = true;
}

bool IsValidCell(int row, int col) {
    return (row >= 0 && row < ROWS && col >= 0 && col < COLOMN);
}

void RevealEmptyCells(int row, int col) {
    if (!IsValidCell(row, col) || BoardGame[row][col].ClickOrNo || BoardGame[row][col].Mine == true) {
        return;
    }

    BoardGame[row][col].ClickOrNo = true; // Mark current cell as revealed

    if (BoardGame[row][col].CountMine == 0) {
        // Recursively check adjacent cells
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (i == 0 && j == 0) continue; // Skip current cell
                RevealEmptyCells(row + i, col + j);
            }
        }
    }
}
