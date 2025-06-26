// lift.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "lift4.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <map>

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
using namespace std;
//definicje dla przycisków by wiedziec który przyscisk został wcisniety
#define ID_BUTTON_FLOOR1 1001
#define ID_BUTTON_FLOOR2 1002
#define ID_BUTTON_FLOOR3 1003
#define ID_BUTTON_FLOOR4 1004
#define ID_BUTTON_FLOOR5 1005
#define BUTTONS_PER_FLOOR 5

HWND floorButtons[5][BUTTONS_PER_FLOOR]; // wektor dla przycisków

//struktura dla czekajacych ludzi
struct WaitingPerson {
    int currentFloor;
    int destinationFloor;
    bool isActive;
    DWORD timestamp;
};
vector <WaitingPerson> waitingPeople; // wektor dla czekajacych ludzi
vector<WaitingPerson> pasazerowieWWindzie;

const int MAKS_PASAZEROW = 8;
const int MASA_PASAZERA = 70;
const int MAKSYMALNA_MASA_WINDY = 600;

bool ktosWysiadl = false;

enum StanWindy { BEZCZYNNOSC, JEDZIE_DO_OSOBY, CZEKA_NA_OSOBE, JEDZIE_DO_CELU, CZEKA_PO_WYSADZENIU };
StanWindy stanWindy = BEZCZYNNOSC;

WaitingPerson currentPerson;
int liczbapasazerow = 0;

DWORD czasStartCzekania = 0;
const DWORD czas_czekania_windy = 2000; // 1 sekunda

int currentFloor = 1;   // aktualne piętro (1–5)
int targetFloor = 1;    // docelowe piętro
bool isMoving = false;  // czy winda aktualnie się porusza
int elevatorY = 420;

const int elevatorHeight = 80;

const int floorYPositions[] = { 500, 420, 340, 260, 180 };

int FloorFromY(int y)
{
    for (int f = 1; f <= 5; ++f) {
        int targetY = floorYPositions[f - 1] - elevatorHeight;
        if (abs(y - targetY) <= 2) // tolerancja 2 px
            return f;
    }
    return 0;
}



int kierunekWindy = 0; // 0 - brak, 1 - w górę, -1 - w dół

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);






int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);





    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LIFT4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LIFT4));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LIFT4));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LIFT4);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

//funkcja tworzenia guzików
void CreateFloorButtons(HWND hWnd)
{
    const wchar_t* buttonLabels[BUTTONS_PER_FLOOR] = { L"1", L"2", L"3", L"4", L"5" };
    const wchar_t* floorLabels[] = { L"Floor 5", L"Floor 4", L"Floor 3", L"Floor 2", L"Floor 1" }; // Reversed order
    int buttonWidth = 40;
    int buttonHeight = 40;
    int margin = 10;
    int labelHeight = 20;

    // Floor Y positions (top to bottom: Floor 5 at top, Floor 1 at bottom)
    int floorYs[] = { 100, 200, 300, 400, 500 };

    for (int floor = 0; floor < 5; floor++)
    {
        // Create floor label (static text control)
        CreateWindow(
            L"STATIC",                      // Static text control
            floorLabels[floor],             // Text ("Floor 5" at top)
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            400,                            // X position (same as buttons)
            floorYs[floor] - labelHeight,   // Y position (above buttons)
            160,                            // Width (enough for 4 buttons)
            labelHeight,                    // Height
            hWnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);

        // Create buttons for this floor
        for (int btn = 0; btn < BUTTONS_PER_FLOOR; btn++)
        {
            floorButtons[floor][btn] = CreateWindow(
                L"BUTTON",
                buttonLabels[btn],
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                400 + btn * (buttonWidth + margin),
                floorYs[floor],
                buttonWidth,
                buttonHeight,
                hWnd,
                (HMENU)(INT_PTR)(ID_BUTTON_FLOOR1 + floor * BUTTONS_PER_FLOOR + btn),  // <--- tu poprawka
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);

        }
    }
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    CreateFloorButtons(hWnd); //tworzenie przycisków
    SetTimer(hWnd, 1, 30, NULL); // co 30ms
    return TRUE;
}


