#ifndef REVERSI_H
#define REVERSI_H

#define BOARD_SIZE 8
#define BLACK 'B'
#define WHITE 'W'
#define EMPTY '.'

struct GameState {
    char board[BOARD_SIZE][BOARD_SIZE];
    char currentTurn;
};

void initializeGame(struct GameState* gameState);
void printBoard(const struct GameState* gameState);
int isValidMove(const struct GameState* gameState, int row, int col);
void makeMove(struct GameState* gameState, int row, int col);
void runServer(int port);
void runClient(const char* serverIP, int port);

#endif  // REVERSI_H
