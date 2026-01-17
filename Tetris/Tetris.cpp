//TODO: CHECK IF THE WAY WE LOOP (INTERNAL Y AND EXTERNAL X) IS UNOPTIMIZED BECAUSE OF SPATIAL LOCALITY ON THE CPU CACHE.
#include <Windows.h>
#include <iostream>
#include <string>

#define BORDER 9
#define EMPTY  0
#define OFFSET 2
#define MAX_TETROMINO_SPACE 4

const int nFieldWidth   = 12;
const int nFieldHeight  = 18;
const int nScreenWidth  = 80;
const int nScreenHeight = 30;
unsigned char* pField = nullptr;        // Dynamically allocated array of size nFieldWidth * nFieldHeight 

std::wstring tetromino[7];

void createAssets();
void createField();
int rotate(int px, int py, int r);
bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY);




int main()
{
    createAssets();
    createField();

    // Window Initialization
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game loop
    bool bGameOver = false;

    while (!bGameOver)
    {
        // GAME TIMING ======================================================================

        // INPUT ============================================================================

        // GAME LOGIC =======================================================================

        // RENDER OUTPUT ====================================================================




        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + OFFSET) * nScreenWidth + (x + OFFSET)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
        



        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);              // Display Frame
    }


    return 0;
}




int rotate(int px, int py, int r)
{
    switch (r % 4)
    {
    case 0: return (4 * py) + px;       // 0° or 360° state
    case 1: return 12 + py - (4 * px);  // 90°  state
    case 2: return 15 - (4 * py) - px;  // 180° state
    case 3: return 3 - py + (4 * px);   // 270° state
    }
    return 0;
}
 
void createAssets()
{
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"....");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L"....");

    tetromino[2].append(L"....");
    tetromino[2].append(L"..XX");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L"XX..");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"....");
    tetromino[4].append(L".X..");
    tetromino[4].append(L".X..");
    tetromino[4].append(L".XX.");

    tetromino[5].append(L"....");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L".XX.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XXX");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"....");
}

void createField()
{
    pField = new unsigned char[nFieldWidth * nFieldHeight];

    for (int x = 0; x < nFieldWidth; x++)
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? BORDER : EMPTY;
}

//                Id              State          Top X      Top Y
bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
    {
        for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
        {
            int pieceIndex = rotate(px, py, nRotation);
            int fieldIndex = (nPosY + py) * nFieldWidth + (nPosX + px);

            // COLLISION DETECTION ===========================================================
            
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)  
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                    if (tetromino[nTetromino][pieceIndex] == L'X' && pField[fieldIndex] != 0)                           
                        return false;
            
            // COLLISION DETECTION ===========================================================
        }
    }

    return true;
}