static void DrawLevitatingPerson(Graphics& g, int x, int y) {
    Pen pen(Color(255, 0, 0, 0), 2); // black pen, 2px wide

    // Head
    g.DrawEllipse(&pen, x, y, 20, 20); // head circle

    int cx = x + 10; // center of the head
    int top = y + 20;

    // Body
    g.DrawLine(&pen, Point(cx, top), Point(cx, top + 25)); // torso

    // Arms (folded, optional)
    g.DrawLine(&pen, Point(cx - 10, top + 10), Point(cx + 10, top + 10));

    // Legs cross-legged: draw an "X" under the body
    g.DrawLine(&pen, Point(cx - 10, top + 25), Point(cx + 10, top + 35));
    g.DrawLine(&pen, Point(cx + 10, top + 25), Point(cx - 10, top + 35));

    // Optional: levitation shadow
    SolidBrush brush(Color(100, 0, 0, 0)); // semi-transparent
    g.FillEllipse(&brush, x, y + 50, 20, 5); // shadow
}

void DrawElevator(Graphics& g, int lewygoraX, int lewygoraY, int szer_pros, int wys_pros)
{
    Pen pen(Color(0, 0, 0), 2);

    // prostokat (poruszanie się windy)
    g.DrawRectangle(&pen, lewygoraX, static_cast<int>(0.6 * lewygoraY), szer_pros, static_cast<int>(1.25 * wys_pros));

    // LEWA
    g.DrawLine(&pen,
        Point(static_cast<int>(0.2 * lewygoraX), static_cast<int>(lewygoraY + 0.2 * wys_pros)),
        Point(lewygoraX, static_cast<int>(lewygoraY + 0.2 * wys_pros)));

    g.DrawLine(&pen,
        Point(static_cast<int>(0.2 * lewygoraX), static_cast<int>(lewygoraY + 0.6 * wys_pros)),
        Point(lewygoraX, static_cast<int>(lewygoraY + 0.6 * wys_pros)));

    g.DrawLine(&pen,
        Point(static_cast<int>(0.2 * lewygoraX), static_cast<int>(lewygoraY + wys_pros)),
        Point(lewygoraX, static_cast<int>(lewygoraY + wys_pros)));

    // PRAWA
    g.DrawLine(&pen,
        Point(lewygoraX + szer_pros, static_cast<int>(lewygoraY + 0.4 * wys_pros)),
        Point(static_cast<int>(1.8 * lewygoraX + szer_pros), static_cast<int>(lewygoraY + 0.4 * wys_pros)));

    g.DrawLine(&pen,
        Point(lewygoraX + szer_pros, static_cast<int>(lewygoraY + 0.8 * wys_pros)),
        Point(static_cast<int>(1.8 * lewygoraX + szer_pros), static_cast<int>(lewygoraY + 0.8 * wys_pros)));
}

void DrawWinda(Graphics& g, int lewygoraX, int lewygoraY, int szer_wind, int wys_wind)
{
    Pen pen(Color(255, 0, 0), 1);

    //prostokat (winda)
    g.DrawRectangle(&pen, lewygoraX + 3, lewygoraY, szer_wind - 7, wys_wind);
}
/* Pierwszya próba rysowania stickfigure
void DrawStickFigure1(Graphics& g, int centerX, int centerY)
{
    Pen pen(Color::Black, 1);

    // Głowa (okrąg)
    int headRadius = 2;
    g.DrawEllipse(&pen, centerX - headRadius, centerY - 3 * headRadius, 2 * headRadius, 2 * headRadius);

    // Ciało (linia w dół od głowy)
    g.DrawLine(&pen, centerX, centerY - headRadius, centerX, centerY + 2 * headRadius);

    // Ręce (pozioma linia)
    g.DrawLine(&pen, centerX - 20, centerY, centerX + 20, centerY);

    // Nogi (dwie linie w dół i na boki)
    g.DrawLine(&pen, centerX, centerY + 2 * headRadius, centerX - 15, centerY + 40);
    g.DrawLine(&pen, centerX, centerY + 2 * headRadius, centerX + 15, centerY + 40);
}
*/
//filipek
void DrawStickFigureONE(Graphics& g, int centerX, int centerY)
{
    Pen pen(Color::Black, 1);

    // Głowa (okrąg)
    int headRadius = 2;
    g.DrawEllipse(&pen, centerX - headRadius, centerY - 3 * headRadius, 2 * headRadius, 2 * headRadius);

    // Ciało (linia w dół od głowy)
    g.DrawLine(&pen, centerX, centerY - headRadius, centerX, centerY + 2 * headRadius);

    // Ręce (pozioma linia)
    g.DrawLine(&pen, centerX - 20, centerY, centerX + 20, centerY);

    // Nogi (dwie linie w dół i na boki)
    g.DrawLine(&pen, centerX, centerY + 2 * headRadius, centerX - 15, centerY + 40);
    g.DrawLine(&pen, centerX, centerY + 2 * headRadius, centerX + 15, centerY + 40);
}

