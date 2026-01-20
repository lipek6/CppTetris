#define WIN32_LEAN_AND_MEAN             // Reduces the amount of things we bring from the monstous windows API
#include <Windows.h>         
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <string>


#define MAX_TETROMINO_SPACE 4
#define MAX_DELETABLE_LINES 4
#define NUM_INPUT_KEYS 7
#define MOVE_VEL 1

#define BORDER 9
#define EMPTY  0
#define LINER  8
#define OFFSET 2
#define HOLDER_OFFSET_X 18
#define HOLDER_OFFSET_Y 14

const int nScreenWidth       = 80;
const int nScreenHeight      = 30;
const int nFieldWidth        = 12;
const int nFieldHeight       = 18;
const int nHolderWidth       = 6;                                   
const int nHolderHeight      = 6;                                   
const int nScorePosition     = 2  * nScreenWidth + nFieldWidth + 6;
const int nPausePosition     = 10 * nScreenWidth + nFieldWidth + 6;
const int nTutorialPosition0 = 25 * nScreenWidth + nFieldWidth + 47;
const int nTutorialPosition1 = nTutorialPosition0 + nScreenWidth;
const int nTutorialPosition2 = nTutorialPosition0 + 2 * nScreenWidth;
const int nTutorialPosition3 = nTutorialPosition0 + 3 * nScreenWidth;
const int nTutorialPosition4 = nTutorialPosition0 + 4 * nScreenWidth;

unsigned char *pField       = nullptr;
unsigned char *pHolder      = nullptr;
const wchar_t *CHAR_MAP     = L" ABCDEFG=#";
const char    *KEY_MAPPINGS = "\x27\x25\x28ZPRC";

std::wstring tetromino[7];


void createAssets();
void createField();
void createHolder();
int  rotate(int px, int py, int r);
bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY);
void paused();



