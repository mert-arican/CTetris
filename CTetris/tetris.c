//
//  main.c
//  CTetris
//
//  Created by Mert Arıcan on 20.12.2019.
//  Copyright © 2019 Mert Arıcan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "tetris.h"

bool gameOver = false;
bool shouldAddNewTetrimino = true;
int N = 3;
volatile int score = 0;
enum TetriminoType nextStep;

TetrisBoard *board;
Tetrimino *currentTetrimino;
Tetrimino *nextStepTetrimino;
Tetrimino *previewTetrimino;
pthread_mutex_t aLock;
pthread_t one, two;

void rotateNinetyDegree(Tetrimino *source_tetrimino, TetrisBoard *board) {
    set(nextStepTetrimino, source_tetrimino);
    int element_count = 0;
    for (int s_col = 0; s_col < N; s_col++) {
        for (int s_row = N-1; s_row >= 0; s_row--) {
            int r_row = element_count / N;
            int r_col = element_count % N;
            nextStepTetrimino->squares[r_row][r_col] = source_tetrimino->squares[s_row][s_col];
            element_count++;
        }
    }
    if (nextStepIsValid(nextStepTetrimino, board)) {
        updateBoardAfterMove(source_tetrimino, nextStepTetrimino, board);
    }
}

void rotate(Tetrimino *tetrimino, bool clockwise, int times, TetrisBoard *board) {
    if (tetrimino->tetrimino_type != O) {
        int rotationValue = (clockwise) ? (times % 4) : ((4 - times) % 4);
        for (int i = 0; i < rotationValue; i++) {
            rotateNinetyDegree(tetrimino, board);
        }
    }
}

void setTetriminosColor(Tetrimino *tetrimino, enum Color color) {
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            tetrimino->squares[row][column].color = color;
        }
    }
}

void drawBoard(TetrisBoard *board) {
    char sqrStr[10];
    printf("\n");
    for (int row = 0; row < 20; row++) {
        printf("                                            ");
        for (int column = 0; column < 10; column++) {
            Square square = board->boardSquares[row][column];
            strcpy(sqrStr, colorSquares[square.color]);
            switch (square.type) {
                case full: printf("%s", sqrStr); break;
                case empty: printf("⬜"); break;
                case current: printf("%s", sqrStr); break;
                case preview: printf("🆕"); break;
            }
        }
        printf("\n");
    }
    printf("\n                                                  Score: %d\n", score);
}

void determineNextStep() {
    nextStep = rand() % 7;
}

void drawIncomingTetrimino() {
    Tetrimino incoming;
    int M = (nextStep == 0) ? 4 : 3;
    for (int i = 0; i < 4; i++) {
        int row = positions[nextStep][i].row;
        int column = positions[nextStep][i].column;
        incoming.squares[row][column].type = current;
        incoming.squares[row][column].color = (int) nextStep;
    }
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    for (int row = 0; row < M; row++) {
        printf("                                                    ");
        for (int column = 0; column < M; column++) {
            if (incoming.squares[row][column].type == current) {
                printf("%s", colorSquares[nextStep]);
            } else {
                printf("⬜");
            }
        }
        printf("\n");
    }
}

void drawRandomTetrimino(Tetrimino *tetrimino) {
    N = nextStep == I ? 4 : 3;
    for (int i = 0; i < 4; i++) {
        int row = positions[nextStep][i].row;
        int column = positions[nextStep][i].column;
        tetrimino->squares[row][column].type = current;
    }
    setTetriminosColor(tetrimino, (int) nextStep);
    tetrimino->tetrimino_type = nextStep;
    determineNextStep();
}

bool addTetriminoToBoard(Tetrimino *tetrimino, TetrisBoard *board) {
    if (!nextStepIsValid(tetrimino, board)) { return false; }
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++) {
            Square square = tetrimino->squares[row][column];
            int rowIndex = tetrimino->row + row;
            int columnIndex = tetrimino->column + column;
            if (board->boardSquares[rowIndex][columnIndex].type != full) {
                board->boardSquares[rowIndex][columnIndex] = square;
            }
        }
    }
    return true;
}

void initializeBoard(TetrisBoard *board) {
    for (int row = 0; row < 20; row++) {
        for (int column = 0; column < 10; column++) {
            Square square = { empty, white };
            board->boardSquares[row][column] = square;
        }
    }
}

void set(Tetrimino *given_tetrimino, Tetrimino *source_tetrimino) {
    given_tetrimino->tetrimino_type = source_tetrimino->tetrimino_type;
    given_tetrimino->row = source_tetrimino->row;
    given_tetrimino->column = source_tetrimino->column;
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            given_tetrimino->squares[row][column] = source_tetrimino->squares[row][column];
            if (given_tetrimino == previewTetrimino && source_tetrimino->squares[row][column].type == current) {
                given_tetrimino->squares[row][column].type = preview;
            }
        }
    }
}