void DrawStickFigure(Graphics& g, int centerX, int centerY, int floorNumber, float scale = 1.0f) {
    Pen pen(Color::Black, 1);

    int headRadius = static_cast<int>(10 * scale);

    // dodanie nr pietra na które ludzik jedzie

    Font font(L"Arial", 10); // możesz dostosować czcionkę i rozmiar
    SolidBrush brush(Color(255, 0, 0, 0)); // czarny kolor

    WCHAR floorText[10];
    swprintf_s(floorText, L"%d", floorNumber);

    // Pozycja tekstu nad głową
    float textX = centerX - 30 * scale;
    float textY = centerY - headRadius - 70 * scale;

    g.DrawString(floorText, -1, &font, PointF(textX, textY), &brush);

    // Głowa
    g.DrawLine(&pen, centerX, centerY - headRadius, centerX, centerY + static_cast<int>(2 * headRadius));

    // Ciało
    g.DrawLine(&pen, centerX, centerY - headRadius, centerX, centerY + static_cast<int>(2 * headRadius));

    // Ręce
    g.DrawLine(&pen, centerX - static_cast<int>(20 * scale), centerY, centerX + static_cast<int>(20 * scale), centerY);

    // Nogi
    g.DrawLine(&pen, centerX, centerY + static_cast<int>(2 * headRadius), centerX - static_cast<int>(15 * scale), centerY + static_cast<int>(40 * scale));

    g.DrawLine(&pen, centerX, centerY + static_cast<int>(2 * headRadius), centerX + static_cast<int>(15 * scale), centerY + static_cast<int>(40 * scale));
}

void dzialaniewindy(int floor, int button)
{

    // sprawdz czy przycisk nie odpowiada aktualnemu piętru
    int clickedFloor = 5 - floor;
    int destinationFloor = button + 1;

    if (clickedFloor == destinationFloor) {
        //blokuje ddoanie pasazera jezeli to to samo pietro
        return;
    }

    // Sprawdź czy nie przekraczamy maksymalnej liczby pasażerów
    if (liczbapasazerow >= MAKS_PASAZEROW) {
        MessageBox(NULL, L"Osiągnięto maksymalną liczbę pasażerów!", L"Ostrzeżenie", MB_OK | MB_ICONWARNING);
        return;
    }

    WaitingPerson person;
    person.currentFloor = 5 - floor; // zmiana bo uuklad zaczyna sie w lewym gornym
    person.destinationFloor = button + 1; // to zostaje – bo guziki 1–5 idą normalnie


    person.isActive = true;
    person.timestamp = GetTickCount();

    waitingPeople.push_back(person);

    if (stanWindy == BEZCZYNNOSC && liczbapasazerow < MAKS_PASAZEROW) {
        currentPerson = person;
        stanWindy = JEDZIE_DO_OSOBY;
        targetFloor = currentPerson.currentFloor;
        kierunekWindy = (person.destinationFloor > person.currentFloor) ? 1 : -1;
        isMoving = true;

    }

    InvalidateRect(GetParent(floorButtons[floor][button]), NULL, TRUE);
}
bool ZnajdzKolejnegoPasazera(WaitingPerson& nowyPasazer)
{
    // Szukamy najstarszego pasażera
    DWORD najstarszyCzas = -1;
    bool znaleziono = false;
    int preferowanyKierunek = 0;

    // Najpierw ustalamy kierunek na podstawie najstarszego pasażera
    for (const auto& p : waitingPeople) {
        if (!p.isActive) continue;

        if (!znaleziono || p.timestamp < najstarszyCzas) {
            najstarszyCzas = p.timestamp;
            nowyPasazer = p;
            preferowanyKierunek = (p.destinationFloor > p.currentFloor) ? 1 : -1;
            znaleziono = true;
        }
    }

    if (!znaleziono) return false;

    // Teraz szukamy osoby w tym samym kierunku jak preferowany
    najstarszyCzas = -1;
    znaleziono = false;

    for (const auto& p : waitingPeople) {
        if (!p.isActive) continue;

        int kierunek = (p.destinationFloor > p.currentFloor) ? 1 : -1;

        if (kierunek == preferowanyKierunek && p.timestamp <= najstarszyCzas) {
            najstarszyCzas = p.timestamp;
            nowyPasazer = p;
            znaleziono = true;
        }
    }

    return znaleziono;
}