int main()
{
    // GAME ASSETS INITIALIZATION ===========================================================
    createAssets();
    createHolder();
    createField();



    // WINDOW INITIALIZATION ================================================================
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;



    // GAME LOGIC DATA ======================================================================
    bool bGameOver = false;                 // Playable state
    bool bKey[NUM_INPUT_KEYS] = { false };  // Keys used to play the game
    bool bRotateKeyHold = false;            // Is the user holding down 'Z'?
    bool bForceDown = false;                // Is nSpeedCounter == nSpeed ?
    bool bPaused = false;
    bool bHolderKeyHold = false;
    bool bHolding = false;                  
    bool bCanHoldNow = true;                // Not allowed to hold and unhold until one piece is fixed

    int nCurrentPiece = rand() % 7;         // Piece that will start at the top
    int nCurrentRotation = 0;               // 0° degrees rotation
    int nCurrentX = nFieldWidth / 2;        // Middle of the field
    int nCurrentY = 0;                      // Top of the field
    int nSpeed = 20;                        // Game speed difficulty
    int nSpeedCounter = 0;                  // If it reaches nSpeed, piece go down
    int nScore = 0;
    int nPieceCount = 0;
    int vLinesCount = 0;
    int vLines[MAX_DELETABLE_LINES] = { 0 };// Stores lines that will disappear 
    int nHoldedPiece = 0;

    // GAME LOOP ============================================================================
    while (!bGameOver)
    {
        // GAME TIMING ======================================================================
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);



        // INPUT ============================================================================
        for (int k = 0; k < NUM_INPUT_KEYS; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)(KEY_MAPPINGS[k]))) != 0;



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
            nCurrentRotation += (!bRotateKeyHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateKeyHold = true;
        }
        else
            bRotateKeyHold = false;

        // P Key (pause)
        if (bKey[4]) bPaused = true;

        // R key (restart) 
        if (bKey[5]) 
        {
            bGameOver = false;                 
            bRotateKeyHold = false;               
            bForceDown = false;                
            bPaused = false;
            nCurrentPiece = rand() % 7;          
            nCurrentRotation = 0;               
            nCurrentX = nFieldWidth / 2;        
            nCurrentY = 0;                      
            nSpeed = 20;                        
            nSpeedCounter = 0;                  
            nScore = 0;
            nPieceCount = 0;
            vLinesCount = 0;
            nHoldedPiece = 0;
            bHolding = false;
            bHolderKeyHold = false;

            for (int y = 0; y < nFieldHeight - 1; y++)
            {
                const int nCurrentFieldY = y * nFieldWidth;
                for (int x = 1; x < nFieldWidth - 1; x++)
                    pField[nCurrentFieldY + x] = 0;
            }

            while ((0x8000 & GetAsyncKeyState((unsigned char)('R'))) != 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // C Key (Hold piece / Use holded piece)
        if (bKey[6] && bCanHoldNow)
        {
            if (!bHolderKeyHold)
            {
                if (bHolding)                                     // Holding a piece
                {
                    std::swap(nCurrentPiece, nHoldedPiece);
                    nCurrentRotation = 0;
                    nCurrentX = nFieldWidth / 2;
                    nCurrentY = 0;
                }
                else                                               // Empty Holder (initial state only)
                {
                    bHolding = true;
                    nHoldedPiece = nCurrentPiece;
                    nCurrentPiece = rand() % 7;
                    nCurrentRotation = 0;
                    nCurrentX = nFieldWidth / 2;
                    nCurrentY = 0;
                }
            }
            bHolderKeyHold = true;
            bCanHoldNow    = false;
        }
        else
            bHolderKeyHold = false;

        // Make piece fall if timer says so
        if (bForceDown)
        {
            nSpeedCounter = 0;
            bForceDown = false;


            if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + MOVE_VEL))
                nCurrentY += MOVE_VEL;
            else
            {
                // Lock the current piece
                for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
                {
                    const int nCurrentTetrominoY = (nCurrentY + py) * nFieldWidth;
                    for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
                        if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                            pField[nCurrentTetrominoY + (nCurrentX + px)] = nCurrentPiece + 1; // This assignment makes sense if you take a look at "// Draw game field"
                }
                nPieceCount++;
                if (nPieceCount % 10 == 0 && nPieceCount >= 10) nSpeed--;

                // Check if we made a line
                for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
                {
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        const int nCurrentTetrominoY = (nCurrentY + py) * nFieldWidth;
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1 && bLine; px++)
                            bLine &= ((pField[nCurrentTetrominoY + px]) != 0);

                        if (bLine)
                        {
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[nCurrentTetrominoY + px] = LINER;

                            vLines[vLinesCount] = nCurrentY + py;
                            vLinesCount++;
                        }
                    }

                    nScore += 25;
                    if (vLinesCount != 0) nScore += (1 << vLinesCount) * 100;
                    
                    bCanHoldNow = true;
                }


                // Choose next piece
                nCurrentX = nFieldWidth / 2;        //TODO
                nCurrentY = 0;
                nCurrentPiece = std::rand() % 7;
                nCurrentRotation = 0;

                // Check if next piece fits (if not, game over)
                bGameOver = !(doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY));
            }

        }



        // RENDER OUTPUT ====================================================================
        // Draw game field
        for (int y = 0; y < nFieldHeight; y++)
        {
            const int nCurrentScreenY = (y + OFFSET) * nScreenWidth;
            const int nCurrentFieldY  = y * nFieldWidth;
            for (int x = 0; x < nFieldWidth; x++)
                screen[nCurrentScreenY + (x + OFFSET)] = CHAR_MAP[pField[nCurrentFieldY + x]];
        }

        // Draw Score
        swprintf_s(&screen[nScorePosition], 16, L"SCORE: %8d", nScore);


        // Draw holding field 
        for (int y = 0; y < nHolderHeight; y++)
        {
            const int nCurrentScreenY = (y + HOLDER_OFFSET_Y) * nScreenWidth;
            const int nCurrentHolderY = y * nHolderWidth;
            for (int x = 0; x < nHolderWidth; x++)
                screen[nCurrentScreenY + (x + HOLDER_OFFSET_X)] = CHAR_MAP[pHolder[nCurrentHolderY + x]];
        }

        // Draw Holded Piece
        if (bHolding)
        {
            for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
            {
                const int nScreenY = (py + HOLDER_OFFSET_Y + 1) * nScreenWidth;
                for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
                {
                    if (tetromino[nHoldedPiece][rotate(px, py, 0)] == L'X')
                        screen[nScreenY + (px + HOLDER_OFFSET_X + 1)] = L"ABCDEFG"[nHoldedPiece];
                }
            }
        }
       

        // Draw tetromino
        for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
        {
            const int nCurrentScreenY = (nCurrentY + py + OFFSET) * nScreenWidth;
            const int nCurrentFieldY = py * nFieldWidth;
            for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
                if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                    screen[nCurrentScreenY + (nCurrentX + px + OFFSET)] = L"ABCDEFG"[nCurrentPiece];
        }

        // Pause the game
        if (bPaused)
        {
            swprintf_s(&screen[nPausePosition], 34, L"GAME PAUSED (press P to continue)");
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            paused();
            bPaused = false;
            swprintf_s(&screen[nPausePosition], 34, L"                                 ");
        }


        // Draw pieces into new place after making a line
        if (vLinesCount)
        {
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            for (int v = 0; v < vLinesCount; v++)
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = vLines[v]; py > 0; py--)
                        pField[(py * nFieldWidth) + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }

            nScore += vLinesCount * 10;

            vLinesCount = 0;
        }

        // Draw instructions
        swprintf_s(&screen[nTutorialPosition0], 9,  L"C - Hold");
        swprintf_s(&screen[nTutorialPosition1], 10, L"P - Pause");
        swprintf_s(&screen[nTutorialPosition2], 11, L"Z - Rotate");
        swprintf_s(&screen[nTutorialPosition3], 12, L"R - Restart");
        swprintf_s(&screen[nTutorialPosition4], 22, L"Move using the ARROWS");

        // Flush the buffer to the screen
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }
    // END OF GAME LOOP =====================================================================

    // Half-life Scientist: "- Oh dear"
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    CloseHandle(hConsole);
    std::cout << "GAME OVER!! SCORE: " << nScore << std::endl;

    while (bKey[0] || bKey[1] || bKey[2] || bKey[3] || bKey[4])
    {
        for (int k = 0; k < NUM_INPUT_KEYS; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)(KEY_MAPPINGS[k]))) != 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    system("pause");
    return 0;
}



