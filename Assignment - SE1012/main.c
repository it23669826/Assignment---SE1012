/* SE1012 – Tic-Tac-Toe (N x N)
   Modes: 1) User vs User  2) User vs Computer  3) Three players (X,O,Z)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>   /* fgets + simple string tweaks (Lecture 8) */

#define MIN_SIZE     3
#define MAX_SIZE    10
#define MAX_PLAYERS  3
#define LOG_MOVES    1   /* set to 0 if you want to compile without logging */

typedef unsigned int uint;

/* ---- function headers ---- */
char **create_board(int n);
void   clear_board(char **b, int n);
void   free_board(char **b, int n);

void   show_board(char **b, int n);
bool   is_valid_move(char **b, int n, int r, int c);
bool   is_board_full(char **b, int n);
bool   has_won(char **b, int n, char s);

int    read_int_in_range(const char *msg, int lo, int hi);
void   read_human_move(int n, int *r0, int *c0);   /* accepts "1 2" or "1,2" */
void   human_move(char **b, int n, int *r0, int *c0);
void   computer_move(char **b, int n, int *r0, int *c0);

void   log_state(FILE *fp, int move_no, const char *who, char sym,
                 int row1, int col1, char **b, int n);

/* ---- main ---- */
int main(void) {
    int n = read_int_in_range("Enter board size (3..10): ", MIN_SIZE, MAX_SIZE);

    printf("\nChoose mode:\n");
    printf("  1) Two Players (User vs User)\n");
    printf("  2) User vs Computer\n");
    printf("  3) Three Players (X, O, Z) with roles\n");
    int mode = read_int_in_range("Your choice (1..3): ", 1, 3);

    int total_players = (mode == 3) ? 3 : 2;
    char sym[MAX_PLAYERS]  = {'X','O','Z'};
    char role[MAX_PLAYERS] = {'H','H','H'};  /* H=Human, C=Computer */

    if (mode == 2) { role[0] = 'H'; role[1] = 'C'; }
    if (mode == 3) {
        int human_count = 0;
        for (int i = 0; i < total_players; i++) {
            char line[32];
            printf("Is Player %d (%c) Human (H) or Computer (C)? ", i+1, sym[i]);
            if (!fgets(line, sizeof line, stdin)) return 0;
            char ch = line[0];
            if (ch=='h' || ch=='H') { role[i]='H'; human_count++; }
            else                     { role[i]='C'; }
        }
        if (human_count == 0) {  /* make at least one human */
            printf("Making Player 1 (X) Human.\n");
            role[0] = 'H';
        }
    }

    char **board = create_board(n);
    if (!board) { puts("Memory error."); return 1; }

#if LOG_MOVES
    FILE *logf = fopen("game_log.txt","w");
    if (!logf) { puts("Could not open game_log.txt"); free_board(board,n); return 1; }
#else
    FILE *logf = NULL;
#endif

    srand((unsigned)time(NULL));

    int turn = 0, move_no = 1;
    while (1) {
        int p = turn % total_players;
        char s = sym[p];

        show_board(board, n);
        printf("Player %d (%c) [%s]\n", p+1, s, (role[p]=='H')?"Human":"Computer");

        int r0=-1, c0=-1;
        if (role[p]=='H') {
            human_move(board, n, &r0, &c0);
        } else {
            computer_move(board, n, &r0, &c0);
            printf("Computer placed %c at (%d,%d)\n", s, r0+1, c0+1);
        }

        board[r0][c0] = s;
        log_state(logf, move_no, (role[p]=='H')?"Human":"Computer",
                  s, r0+1, c0+1, board, n);
        move_no++;

        if (has_won(board, n, s)) {
            show_board(board, n);
            printf("Player %d (%c) wins!\n", p+1, s);
            break;
        }
        if (is_board_full(board, n)) {
            show_board(board, n);
            puts("It's a draw.");
            break;
        }
        turn++;
    }

    if (logf) fclose(logf);
    free_board(board, n);
    return 0;
}

