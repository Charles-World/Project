#pragma once
//дһ����������Ϸ
//1.��ʼ������
//2.��ӡ������
//3.�õ�����
//4.�����
//5.�ж���Ӯ��ƽ��
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

