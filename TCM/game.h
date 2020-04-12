#pragma once
//写一个三字棋游戏
//1.初始化棋盘
//2.打印出棋盘
//3.让电脑走
//4.玩家走
//5.判断输赢或平局
#define _CRT_SECURE_NO_WARNINGS 1


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROW 3
#define COL 3

void game();
void menu();
void Displaygame(char board[ROW][COL], int row, int col);
void Initboard(char board[ROW][COL], int row, int col);
void Computermove(char board[ROW][COL], int row, int col);
void playermove(char board[ROW][COL], int row, int col);
int Isfull(char board[ROW][COL], int row, int col);
char Iswin(char board[ROW][COL], int row, int col);