/* ---- board helpers ---- */
char **create_board(int n) {
    /* make an n x n grid on the heap */
    char **b = (char**)malloc(n * sizeof *b);
    if (!b) return NULL;
    for (int i = 0; i < n; i++) {
        b[i] = (char*)malloc(n * sizeof *b[i]);
        if (!b[i]) {
            for (int k = 0; k < i; k++) free(b[k]);
            free(b);
            return NULL;
        }
    }
    clear_board(b, n);
    return b;
}

void clear_board(char **b, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            b[i][j] = ' ';
}

void free_board(char **b, int n) {
    if (!b) return;
    for (int i = 0; i < n; i++) free(b[i]);
    free(b);
}

/* ---- drawing ---- */
void show_board(char **b, int n) {
    /* header */
    printf("\n    ");
    for (int j = 0; j < n; j++) printf("%2d ", j+1);
    printf("\n");

    for (int i = 0; i < n; i++) {
        printf("   +");
        for (int j = 0; j < n; j++) printf("---+");
        printf("\n");

        printf("%2d |", i+1);
        for (int j = 0; j < n; j++) printf(" %c |", b[i][j]);
        printf("\n");
    }
    printf("   +");
    for (int j = 0; j < n; j++) printf("---+");
    printf("\n\n");
}

/* ---- rules ---- */
bool is_valid_move(char **b, int n, int r, int c) {
    return (r >= 0 && r < n && c >= 0 && c < n && b[r][c] == ' ');
}

bool is_board_full(char **b, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (b[i][j] == ' ') return false;
    return true;
}

bool has_won(char **b, int n, char s) {
    /* rows and cols */
    for (int i = 0; i < n; i++) {
        bool row_ok = true, col_ok = true;
        for (int j = 0; j < n; j++) {
            if (b[i][j] != s) row_ok = false;
            if (b[j][i] != s) col_ok = false;
        }
        if (row_ok || col_ok) return true;
    }
    /* diagonals */
    bool d1 = true, d2 = true;
    for (int i = 0; i < n; i++) {
        if (b[i][i] != s)           d1 = false;
        if (b[i][n-1-i] != s)       d2 = false;
    }
    return d1 || d2;
}

/* ---- input ---- */
int read_int_in_range(const char *msg, int lo, int hi) {
    char line[64];
    int x;
    while (1) {
        printf("%s", msg);
        if (!fgets(line, sizeof line, stdin)) continue;
        if (sscanf(line, "%d", &x) == 1 && x >= lo && x <= hi) return x;
        printf("Enter a number between %d and %d.\n", lo, hi);
    }
}

void read_human_move(int n, int *r0, int *c0) {
    char line[64];
    while (1) {
        printf("Enter move (row col) or (row,col) [1..%d]: ", n);
        if (!fgets(line, sizeof line, stdin)) continue;

        /* allow comma: turn commas into spaces */
        for (char *p = line; *p; ++p) if (*p == ',') *p = ' ';

        int r=-1, c=-1;
        if (sscanf(line, "%d %d", &r, &c) == 2 &&
            r >= 1 && r <= n && c >= 1 && c <= n) {
            *r0 = r - 1;
            *c0 = c - 1;
            return;
        }
        puts("Bad input. Try again.");
    }
}

void human_move(char **b, int n, int *r0, int *c0) {
    while (1) {
        int r, c;
        read_human_move(n, &r, &c);
        if (is_valid_move(b, n, r, c)) { *r0 = r; *c0 = c; return; }
        puts("Cell in use. Pick another.");
    }
}

void computer_move(char **b, int n, int *r0, int *c0) {
    if (is_board_full(b, n)) { *r0 = *c0 = 0; return; }
    int r, c;
    do { r = rand() % n; c = rand() % n; } while (!is_valid_move(b, n, r, c));
    *r0 = r; *c0 = c;
}

/* ---- logging (simple text) ---- */
void log_state(FILE *fp, int move_no, const char *who, char sym,
               int row1, int col1, char **b, int n) {
    if (!fp) return;                      /* can be compiled out */
    fprintf(fp, "Move %d: %s (%c) -> (%d,%d)\n", move_no, who, sym, row1, col1);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            char ch = b[i][j];
            fputc(ch == ' ' ? '.' : ch, fp);
            if (j < n - 1) fputc(' ', fp);
        }
        fputc('\n', fp);
    }
    fputs("-----\n", fp);
    fflush(fp);
}
