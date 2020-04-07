// ConsoleTetrisScratch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>

#define FIELD_WIDTH 14
#define FIELD_HEIGHT 21
#define PIECE_DIM 4

// Tetris pieces, stored in wstrings since wchars take less bytes than ints
std::wstring piece[7] = {
    L"..X...X...X...X.", //  I - piece
    L"..X..XX..X......", // N - piece
    L".X...XX...X.....", // N - piece mirrored 
    L".....XX..XX.....", // Square piece
    L".X...X...X...XX.", // L - piece
    L"..X...X...X..XX.", // L - piece mirrored
    L".X...XX..X......"
};


/*
rotateCoord : Rotates a piece coordinate by coordinate
==================================================================
Params:
>> pX, pY: Current coordinate within the piece
>> nRotation: How much to rotate the piece by (0 - 0 deg, 1 - 90 deg, 2 - 180 deg, 3 - 270 deg)
*/
int rotateCoord(int pX, int pY, int nRotation)
{
    int rotatedIndex = 0;
    switch (nRotation)
    {
    case 0:
        rotatedIndex = pX + (PIECE_DIM * pY);
        break;
    case 1:
        rotatedIndex = 12 + pY - (PIECE_DIM * pX);
        break;
    case 2:
        rotatedIndex = 15 - (pX * PIECE_DIM) - pY;
        break;
    case 3:
        rotatedIndex = 3 + (PIECE_DIM * pX) - pY;
    }
    return rotatedIndex;
}


int nScreenWidth;
int nScreenHeight;
wchar_t* screen;
// playing field is a 1-D array allocated dynamically - set it as a nullptr for now
unsigned char* pField = nullptr;

/*
doesPieceFit: Checks if a piece fits within the playing field or not.

Params:
>> pType : Type of the piece
>> pRotation : Rotation of the piece
>> pX, pY : Location of the piece in the playing field (stored as the top-left corner)
===================================================
*/
bool doesPieceFit(int pType, int pRotation, int pX, int pY)
{
    for (int i = 0; i < PIECE_DIM; i++)
    {
        for (int j = 0; j < PIECE_DIM; j++)
        {
            int pieceIndex = rotateCoord(i, j, pRotation);
            int fieldIndex = (pX + i) + (pY + j ) * FIELD_WIDTH;
            // Only check for collision if the indices are within bounds.
            // Otherwise, we might have indices out of bounds (e.g empty space can be pushed to negative indices)
            if (i + pX >= 0 && i + pX < FIELD_WIDTH 
                && j + pY >= 0 && j + pY < FIELD_HEIGHT)
            {
                if (piece[pType][pieceIndex] != '.'
                    && pField[fieldIndex] != 0)

                    return false;
            }
        }
    }
      
    // if it passes everything, return true
    return true;
}

