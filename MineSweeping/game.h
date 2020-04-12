#ifndef __game_h__
#define __game_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ROW 9
#define COL 9
#define ROWS ROW + 2
#define COLS COL + 2
#define EasyMode 10
#define MediumMode 20
#define DifficultMode 79

void menu();
void menu2();
void InitBoard(char Board[ROWS][COLS], int row, int col, char zf);
void DisPlay(char Board[ROWS][COLS], int row, int col);
void SetMine(char Board[ROWS][COLS], int row, int col, int m);
int GetMine(char MineBoard[ROWS][COLS], int x, int y);
void ExpandBoard(char MineBoard[ROWS][COLS], char ShowBoard[ROWS][COLS], int row, int col, int x, int y);
int Iswin(char ShowBoard[ROWS][COLS], int row, int col);
void FindMine(char MineBoard[ROWS][COLS], char ShowBoard[ROWS][COLS], int row, int col, int m);

#endif//__game_h__