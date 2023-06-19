#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <ctype.h>
#include "reversi.h"

#define BOARD_SIZE 8
#define BLACK 'B'
#define WHITE 'W'
#define EMPTY '.'

void initializeGame(struct GameState* gameState) {
    memset(gameState->board, EMPTY, sizeof(gameState->board));
    gameState->board[3][3] = BLACK;
    gameState->board[3][4] = WHITE;
    gameState->board[4][3] = WHITE;
    gameState->board[4][4] = BLACK;
    gameState->currentTurn = BLACK;
}

void printBoard(const struct GameState* gameState) {
    int i, j;
    int h;
    int startRow, startCol;

    int boardHeight = 17;
    int boardWidth = 33; 

    int windowHeight, windowWidth;
    getmaxyx(stdscr, windowHeight, windowWidth);
    startRow = (windowHeight - boardHeight) / 2;
    startCol = (windowWidth - boardWidth) / 2;

    clear();

    header(startCol, "Reversi");

    for (i = 0; i < BOARD_SIZE; i++) {
        mvprintw(startRow + i * 2 + 1, startCol - 3, "%d", i);
    }

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

    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            char stone = gameState->board[i][j];
            if (stone != EMPTY) {
                int row = startRow + i * 2 + 1;
                int col = startCol + j * 4 + 2;
                mvaddch(row, col, stone);
            }
        }
    }

    refresh();
}

void header(int startCol, const char* title) {
    int i;
    int titleLength = strlen(title);
    int headerWidth = 33;
    int startHeaderCol = startCol + (headerWidth - titleLength) / 2;

    mvprintw(0, startHeaderCol, title);
    for (i = 0; i < headerWidth; i++) {
        mvaddch(1, startCol + i, '-');
    }
}

int isValidMove(const struct GameState* gameState, int row, int col) {
    char currentPlayer = gameState->currentTurn;
    char opponentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;

    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return 0;
    }

    if (gameState->board[row][col] != EMPTY) {
        return 0;
    }

    int dr, dc;
    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            int r = row + dr;
            int c = col + dc;
            if (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && gameState->board[r][c] == opponentPlayer) {
                while (1) {
                    r += dr;
                    c += dc;
                    if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) {
                        break;
                    }
                    if (gameState->board[r][c] == currentPlayer) {
                        return 1;
                    }
                    if (gameState->board[r][c] == EMPTY) {
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

void makeMove(struct GameState* gameState, int row, int col) {
    if (!isValidMove(gameState, row, col)) {
        return;
    }

    char currentPlayer = gameState->currentTurn;
    char opponentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;

    gameState->board[row][col] = currentPlayer;

    int dr, dc;
    for (dr = -1; dr <= 1; dr++) {
        for (dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            int r = row + dr;
            int c = col + dc;
            int flipCount = 0;
            while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && gameState->board[r][c] == opponentPlayer) {
                flipCount++;
                r += dr;
                c += dc;
            }
            if (flipCount > 0 && r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && gameState->board[r][c] == currentPlayer) {
                r = row + dr;
                c = col + dc;
                while (flipCount > 0) {
                    gameState->board[r][c] = currentPlayer;
                    r += dr;
                    c += dc;
                    flipCount--;
                }
            }
        }
    }

    gameState->currentTurn = opponentPlayer;

    printBoard(gameState);
}

void runServer(int port) {
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(socket_desc, 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        perror("Accept failed");
        exit(1);
    }

    puts("Connection accepted");

    struct GameState gameState;
    initializeGame(&gameState);

    int row, black_col;
    char col;
    char message[256];
    int gameover = 0;
    bool serverTurn = false;

    while (!gameover) {
        clear();
        printBoard(&gameState);
        refresh();

        if (!serverTurn) {
            mvprintw(LINES - 1, 0, "Waiting for client's move...");
            refresh();

            if (recv(client_sock, message, sizeof(message), 0) < 0) {
                perror("Receive failed");
                exit(1);
            }

            sscanf(message, "%d,%d", &row, &black_col);
            printf("Client move: %d,%d\n", row, black_col);

            if (!isValidMove(&gameState, row, black_col)) {
                printf("Invalid move, try again.\n");
                continue;
            }

            makeMove(&gameState, row, black_col); // Update the game board
            printBoard(&gameState); // Update the board after making a move

            serverTurn = true;
        } else {
            mvprintw(LINES - 1, 0, "Enter your move (e.g., 3C): ");
            refresh();

            char userInput[256];
            echo();
            wgetstr(stdscr, userInput);
            noecho();

            if (sscanf(userInput, "%d%c", &row, &col) != 2) {
                mvprintw(LINES - 1, 0, "Invalid move, try again.     ");
                continue;
            }
            col = toupper(col) - 'A';

            snprintf(message, sizeof(message), "%d,%d", row, col);
            if (send(client_sock, message, strlen(message), 0) < 0) {
                perror("Send failed");
                exit(1);
            }

            makeMove(&gameState, row, col); // Update the game board
            printBoard(&gameState); // Update the board after making a move

            serverTurn = false; // Fix the serverTurn flag
        }

        printBoard(&gameState);
        refresh();
    }

    printBoard(&gameState); // Update the board before printing the final state
    printf("Game over!\n");

    close(client_sock);
    close(socket_desc);
}

void runClient(const char* serverIP, int port) {
    int sock;
    struct sockaddr_in server;
    char message[256];
    struct GameState gameState;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    server.sin_addr.s_addr = inet_addr(serverIP);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        exit(1);
    }

    printf("Connected to server\n");

    initializeGame(&gameState);

    int row, black_col;
    char col;
    int gameover = 0;
    bool serverTurn = true;

    while (!gameover) {
        clear();
        printBoard(&gameState);
        refresh();

        if (serverTurn) {
            mvprintw(LINES - 1, 0, "Enter your move (e.g., 3C): ");
            refresh();

            char userInput[256];
            echo();
            wgetstr(stdscr, userInput);
            noecho();

            if (sscanf(userInput, "%d%c", &row, &col) != 2) {
                mvprintw(LINES - 1, 0, "Invalid move, try again.     ");
                continue;
            }
            col = toupper(col) - 'A';

            snprintf(message, sizeof(message), "%d,%d", row, col);
            if (send(sock, message, strlen(message), 0) < 0) {
                perror("Send failed");
                exit(1);
            }

            makeMove(&gameState, row, col); // Update the game board
            printBoard(&gameState); // Update the board after making a move

            serverTurn = false;
        } else {
            mvprintw(LINES - 1, 0, "Waiting for server's move...");
            refresh();

            if (recv(sock, message, sizeof(message), 0) < 0) {
                perror("Receive failed");
                exit(1);
            }

            sscanf(message, "%d,%d", &row, &black_col);
            printf("Server move: %d,%d\n", row, black_col);

            if (!isValidMove(&gameState, row, black_col)) {
                printf("Invalid move, try again.\n");
                continue;
            }

            makeMove(&gameState, row, black_col); // Update the game board
            printBoard(&gameState); // Update the board after making a move

            serverTurn = true;
        }

        printBoard(&gameState);
        refresh();
    }

    printBoard(&gameState); // Update the board before printing the final state
    printf("Game over!\n");

    close(sock);
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