int main()
{
    // BEGIN INIT ====================================================
    // Set dimensions of screen dynamically
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    nScreenWidth = columns;
    nScreenHeight = rows;

    // Initialise screen buffer of wchar and fill everything with empty chars
    screen = new wchar_t[nScreenHeight * nScreenWidth];
    for (int i = 0; i < nScreenHeight * nScreenWidth; i++) screen[i] = L' ';

    // Create new console screen buffer and set it as the active one
    HANDLE hConsole = CreateConsoleScreenBuffer(/*desired access*/ GENERIC_READ | GENERIC_WRITE,
                                                /* share mode */ 0,
                                                /* security attributes */ NULL,
                                                /* dwflags */ CONSOLE_TEXTMODE_BUFFER,
                                                /* ipscreen data */ NULL);
    
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Initialise playing field buffer
    pField = new unsigned char[FIELD_WIDTH * FIELD_HEIGHT];
    // Set it to 0 for an empty space, and to 9 for boundaries
    for (int x = 0; x < FIELD_WIDTH; x ++)
        for (int y = 0; y < FIELD_HEIGHT; y++)
            pField[x + y * FIELD_WIDTH] = ( x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1 ) ? 9 : 0;

    // END INIT  ===========================================================

    // GAME LOGIC ==========================================================
    // Current piece type and position
    int nCurrentPiece = rand() % 7;
    int nCurrentRotation = rand () % 4;
    int nCurrentX = (FIELD_WIDTH / 2) - 2;
    int nCurrentY = 0;

    // Check if current piece can be moved down
    bool bForceDown;
    // Array containing whether the keys 'left, right, down, z' are pressed
    bool bKey[4];
    
    // Set the block to fall down after speed ticks of 50ms each
    // To make the game more difficult, make speed lower and lower
    // difficulty ranges from 0 to 20
    int easiness = 20;
    int tickCount = 0;
    bool isGameOver = false;
    bool rotateHold = true;

    int nScore = 0;
    
    std::vector<int> vLines;

    while (!isGameOver) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        tickCount++;
        bForceDown = (easiness == tickCount);

        if (!doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY))
            isGameOver = true;

        // BEGIN: HANDLING INPUTS ==============================================================
        bKey[0] = GetAsyncKeyState(VK_LEFT);
        bKey[1] = GetAsyncKeyState(VK_RIGHT);
        bKey[2] = GetAsyncKeyState(VK_DOWN);
        bKey[3] = GetAsyncKeyState(VK_UP);

        if (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
            nCurrentX--;
        if (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))
            nCurrentX++;
        if (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
            nCurrentY++;
     
        
        // TODO: FIX THIS
        // If rotation key is pressed and it fits the next rotation
        if (bKey[3])
        {
            // if the button is held down already, then rotate
            if (rotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY))
            {
                nCurrentRotation = (nCurrentRotation + 1) % 4;
                rotateHold = false;
            }
        }
        rotateHold = true;
  
        // END: HANDLING INPUTS ================================================================
        
        // BEGIN: CHECK IF PIECE CAN MOVE DOWN =================================================
        if (bForceDown)
            if (doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
            {
                tickCount = 0;
                nCurrentY++;
            }   
            // If we can't force the current piece down, check for lines and generate new piece
            else {
                nScore += 25;
                // Add the current piece to the playing field
                for (int i = 0; i < PIECE_DIM; i++)
                    for (int j = 0; j < PIECE_DIM; j++)
                        if (piece[nCurrentPiece][rotateCoord(i, j, nCurrentRotation)] != '.')
                            // Add 1 to the current piece, since 0 gives us a blank space
                            pField[nCurrentX + i + (nCurrentY + j) * FIELD_WIDTH] = nCurrentPiece + 1;

                // Check for all rows
                for (int j = 1; j < FIELD_HEIGHT - 1; j++)
                {
                    int checkLine = 1;
                    for (int i = 1; i < FIELD_WIDTH - 1; i++)
                    {
                        if (pField[i + j * FIELD_WIDTH] != 0)
                            checkLine++;
                    }
                    if (checkLine == FIELD_WIDTH- 1)
                        vLines.push_back(j);
                }


                
                if (!vLines.empty())
                {
                    nScore += 100*vLines.size();
                    // For each line, replace that line with the line above it
                    for (auto &v : vLines)
                    {
                        for (int x = 1; x < FIELD_WIDTH - 1; x++)
                        {
                            for (int y = v; y > 0; y--)
                                pField[x + y * FIELD_WIDTH] = pField[x + (y - 1) * FIELD_WIDTH];
                            pField[x] = 0;
                        }

                    }
                    vLines.clear();
                }

                nCurrentPiece = rand() % 7;
                nCurrentX = (FIELD_WIDTH / 2) - 2;
                nCurrentY = 0;
                nCurrentRotation = rand() % 4;
                tickCount = 0;
            }
        // END: CHECK IF PIECE CAN MOVE DOWN =================================================


        // BEGIN: DRAWING TO BUFFER =============================================
        // Draw playing field
        for (int x = 0; x < FIELD_WIDTH; x++)
            for (int y = 0; y < FIELD_HEIGHT; y++)
                screen[(x + 4) + (y + 2) * nScreenWidth] = L" ABCDEFGH#"[pField[x + y * FIELD_WIDTH]];

        // Draw current block
        for (int pX = 0; pX < PIECE_DIM; pX++)
            for (int pY = 0; pY < PIECE_DIM; pY++)
                if (piece[nCurrentPiece][rotateCoord(pX, pY, nCurrentRotation)] != L'.')
                    screen[(nCurrentX + pX + 4) + (nCurrentY + pY + 2) * nScreenWidth] = nCurrentPiece + 65;
        
        swprintf_s(&screen[2 * nScreenWidth + FIELD_WIDTH + 10], 16, L"SCORE: %8d", nScore);

        // END: DRAWING TO BUFFER ===============================================

        WriteConsoleOutputCharacter(/* handle */ hConsole,
                                    /* ipcharacter */ screen,
                                    /* dword length */ nScreenHeight * nScreenWidth,
                                    /* start coord */{ 0,0 },
                                    /* nchars written*/ &dwBytesWritten);
    }
    
    // END: DRAWING TO BUFFER =============================================
    // Will not print since we are using a different handle!
    std::cout << "GAME OVER!" << std::endl;
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
