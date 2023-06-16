#ifndef REVERSI_H
#define REVERSI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>

#define BOARD_SIZE 8

struct GameState {
    char board[BOARD_SIZE][BOARD_SIZE];
    int currentPlayer;
};

void initializeGame(struct GameState* gameState);
void printBoard(const struct GameState* gameState);
int isValidMove(const struct GameState* gameState, int row, int col);
void makeMove(struct GameState* gameState, int row, int col);

void runServer(int port);
void runClient(const char* serverIP, int port);

#endif  // REVERSI_H

