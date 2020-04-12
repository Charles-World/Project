#include "game.h"

void menu()
{
	printf("**********************************\n");
	printf("*********     1.play    **********\n");
	printf("*********     2.exit    **********\n");
	printf("**********************************\n");
}
//打印棋盘
void Displaygame(char board[ROW][COL], int row, int col)
{
	int i, j;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			printf(" %c ", board[i][j]);
			if (j < col - 1)
			{
				printf("|");
			}
		}
		printf("\n");
		if (i < row - 1)
		{
			for (j = 0; j < col; j++)
			{
				printf("___");
				if (j < col - 1)
				{
					printf("|");
				}
			}
		}
		printf("\n");
	}
}
//初始化棋盘
void Initboard(char board[ROW][COL], int row, int col)
{
	int i, j;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			board[i][j] = ' ';
		}
	}
}
//电脑走
void Computermove(char board[ROW][COL], int row, int col)
{
	while (1)
	{
		int x = rand() % 3;
		int y = rand() % 3;
		if (board[x][y] == ' ')
		{
			board[x][y] = 'X';
			break;
		}
	}
}
// 玩家走
void playermove(char board[ROW][COL], int row, int col)
{
	int x, y;
	printf("玩家输入坐标:>\n");
	while (1)
	{
		scanf("%d%d", &x, &y);
		if (x >= 1 && x <= 3 && y >= 1 && y <= 3)
		{
			if (board[x - 1][y - 1] == ' ')
			{
				board[x - 1][y - 1] = 'Y';
				break;
			}
			else
			{
				printf("输入有误，请重新输入:>\n");
			}
		}
		else
		{
			printf("输入有误，请重新输入\n");
		}
	}
}
//判断是否平局
int Isfull(char board[ROW][COL], int row, int col)
{
	int i, j;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			if (board[i][j] == ' ')
			{
				return 0;
			}
		}
	}
	return 1;
}
//判断是否输赢
char Iswin(char board[ROW][COL], int row, int col)
{
	int i, ret;
	for (i = 0; i < col; i++)
	{
		if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ')
		{
			return board[i][0];
		}
	}
	for (i = 0; i < row; i++)
	{
		if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ')
		{
			return board[0][i];
		}
	}
	if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ')
	{
		return board[1][1];
	}
	if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[2][0] != ' ')
	{
		return board[1][1];
	}
	if (ret = Isfull(board, ROW, COL) == 1)
	{
		return 'Q';
	}
	return ' ';
}
//运行游戏的代码
void game()
{
	int ret;
	char board[ROW][COL] = { 0 };
	printf("游戏即将开始，请稍候...\n");
	Initboard(board, ROW, COL);
	Displaygame(board, ROW, COL);
	while (1)
	{
		printf("电脑走:>\n");
		Computermove(board, ROW, COL);
		Displaygame(board, ROW, COL);
		if ((ret = Iswin(board, ROW, COL)) != ' ')
		{
			break;
		}
		printf("玩家走:>\n");
		playermove(board, ROW, COL);
		Displaygame(board, ROW, COL);
		Iswin(board, ROW, COL);
		if ((ret = Iswin(board, ROW, COL)) != ' ')
		{
			break;
		}
	}
	if (ret == 'X')
	{
		printf("电脑胜利\n");
	}
	else if (ret == 'Y')
	{
		printf("玩家胜利\n");
	}
	else
	{
		printf("平局\n");
	}
}