bool nextStepTetriminoIsOutOfBounds(Tetrimino *nextStepTetrimino) {
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            Square square = nextStepTetrimino->squares[row][column];
            int yP = nextStepTetrimino->row + row;
            int xP = nextStepTetrimino->column + column;
            if (((xP < 0 && square.type == current) || (xP > 9 && square.type == current)) || (yP > 19 && square.type == current)) {
                return true;
            }
        }
    }
    return false;
}

bool thereIsCollisionWithNextStepTetrimino(Tetrimino *nextStepTetrimino, TetrisBoard *board) {
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            int nRow = nextStepTetrimino->row + row;
            int nColumn = nextStepTetrimino->column + column;
            Square currentSquare = nextStepTetrimino->squares[row][column];
            Square squareAtGivenPosition = board->boardSquares[nRow][nColumn];
            if (currentSquare.type == current && squareAtGivenPosition.type == full) {
                return true;
            }
        }
    }
    return false;
}

bool nextStepIsValid(Tetrimino *nextStepTetrimino, TetrisBoard *board) {
    return !nextStepTetriminoIsOutOfBounds(nextStepTetrimino) && !thereIsCollisionWithNextStepTetrimino(nextStepTetrimino, board);
}

void initializeTetrimino(Tetrimino *tetrimino) {
    tetrimino->row = initialRow; tetrimino->column = initialColumn;
    for (int row = 0; row < 4; row++) {
        for (int column = 0; column < 4; column++) {
            Square square = { empty, white };
            tetrimino->squares[row][column] = square;
        }
    }
}

void updateBoardAfterMove(Tetrimino *oldTetrimino, Tetrimino *newTetrimino, TetrisBoard *board) {
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            int oldCol = oldTetrimino->column + column;
            int oldRow = oldTetrimino->row + row;
            if (board->boardSquares[oldRow][oldCol].type != full) {
                board->boardSquares[oldRow][oldCol].type = empty;
            }
        }
    }
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            int newCol = newTetrimino->column + column;
            int newRow = newTetrimino->row + row;
            if (board->boardSquares[newRow][newCol].type != full) {
                board->boardSquares[newRow][newCol] = newTetrimino->squares[row][column];
            }
        }
    }
    set(oldTetrimino, newTetrimino);
}

void moveTetrimino(enum Direction direction, Tetrimino *givenTetrimino, Tetrimino *nextStepTetrimino, TetrisBoard *board) {
    set(nextStepTetrimino, givenTetrimino);
    switch (direction) {
        case left: nextStepTetrimino->column = nextStepTetrimino->column-1; break;
        case right: nextStepTetrimino->column = nextStepTetrimino->column+1; break;
        case down: nextStepTetrimino->row = nextStepTetrimino->row+1; break;
    }
    if (nextStepIsValid(nextStepTetrimino, board)) {
        updateBoardAfterMove(givenTetrimino, nextStepTetrimino, board);
    } else {
        if (direction == down) {
            // End tetriminos fall.
            for (int row = 0; row < N; row++) {
                for (int column = 0; column < N; column++) {
                    int brow = givenTetrimino->row + row;
                    int bcol = givenTetrimino->column + column;
                    if (board->boardSquares[brow][bcol].type == current) {
                        board->boardSquares[brow][bcol].type = full;
                    }
                }
            }
            shouldAddNewTetrimino = true;
        }
        //Do nothing
    }
}

Result areThereAnyMatch(TetrisBoard *board) {
    Result matchResult;
    matchResult.numberOfMatchedRows = 0;
    for (int row = 0; row < 20; row++) {
        int numberOfFullSquares = 0;
        for (int column = 0; column < 10; column++) {
            if (board->boardSquares[row][column].type == full) {
                numberOfFullSquares++;
            }
            if (numberOfFullSquares == 10) {
                matchResult.isMatch = true;
                matchResult.matchedRows[matchResult.numberOfMatchedRows] = row;
                matchResult.numberOfMatchedRows++;
            }
        }
    }
    return matchResult;
}

int getShiftValueForSquares(int atRow, Result result) {
    int shiftValue = 0;
    for (int i = 0; i < result.numberOfMatchedRows; i++) {
        if (result.matchedRows[i] > atRow) { shiftValue++; }
    }
    return shiftValue;
}

void updateBoardAfterMatch(Result matchResult, TetrisBoard *board) {
    // Emptying matched rows.
    for (int i = 0; i < matchResult.numberOfMatchedRows; i++) {
        int row = matchResult.matchedRows[i];
        for (int column = 0; column < 10; column++) {
            board->boardSquares[row][column].type = empty;
        }
    }
    // Shift squares which are higher than the matched rows.
    int shiftValue = matchResult.numberOfMatchedRows;
    for (int effectedRowIndex = matchResult.matchedRows[shiftValue-1]; effectedRowIndex > 0; effectedRowIndex--) {
        for (int column = 0; column < 10; column++) {
            shiftValue = getShiftValueForSquares(effectedRowIndex, matchResult);
            if (board->boardSquares[effectedRowIndex][column].type == full) {
                board->boardSquares[effectedRowIndex][column].type = empty;
                board->boardSquares[effectedRowIndex+shiftValue][column].type = full;
                board->boardSquares[effectedRowIndex+shiftValue][column].color = board->boardSquares[effectedRowIndex][column].color;
                board->boardSquares[effectedRowIndex][column].color = white;
            }
        }
    }
    score += (matchResult.numberOfMatchedRows * matchResult.numberOfMatchedRows) * 10;
}

