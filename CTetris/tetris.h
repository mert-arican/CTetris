//
//  tetris.h
//  CTetris
//
//  Created by Mert Arıcan on 22.12.2019.
//  Copyright © 2019 Mert Arıcan. All rights reserved.
//

#ifndef tetris_h
#define tetris_h
#include <stdbool.h>

enum SquareType { full, empty, current, preview };

enum TetriminoType { I, O, T, S, Z, J, L };

enum Direction { left, right, down };

enum Color { brown, yellow, purple, green, red, blue, orange, white };

typedef struct square {
    
    enum SquareType type;
    
    enum Color color;
    
} Square;

typedef struct tetrimino {
    
    int row;
    
    int column;
    
    Square squares[4][4];
    
    enum TetriminoType tetrimino_type;
    
} Tetrimino;

typedef struct tetris_board {
    
    Square boardSquares[20][10];
    
} TetrisBoard;

typedef struct result {
    
    int numberOfMatchedRows;
    
    int matchedRows[4];
    
    bool isMatch;
    
} Result;

typedef struct position {
    
    int row;
    
    int column;
    
} Position;

const Position positions[7][4] = { { {1, 0}, {1, 1}, {1, 2}, {1, 3} }, { {0, 1}, {0, 2}, {1, 1}, {1, 2} }, { {0, 1}, {1, 0}, {1, 1}, {1, 2} }, { {0, 1}, {0, 2}, {1, 0}, {1, 1} }, { {0, 0}, {0, 1}, {1, 1}, {1, 2} }, { {0, 0}, {1, 0}, {1, 1}, {1, 2} }, { {0, 2}, {1, 0}, {1, 1}, {1, 2} } };
const char colorSquares[8][10] = { {"🟫"}, {"🟨"}, {"🟪"}, {"🟩"}, {"🟥"}, {"🟦"}, {"🟧"}, {"⬜"} };
const int initialColumn = 4;
const int initialRow = 0;

void initializeBoard(TetrisBoard *board);
void initializeTetrimino(Tetrimino *tetrimino);
void determineNextStep(void);
void drawBoard(TetrisBoard *board);
void setTetriminosColor(Tetrimino *tetrimino, enum Color color);
void drawRandomTetrimino(Tetrimino *tetrimino);
bool addTetriminoToBoard(Tetrimino *tetrimino, TetrisBoard *board);
void set(Tetrimino *given_tetrimino, Tetrimino *source_tetrimino);
bool nextStepTetriminoIsOutOfBounds(Tetrimino *nextStepTetrimino);
bool thereIsCollisionWithNextStepTetrimino(Tetrimino *nextStepTetrimino, TetrisBoard *board);
bool nextStepIsValid(Tetrimino *nextStepTetrimino, TetrisBoard *board);
void moveTetrimino(enum Direction direction, Tetrimino *givenTetrimino, Tetrimino *nextStepTetrimino, TetrisBoard *board);
void updateBoardAfterMove(Tetrimino *oldTetrimino, Tetrimino *newTetrimino, TetrisBoard *board);
void rotateNinetyDegree(Tetrimino *source_tetrimino, TetrisBoard *board);
void rotate(Tetrimino *tetrimino, bool clockwise, int times, TetrisBoard *board);
Result areThereAnyMatch(TetrisBoard *board);
int getShiftValueForSquares(int atRow, Result result);
void updateBoardAfterMatch(Result matchResult, TetrisBoard *board);
void cleanPreviews(void);
void determinePreview(Tetrimino *previewTetrimino, TetrisBoard *board);
void drawPreview(Tetrimino *previewTetrimino, TetrisBoard *board);
void drawIncomingTetrimino(void);
void ifShouldAddNewTetrimino(void);
void threadFunc(void *arg);
void threadFunc2(void *arg);
#endif /* tetris_h */
