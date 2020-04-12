#include "game.h"

void menu()
{
	printf("**********************************\n");
	printf("*********     1.play    **********\n");
	printf("*********     2.exit    **********\n");
	printf("**********************************\n");
}
//��ӡ����
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
//��ʼ������
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
//������
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
// �����
void playermove(char board[ROW][COL], int row, int col)
{
	int x, y;
	printf("�����������:>\n");
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
				printf("������������������:>\n");
			}
		}
		else
		{
			printf("������������������\n");
		}
	}
}
//�ж��Ƿ�ƽ��
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
//�ж��Ƿ���Ӯ
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
//������Ϸ�Ĵ���
void game()
{
	int ret;
	char board[ROW][COL] = { 0 };
	printf("��Ϸ������ʼ�����Ժ�...\n");
	Initboard(board, ROW, COL);
	Displaygame(board, ROW, COL);
	while (1)
	{
		printf("������:>\n");
		Computermove(board, ROW, COL);
		Displaygame(board, ROW, COL);
		if ((ret = Iswin(board, ROW, COL)) != ' ')
		{
			break;
		}
		printf("�����:>\n");
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
		printf("����ʤ��\n");
	}
	else if (ret == 'Y')
	{
		printf("���ʤ��\n");
	}
	else
	{
		printf("ƽ��\n");
	}
}