//  Rotates a piece by calculating where each tile is mapped in the given rotation.
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



// Creates all the 7 tetromino pieces assets used in the game. All tetrominos are 4x4.
void createAssets()
{   
    tetromino[0] = (L"..X...X...X...X.");
    // ..X.
    // ..X.
    // ..X.
    // ..X.

    tetromino[1] = (L"..X..XX...X.....");
    // ..X.
    // .XX.
    // ..X.
    // ....

    tetromino[2] = (L".....XX..XX.....");
    // ....
    // .XX.
    // .XX.
    // ....

    tetromino[3] = (L"..X..XX..X......");
    // ..X.
    // .XX.
    // .X..
    // ....

    tetromino[4] = (L".X...XX...X.....");
    // .X..
    // .XX.
    // ..X.
    // ....

    tetromino[5] = (L".X...X...XX.....");
    // .X..
    // .X..
    // .XX.
    // ....

    tetromino[6] = (L"..X...X..XX.....");
    // ..X.
    // ..X.
    // .XX.
    // ....
}



// It populates the pField object with numbers representing walls or empty spaces that are written on the screen HANDLE as characters.
void createField()
{
    pField = new unsigned char[nFieldWidth * nFieldHeight];

    for (int y = 0; y < nFieldHeight; y++)
    {
        const int nCurrentY = y * nFieldWidth;
        for (int x = 0; x < nFieldWidth; x++)
            pField[nCurrentY + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? BORDER : EMPTY;
    }
}



// It populates the pHolder object with numbers representing walls or empty spaces that are written on the screen HANDLE as characters.
void createHolder()
{ 
    pHolder = new unsigned char[nHolderWidth * nHolderHeight];

    for (int y = 0; y < nHolderHeight; y++)
    {
        const int nCurrentY = y * nHolderWidth;
        for (int x = 0; x < nHolderWidth; x++)
            pHolder[nCurrentY + x] = (x == 0 || x == nHolderWidth - 1 || y == 0 || y == nHolderHeight - 1) ? BORDER : EMPTY;
    }
}



// Checks if a given piece, at a given rotation state, fits on the given location.
bool doesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for (int py = 0; py < MAX_TETROMINO_SPACE; py++)
    {
        const int nCurrentY = (nPosY + py) * nFieldWidth;
        for (int px = 0; px < MAX_TETROMINO_SPACE; px++)
        {
            const int nCurrentX = nPosX + px;

            int pieceIndex = rotate(px, py, nRotation);
            int fieldIndex = nCurrentY + nCurrentX;

            // COLLISION DETECTION ===========================================================

            if (nCurrentX >= 0 && nCurrentX < nFieldWidth)
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                    if (tetromino[nTetromino][pieceIndex] == L'X' && pField[fieldIndex] != 0)
                        return false;

            // COLLISION DETECTION ===========================================================
        }
    }

    return true;
}



// Holds the game frozen while P is not pressed again
void paused()
{
    // Wait for user to release the P key
    while ((0x8000 & GetAsyncKeyState((unsigned char)('P'))) != 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    

    //Check for P presses to Unpause the game
    bool bPauseKey = false;
    while (!bPauseKey)
    {
        bPauseKey = (0x8000 & GetAsyncKeyState((unsigned char)('P'))) != 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }


    // Wait for user to release the P key
    while ((0x8000 & GetAsyncKeyState((unsigned char)('P'))) != 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}