void DrawPassengersInElevator(Graphics& g, int elevatorX, int elevatorY, int elevatorWidth, int elevatorHeight) {
    if (pasazerowieWWindzie.empty()) return;

    const float scale = 0.2f;
    const int margin = 10;
    const int availableWidth = elevatorWidth - 2 * margin;
    const int spacing = availableWidth / (pasazerowieWWindzie.size() + 1);

    for (size_t i = 0; i < pasazerowieWWindzie.size(); ++i) {
        int x = elevatorX + margin + (i + 1) * spacing;
        int y = elevatorY + elevatorHeight / 2;
        DrawStickFigure(g, x, y, pasazerowieWWindzie[i].destinationFloor, scale);
    }
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        if (isMoving)
        {
            int targetY = floorYPositions[targetFloor - 1] - elevatorHeight;

            if (elevatorY < targetY) {
                elevatorY += 2;
                if (elevatorY > targetY) elevatorY = targetY;
            }
            else if (elevatorY > targetY) {
                elevatorY -= 2;
                if (elevatorY < targetY) elevatorY = targetY;
            }
            int floorHere = FloorFromY(elevatorY);

            if (floorHere != 0 && floorHere != currentFloor) {
                bool ktosWsiadl = false;
                bool ktosWysiadl = false;

                // Wysiadanie
                for (auto it = pasazerowieWWindzie.begin(); it != pasazerowieWWindzie.end(); ) {
                    if (it->destinationFloor == floorHere) {
                        it = pasazerowieWWindzie.erase(it);
                        liczbapasazerow--;
                        ktosWysiadl = true;
                    }
                    else {
                        ++it;
                    }
                }

                // Wsiadanie
                for (auto& p : waitingPeople) {
                    if (!p.isActive) continue;
                    if (liczbapasazerow >= MAKS_PASAZEROW) break;

                    if (p.currentFloor == floorHere &&
                        ((kierunekWindy == 1 && p.destinationFloor > floorHere) ||
                            (kierunekWindy == -1 && p.destinationFloor < floorHere))) {

                        p.isActive = false;
                        liczbapasazerow++;
                        pasazerowieWWindzie.push_back(p);
                        ktosWsiadl = true;

                        if (kierunekWindy == 1 && p.destinationFloor > targetFloor)
                            targetFloor = p.destinationFloor;
                        else if (kierunekWindy == -1 && p.destinationFloor < targetFloor)
                            targetFloor = p.destinationFloor;
                    }
                }

                if (ktosWsiadl || ktosWysiadl) {
                    currentFloor = floorHere;
                    stanWindy = CZEKA_NA_OSOBE;
                    czasStartCzekania = GetTickCount();
                    isMoving = false;
                    InvalidateRect(hWnd, NULL, TRUE);
                    return 0;
                }
            }


            if (floorHere && floorHere != currentFloor)
            {
                if (stanWindy == JEDZIE_DO_CELU)
                {
                    bool ktosZabrany = false;

                    for (auto& p : waitingPeople)
                    {
                        if (!p.isActive) continue;


                        if (p.currentFloor == floorHere &&
                            ((kierunekWindy == 1 && p.destinationFloor > p.currentFloor) ||
                                (kierunekWindy == -1 && p.destinationFloor < p.currentFloor)))

                        {
                            p.isActive = false;
                            pasazerowieWWindzie.push_back(p);
                            liczbapasazerow++;

                            if (kierunekWindy == 1 && p.destinationFloor > targetFloor)
                                targetFloor = p.destinationFloor;
                            if (kierunekWindy == -1 && p.destinationFloor < targetFloor)
                                targetFloor = p.destinationFloor;

                            ktosZabrany = true;
                        }
                    }

                    if (ktosZabrany)
                    {
                        isMoving = false;
                        stanWindy = CZEKA_NA_OSOBE;
                        czasStartCzekania = GetTickCount();
                        currentFloor = floorHere;
                    }

                }

                currentFloor = floorHere;
            }
            currentFloor = FloorFromY(elevatorY);

            if (elevatorY == targetY)
            {
                currentFloor = targetFloor;
                isMoving = false;

                // Sprawdź, czy ktoś właśnie stoi na tym piętrze i jedzie w tym samym kierunku
                for (auto& p : waitingPeople)
                {
                    if (!p.isActive) continue;

                    // ten warunek jest minimalny i poprawny
                    if (p.currentFloor == currentFloor &&
                        ((kierunekWindy == 1 && p.destinationFloor > p.currentFloor) ||
                            (kierunekWindy == -1 && p.destinationFloor < p.currentFloor)))
                    {
                        p.isActive = false;
                        pasazerowieWWindzie.push_back(p);
                        liczbapasazerow++;

                        // Aktualizacja targetFloor, żeby jechać dalej, jeśli trzeba
                        if (kierunekWindy == 1 && p.destinationFloor > targetFloor)
                            targetFloor = p.destinationFloor;
                        else if (kierunekWindy == -1 && p.destinationFloor < targetFloor)
                            targetFloor = p.destinationFloor;
                    }
                }


                if (stanWindy == JEDZIE_DO_OSOBY)
                {
                    pasazerowieWWindzie.push_back(currentPerson);
                    currentPerson.isActive = false;
                    // trzeba również zdezaktywować **oryginalnego** pasażera z `waitingPeople`
                    for (auto& p : waitingPeople) {
                        if (p.currentFloor == currentPerson.currentFloor &&
                            p.destinationFloor == currentPerson.destinationFloor &&
                            p.timestamp == currentPerson.timestamp) {
                            p.isActive = false;
                            break;
                        }
                    }
                    liczbapasazerow++;

                    kierunekWindy = (currentPerson.destinationFloor > currentFloor) ? 1 : -1;
                    targetFloor = currentPerson.destinationFloor;

                    czasStartCzekania = GetTickCount();
                    stanWindy = CZEKA_NA_OSOBE;
                }

                else if (stanWindy == JEDZIE_DO_CELU) {
                    bool ktosWysiadl = false;

                    // Pasażerowie wysiadający na tym piętrze
                    for (auto it = pasazerowieWWindzie.begin(); it != pasazerowieWWindzie.end(); ) {
                        if (it->destinationFloor == currentFloor) {
                            it = pasazerowieWWindzie.erase(it);
                            liczbapasazerow--;
                            ktosWysiadl = true;
                        }
                        else {
                            ++it;
                        }
                    }

                    // Jeśli ktoś wysiadł – zatrzymaj się na 2 sekundy
                    if (ktosWysiadl) {
                        stanWindy = CZEKA_NA_OSOBE;
                        czasStartCzekania = GetTickCount();
                        InvalidateRect(hWnd, NULL, TRUE); // Odśwież okno
                        return 0;
                    }

                    // Zabieramy osoby z tego piętra jadące w tym samym kierunku
                    for (auto& p : waitingPeople) {
                        if (p.isActive && liczbapasazerow < MAKS_PASAZEROW &&
                            p.currentFloor == currentFloor &&
                            ((kierunekWindy == 1 && p.destinationFloor > currentFloor) ||
                                (kierunekWindy == -1 && p.destinationFloor < currentFloor))) {
                            pasazerowieWWindzie.push_back(p);
                            p.isActive = false;
                            liczbapasazerow++;
                        }
                    }

                    if (!pasazerowieWWindzie.empty()) {
                        // Ustal nowy cel na podstawie wszystkich pasażerów
                        int nowyCel = pasazerowieWWindzie[0].destinationFloor;
                        for (const auto& p : pasazerowieWWindzie) {
                            if (kierunekWindy == 1 && p.destinationFloor > nowyCel) {
                                nowyCel = p.destinationFloor;
                            }
                            else if (kierunekWindy == -1 && p.destinationFloor < nowyCel) {
                                nowyCel = p.destinationFloor;
                            }
                        }

                        targetFloor = nowyCel;
                        isMoving = true;
                        stanWindy = JEDZIE_DO_CELU;
                    }
                    else {
                        WaitingPerson nastepny;
                        if (ZnajdzKolejnegoPasazera(nastepny)) {
                            currentPerson = nastepny;
                            kierunekWindy = (currentPerson.destinationFloor > currentPerson.currentFloor) ? 1 : -1;
                            targetFloor = currentPerson.currentFloor;
                            stanWindy = JEDZIE_DO_OSOBY;
                            isMoving = true;
                        }
                        else {
                            if (currentFloor != 1) {
                                targetFloor = 1;
                                kierunekWindy = -1;
                                isMoving = true;
                                stanWindy = JEDZIE_DO_CELU;
                            }
                            else {
                                stanWindy = BEZCZYNNOSC;
                                kierunekWindy = 0;
                            }
                        }
                    }
                }



                czasStartCzekania = GetTickCount();
                stanWindy = CZEKA_PO_WYSADZENIU;

            }
            InvalidateRect(hWnd, NULL, FALSE); // FALSE aby uniknąć mrugania
        }
        else if (stanWindy == CZEKA_NA_OSOBE)
        {
            if (GetTickCount() - czasStartCzekania >= czas_czekania_windy)
            {
                stanWindy = JEDZIE_DO_CELU;
                isMoving = true;
            }
        }
        else if (stanWindy == CZEKA_PO_WYSADZENIU)
        {
            if (GetTickCount() - czasStartCzekania >= czas_czekania_windy)
            {
                // Ustal nowy cel lub stan bezczynności
                if (!pasazerowieWWindzie.empty())
                {
                    targetFloor = pasazerowieWWindzie[0].destinationFloor;
                    for (const auto& p : pasazerowieWWindzie)
                    {
                        if (kierunekWindy == 1 && p.destinationFloor > targetFloor)
                            targetFloor = p.destinationFloor;
                        if (kierunekWindy == -1 && p.destinationFloor < targetFloor)
                            targetFloor = p.destinationFloor;
                    }
                    stanWindy = JEDZIE_DO_CELU;
                    isMoving = true;
                }
                else
                {
                    bool ktosPoDrodze = false;
                    for (auto& p : waitingPeople)
                    {
                        if (!p.isActive) continue;
                        if ((kierunekWindy == 1 && p.currentFloor >= currentFloor && p.destinationFloor > p.currentFloor) ||
                            (kierunekWindy == -1 && p.currentFloor <= currentFloor && p.destinationFloor < p.currentFloor))


                        {
                            currentPerson = p;
                            p.isActive = false;
                            targetFloor = p.currentFloor;
                            stanWindy = JEDZIE_DO_OSOBY;
                            isMoving = true;
                            kierunekWindy = (p.destinationFloor > p.currentFloor) ? 1 : -1;
                            ktosPoDrodze = true;
                            break;
                        }
                    }

                    if (!ktosPoDrodze)
                    {
                        // Spróbuj znaleźć osobę w przeciwnym kierunku
                        WaitingPerson nowy;
                        bool ktosWsiadl = false;
                        for (auto& p : waitingPeople) {
                            if (p.isActive &&
                                p.currentFloor == currentFloor &&
                                ((p.destinationFloor > currentFloor && kierunekWindy == -1) ||
                                    (p.destinationFloor < currentFloor && kierunekWindy == 1))) {
                                pasazerowieWWindzie.push_back(p);
                                p.isActive = false;
                                liczbapasazerow++;
                                ktosWsiadl = true;
                            }
                        }

                        if (ktosWsiadl) {
                            int nowyCel = pasazerowieWWindzie[0].destinationFloor;
                            for (const auto& p : pasazerowieWWindzie) {
                                if (kierunekWindy == 1 && p.destinationFloor > nowyCel)
                                    nowyCel = p.destinationFloor;
                                else if (kierunekWindy == -1 && p.destinationFloor < nowyCel)
                                    nowyCel = p.destinationFloor;
                            }
                            targetFloor = nowyCel;
                            isMoving = true;
                            stanWindy = JEDZIE_DO_CELU;
                            return 0;
                        }

                        if (ZnajdzKolejnegoPasazera(nowy))
                        {
                            currentPerson = nowy;
                            kierunekWindy = (nowy.destinationFloor > nowy.currentFloor) ? 1 : -1;
                            targetFloor = nowy.currentFloor;
                            stanWindy = JEDZIE_DO_OSOBY;
                            isMoving = true;
                        }
                        else if (currentFloor != 1)
                        {
                            targetFloor = 1;
                            kierunekWindy = -1;
                            stanWindy = JEDZIE_DO_CELU;
                            isMoving = true;
                        }
                        else
                        {
                            stanWindy = BEZCZYNNOSC;
                            kierunekWindy = 0;
                        }
                    }

                }
            }
        }



        InvalidateRect(hWnd, NULL, TRUE);
        break;
    }


    case WM_COMMAND:
    {
        HWND clickedButton = (HWND)lParam;

        for (int floor = 0; floor < 5; ++floor) {
            for (int btn = 0; btn < 5; ++btn) {
                if (floorButtons[floor][btn] == clickedButton) {
                    dzialaniewindy(floor, btn);
                    return 0;
                }
            }
        }

        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);

        // Wywołanie funkcji rysującej windę
        DrawElevator(graphics, 100, 100, 160, 400);

        DrawWinda(graphics, 100, elevatorY, 160, 80);

        DrawStickFigureONE(graphics, 500, 150); //Filipek

        // wylaczenie przyciskow dla aktualnego piętra
        for (int floor = 0; floor < 5; ++floor) {
            int currentBuildingFloor = 5 - floor;
            for (int btn = 0; btn < 5; ++btn) {
                if ((btn + 1) == currentBuildingFloor) {
                    EnableWindow(floorButtons[floor][btn], FALSE); // wylacz przycisk
                }
                else {
                    EnableWindow(floorButtons[floor][btn], TRUE); //wlacz reszte
                }
            }
        }

        // Rysowanie wszystkich czekających osób
        map<int, int> peopleCountPerFloor;

        for (const auto& person : waitingPeople) {
            if (person.isActive) {
                // Oblicz pozycję X - rozstawiamy osoby obok siebie
                double x = 200;
                double y = 200;


                switch (person.currentFloor) {
                case 1: x = 0.9 * 100;       y = 100 + 400;        break; // piętro 1 (najniżej)
                case 2: x = 2.5 * 100 + 20;  y = 100 + 0.8 * 400;  break;
                case 3: x = 0.9 * 100;       y = 100 + 0.6 * 400;  break;
                case 4: x = 2.5 * 100 + 20;  y = 100 + 0.4 * 400;  break;
                case 5: x = 0.9 * 100;       y = 100 + 0.2 * 400;  break; // piętro 5 (najwyżej)
                default: x = 200;            y = 200;              break;
                }

                double xforludz;//koncowe polozenie czlowieka
                int odstep = 10; // odstęp między osobami

                int index = peopleCountPerFloor[person.currentFloor]++; // ile już narysowano na tym piętrze

                if (x == 0.9 * 100)
                {
                    xforludz = x - odstep * index; // przesunięcie w lewo
                }
                else if (x == 2.5 * 100 + 20) xforludz = x + odstep * index; // przesunięcie w prawo

                // Rysuj osobę na odpowiednim piętrze
                DrawStickFigure(graphics, xforludz, y - 10, person.destinationFloor, 0.2f);
            }
        }

        //rysowanie ludzi w windzie
        DrawPassengersInElevator(graphics, 100 + 3, elevatorY, 160 - 7, 80);

        // Wyświetlanie informacji o masie i liczbie pasażerów
        Font font(L"Arial", 12);
        SolidBrush brush(Color(255, 0, 0, 0));
        WCHAR masaText[50];
        int calkowitaMasa = liczbapasazerow * MASA_PASAZERA;
        swprintf_s(masaText, L"Pasażerów: %d/%d (%d/%d kg)",
            liczbapasazerow, MAKS_PASAZEROW,
            calkowitaMasa, MAKSYMALNA_MASA_WINDY);

        // Rysuj tekst w prawym dolnym rogu
        graphics.DrawString(masaText, -1, &font, PointF(400, 550), &brush);

        // Rysowanie paska obciążenia
        Pen blackPen(Color::Black);  // Tworzymy obiekt Pen przed użyciem
        RectF progressBar(400, 570, 200, 20);
        graphics.DrawRectangle(&blackPen, progressBar);  // Teraz przekazujemy adres istniejącego obiektu

        float procentZaladowania = min(1.0f, static_cast<float>(liczbapasazerow) / MAKS_PASAZEROW);
        RectF fillBar(400, 570, 200 * procentZaladowania, 20);

        // Używamy operatora warunkowego do wyboru koloru
        if (procentZaladowania > 0.9f) {
            SolidBrush redBrush(Color::Red);
            graphics.FillRectangle(&redBrush, fillBar);
        }
        else {
            SolidBrush greenBrush(Color::Green);
            graphics.FillRectangle(&greenBrush, fillBar);
        }

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}