void cleanPreviews() {
    for (int row = 0; row < 20; row++) {
        for (int column = 0; column < 10; column++) {
            if (board->boardSquares[row][column].type == preview) {
                board->boardSquares[row][column].type = empty;
            }
        }
    }
}

void determinePreview(Tetrimino *previewTetrimino, TetrisBoard *board) {
    set(previewTetrimino, currentTetrimino);
    bool nextPreviewIsValid = true;
    while (nextPreviewIsValid) {
        previewTetrimino->row++;
        for (int row = 0; row < N; row++) {
            for (int column = 0; column < N; column++) {
                int brow = previewTetrimino->row + row;
                int bcol = previewTetrimino->column + column;
                Square currentSquare = previewTetrimino->squares[row][column];
                Square squareAtGivenPosition = board->boardSquares[brow][bcol];
                if ((squareAtGivenPosition.type == full && currentSquare.type == preview) || (currentSquare.type == preview && brow > 19)) { nextPreviewIsValid = false; }
            }
        }
    }
    previewTetrimino->row--;
    drawPreview(previewTetrimino, board);
    drawIncomingTetrimino();
}

void drawPreview(Tetrimino *previewTetrimino, TetrisBoard *board) {
    cleanPreviews();
    for (int row = 0; row < N; row++) {
        for (int column = 0; column < N; column++) {
            int board_row = previewTetrimino->row + row;
            int board_column = previewTetrimino->column + column;
            if (previewTetrimino->squares[row][column].type == preview && board->boardSquares[board_row][board_column].type != full && board->boardSquares[board_row][board_column].type != current) {
                board->boardSquares[board_row][board_column].type = preview;
            }
        }
    }
}

void ifShouldAddNewTetrimino() {
    if (shouldAddNewTetrimino) {
        initializeTetrimino(currentTetrimino);
        initializeTetrimino(previewTetrimino);
        set(nextStepTetrimino, currentTetrimino);
        drawRandomTetrimino(currentTetrimino);
        shouldAddNewTetrimino = false;
        if (!addTetriminoToBoard(currentTetrimino, board)) {
            free(board); free(currentTetrimino); free(nextStepTetrimino); free(previewTetrimino);
            gameOver = true; printf("Game Over!");
        }
    }
}

void threadFunc(void *arg) {
    while(!gameOver) {
        sleep(1);
        if (!gameOver) {
            pthread_detach(two);
            ifShouldAddNewTetrimino();
            determinePreview(previewTetrimino, board);
            drawBoard(board);
            if (!gameOver) {
                moveTetrimino(down, currentTetrimino, nextStepTetrimino, board);
                Result result = areThereAnyMatch(board);
                if (result.isMatch) {
                    updateBoardAfterMatch(result, board);
                }
                determinePreview(previewTetrimino, board);
                drawBoard(board);
                pthread_create(&two, NULL, (void*)&threadFunc2, NULL);
            }
        }
    }
}

void threadFunc2(void *arg) {
    while(!gameOver) {
        ifShouldAddNewTetrimino();
        if (!gameOver) {
            determinePreview(previewTetrimino, board);
            drawBoard(board);
            char c;
            scanf("%s", &c);
            if (!gameOver) {
                if (c == 'a') { moveTetrimino(left, currentTetrimino, nextStepTetrimino, board); }
                else if (c == 'd') { moveTetrimino(right, currentTetrimino, nextStepTetrimino, board); }
                else if (c == 's') { while (nextStepIsValid(nextStepTetrimino, board)) { moveTetrimino(down, currentTetrimino, nextStepTetrimino, board); } }
                else if (c == 'r') { rotate(currentTetrimino, true, 1, board); }
                else if (c == 'e') { rotate(currentTetrimino, false, 1, board); }
                Result result = areThereAnyMatch(board);
                if (result.isMatch) {
                    updateBoardAfterMatch(result, board);
                }
                determinePreview(previewTetrimino, board);
                drawBoard(board);
            }
        }
    }
}

int main(int argc, const char * argv[]) {
    srand((unsigned int)(time(NULL)));
    board = malloc(sizeof(TetrisBoard));
    initializeBoard(board);
    currentTetrimino = malloc(sizeof(Tetrimino));
    nextStepTetrimino = malloc(sizeof(Tetrimino));
    previewTetrimino = malloc(sizeof(Tetrimino));
    determineNextStep();
    
    pthread_mutex_init(&aLock, NULL);
    
    pthread_create(&one, NULL, (void*)&threadFunc, NULL);
    pthread_create(&two, NULL, (void*)&threadFunc2, NULL);
    
    pthread_join(one, NULL);
    pthread_join(two, NULL);
    
    return 0;
}
