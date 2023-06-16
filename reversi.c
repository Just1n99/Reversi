#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>

#define BOARD_SIZE 8
#define BLACK 'B'
#define WHITE 'W'
#define EMPTY '.'

struct GameState {
    char board[BOARD_SIZE][BOARD_SIZE];
    char currentTurn;
};

void initializeGame(struct GameState* gameState) {
    memset(gameState->board, EMPTY, sizeof(gameState->board));
    gameState->board[3][3] = WHITE;
    gameState->board[3][4] = BLACK;
    gameState->board[4][3] = BLACK;
    gameState->board[4][4] = WHITE;
    gameState->currentTurn = BLACK;
}

void printBoard(const struct GameState* gameState) {
    int i, j;
    int h;
    int startRow, startCol;

    int boardHeight = 17; // Height of the board
    int boardWidth = 33; // Width of the board

    // Calculate the starting row and column to center the board
    int windowHeight, windowWidth;
    getmaxyx(stdscr, windowHeight, windowWidth);
    startRow = (windowHeight - boardHeight) / 2;
    startCol = (windowWidth - boardWidth) / 2;

    // Clear the screen
    clear();

    // Draw row labels
    for (i = 0; i < BOARD_SIZE; i++) {
        mvprintw(startRow + i * 2 + 1, startCol - 3, "%d", i);
    }

    // Draw column labels
    for (j = 0; j < BOARD_SIZE; j++) {
        mvprintw(startRow - 1, startCol + j * 4 + 2, "%c", 'A' + j);
    }

    for (i = 0; i <= 16; i++) {
        mvaddch(startRow + i, startCol, '|');
        mvaddch(startRow + i, startCol + 4, '|');
        mvaddch(startRow + i, startCol + 8, '|');
        mvaddch(startRow + i, startCol + 12, '|');
        mvaddch(startRow + i, startCol + 16, '|');
        mvaddch(startRow + i, startCol + 20, '|');
        mvaddch(startRow + i, startCol + 24, '|');
        mvaddch(startRow + i, startCol + 28, '|');
        mvaddch(startRow + i, startCol + 32, '|');

        if (i % 2 == 0) {
            for (h = 0; h <= 32; h++) {
                mvaddch(startRow + i, startCol + h, '-');
            }
        }
    }

    // Draw stones on the board
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            char stone = gameState->board[i][j];
            if (stone != EMPTY) {
                int row = startRow + i * 2 + 1;
                int col = startCol + j * 4 + 2;
                mvprintw(row, col, "%c", stone);
            }
        }
    }

    // Refresh the screen to display the changes
    refresh();
}


int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-server") == 0) {
        int port = atoi(argv[2]);
        initscr();
        runServer(port);
        endwin();
    } else if (strcmp(argv[1], "-client") == 0) {
        const char* serverIP = argv[2];
        int port = atoi(argv[3]);
        initscr();
        runClient(serverIP, port);
        endwin();
    } else {
        printf("Invalid option\n");
        return 1;
    }

    return 0;
}
