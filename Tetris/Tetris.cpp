//TODO: CHECK IF THE WAY WE LOOP (INTERNAL Y AND EXTERNAL X) IS UNOPTIMIZED BECAUSE OF SPATIAL LOCALITY ON THE CPU CACHE.
#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <thread>
#include <chrono>
#include <string>


#define BORDER 9
#define EMPTY  0
#define LINER  8
#define OFFSET 2
#define MAX_TETROMINO_SPACE 4
#define NUM_INPUT_KEYS 4
#define MOVE_VEL 1

const int nFieldWidth   = 12;
const int nFieldHeight  = 18;
const int nScreenWidth  = 80;
const int nScreenHeight = 30;
unsigned char* pField = nullptr;        // Dynamically allocated array of size nFieldWidth * nFieldHeight 

std::wstring tetromino[7];


void createAssets();
void createField();
int  rotate(int px, int py, int r);
bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY);




int main()
{
    // GAME ASSETS INITIALIZATION ===========================================================
    createAssets();
    createField();



    // WINDOW INITIALIZATION ================================================================
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;



    // GAME LOGIC DATA ======================================================================
    bool bGameOver            = false;              // Playable state
    bool bKey[NUM_INPUT_KEYS] = {false};            // Keys used to play the game
    bool bRotateHold          = false;              // Is the user holding down 'Z'?
    bool bForceDown           = false;              // Is nSpeedCounter == nSpeed ?

    int nCurrentPiece    = 0;                       // Line by default
    int nCurrentRotation = 0;                       // 0° degrees rotation
    int nCurrentX        = nFieldWidth / 2;         // Middle of the field
    int nCurrentY        = 0;                       // Top of the field
    int nSpeed           = 20;                      // Game speed difficulty
    int nSpeedCounter    = 0;                       // If it reaches nSpeed, piece go down
    int nScore           = 0;
    int nPieceCount      = 0;

    std::vector<int> vLines;                        // Stores lines that will disappear 


    // GAME LOOP ============================================================================
    while (!bGameOver)
    {
        // GAME TIMING ======================================================================
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);



        // INPUT ============================================================================
        for (int k = 0; k < NUM_INPUT_KEYS; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
                                                                //RightLeftDownZ


        // GAME LOGIC =======================================================================       
        // Right Key 
        nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + MOVE_VEL, nCurrentY)) ? MOVE_VEL : 0;

        // Left Key 
        nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - MOVE_VEL, nCurrentY)) ? MOVE_VEL : 0;

        // Down Key
        nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + MOVE_VEL)) ? MOVE_VEL : 0;

        // Z Key (rotate)
        if (bKey[3])
        {
            nCurrentRotation += (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;


        // Make piece fall if timer says so
        if (bForceDown)
        {
            nSpeedCounter = 0;
            bForceDown    = false;


            if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + MOVE_VEL))
                nCurrentY += MOVE_VEL;
            else
            {
                // Lock the current piece
                for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
                    for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
                        if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1; // This assignment makes sense if you take a look at "// Draw game field"
                
                nPieceCount++;
                if (nPieceCount % 10 == 0 && nPieceCount >= 10) nSpeed--;

                // Check if we made a line
                for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
                {
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1 && bLine; px++)
                            bLine &= ((pField[(nCurrentY + py) * nFieldWidth + px]) != 0);
                        
                        if (bLine)
                        {
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = LINER;
                            
                            vLines.push_back(nCurrentY + py);
                        }
                    }

                    nScore += 25;
                    if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;
                }
                

                // Choose next piece
                nCurrentX        = nFieldWidth / 2;    
                nCurrentY        = 0;
                nCurrentPiece    = std::rand() % 7;
                nCurrentRotation = 0;

                // Check if next piece fits (if not, game over)
                bGameOver = !(doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY));
            }

        }



        // RENDER OUTPUT ====================================================================
        // Draw game field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + OFFSET) * nScreenWidth + (x + OFFSET)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
        
        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);


        // Draw tetromino
        for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
            for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
                if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + OFFSET) * nScreenWidth + (nCurrentX + px + OFFSET)] = L"ABCDEFG"[nCurrentPiece];
        
        // Draw pieces into new place after making a line
        if (!vLines.empty())
        {
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            for (int &v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = v; py > 0; py--)
                        pField[(py * nFieldWidth) + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }

            nScore += vLines.size() * 10;

            vLines.clear();
        }

        // Flush the buffer to the screen
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);              
    }
    // END OF GAME LOOP =====================================================================

    // Half-life Scientist: "- Oh dear"
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    CloseHandle(hConsole);
    std::cout << "GAME OVER!! SCORE: " << nScore << std::endl;
    system("pause");

    return 0;
}




int rotate(int px, int py, int r)
{
    switch (r % 4)
    {
    case 0: return (MAX_TETROMINO_SPACE * py) + px;       // 0° or 360° state
    case 1: return 12 + py - (MAX_TETROMINO_SPACE * px);  // 90°  state
    case 2: return 15 - (MAX_TETROMINO_SPACE * py) - px;  // 180° state
    case 3: return 3 - py + (MAX_TETROMINO_SPACE * px);   // 270° state
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