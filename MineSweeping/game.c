#define _CRT_SECURE_NO_WARNINGS 1
#include "game.h"

void menu()
{
	printf("*************************************\n");
	printf("***********     1.play     **********\n"); 
	printf("***********     2.exit     **********\n");
	printf("*************************************\n");
}

void menu2()
{
	printf("*************************************\n");
	printf("********      1.EasyMode     ********\n");
	printf("********      2.MediumMode     ******\n");
	printf("********      3.DiffcultMode*********\n");
	printf("*************************************\n");
}

void InitBoard(char Board[ROWS][COLS], int row, int col, char zf)
{
	memset(&Board[0][0], zf, row*col*sizeof(Board[0][0]));//�������ȡ����������Ԫ�ص�ַ��Ȼ������Ҫ����Ķ��������Ҫ������ֽ���
}

void DisPlay(char Board[ROWS][COLS], int row, int col)
{
	int i = 0;
	int j = 0;

	for (i = 0; i <= col; i++)
	{
		printf("%d ", i);
	}
	printf("\n");
	for (i = 1; i <= row; i++)//�����1��ʼ����Ϊ��������׵�ʱ��ǰ���ֱ�ӳɡ�0���ˣ���Ȼ���������
	{
		printf("%d ", i);
		for (j = 1; j <= col; j++)
		{
			printf("%c ", Board[i][j]);
		}
		printf("\n");
	}
}

void SetMine(char Board[ROWS][COLS], int row, int col, int m)
{
	int x = 0;
	int y = 0;

	while (m)
	{
		x = rand() % row + 1;
		y = rand() % col + 1;
		if (Board[x][y] == '0')
		{
			m--;
			Board[x][y] = '1';
		}
	}
}

int GetMine(char MineBoard[ROWS][COLS], int x, int y)
{
	return MineBoard[x][y + 1]
		+ MineBoard[x][y - 1]
		+ MineBoard[x + 1][y] 
		+ MineBoard[x - 1][y]
		+ MineBoard[x - 1][y - 1]
		+ MineBoard[x + 1][y + 1]
		+ MineBoard[x - 1][y + 1] 
		+ MineBoard[x + 1][y - 1] - 8 * '0';
}

//�˺���Ϊ����Χ�����׵ģ�����ʽչ��������˼·��
//�ȶ������������Χ�����Ų������Χ���׾���ʾ��
//Ȼ�����û���ף��ȶ����Ͻǵĵ���������Ų���Χ�İ˸������Ƿ����ף��������ʾ�м��������û��������������Ų�
//�ܵ���˵���ǣ�������������ǣ�4��3������ô�Ȳ飨3��2�������û���ף��Ͳ��������Ͻǣ��������ࣩ����3��3������3��4������4��2������4��4������5��2������5��3������5��4��
//�м���Ǹ��������ճ�������Ȼ�ͻ���ѭ��

void ExpandBoard(char MineBoard[ROWS][COLS], char ShowBoard[ROWS][COLS], int row, int col, int x, int y)
{
	int i = 0;
	int j = 0;

	if (x >= 1 && x <= row && y >= 1 && y <= col)
	{
		if (!GetMine(MineBoard, x, y))
		{
			ShowBoard[x][y] = '0';
			for (i = -1; i <= 1; i++)
			{
				for (j = -1; j <= 1; j++)
				{
					if (x + i >= 1 && x + i <= row && y + j >= 1 && y + j <= col)
					{
						if (i != 0 || j != 0)
						{
							if (ShowBoard[x + i][y + j] != '0')
							{
								ExpandBoard(MineBoard, ShowBoard, row, col, x + i, y + j);
							}
						}
					}
				}
			}
		}
		else
		{

			ShowBoard[x][y] = GetMine(MineBoard, x, y) + '0';
		}
	}
}

int Iswin(char ShowBoard[ROWS][COLS], int row, int col)
{
	int i = 0;
	int j = 0;
	int mine = 0;

	for (i = 1; i <= row; i++)
	{
		for (j = 1; j <= col; j++)
		{
			if (ShowBoard[i][j] == '*')
			{
				mine++;
			}
		}
	}
	return mine;
}

void FindMine(char MineBoard[ROWS][COLS], char ShowBoard[ROWS][COLS], int row, int col, int m)
{
	int x = 0;
	int y = 0;
	int win = 0;
	int mine = row * col;
	char ch = 0;

	while(mine > m)
	{
		printf("�������������:>");
		scanf("%d%d", &x, &y);
		while (((ch = getchar()) != '\n') && (ch != EOF))
		{ }
			if (x >= 1 && x <= row && y >= 1 && y <= col)
			{
				if (MineBoard[x][y] == '1')
				{
					if (win == 0)
					{
						MineBoard[x][y] = '0';
						SetMine(MineBoard, row, col, 1);
						ExpandBoard(MineBoard, ShowBoard, ROW, COL, x, y);
						DisPlay(ShowBoard, ROW, COL);
						mine = Iswin(ShowBoard, ROW, COL);
						win++;
					}
					else
					{
						printf("���ź����㱻ը���ˣ�\n");
						DisPlay(MineBoard, ROW, COL);
						break;
					}
				}
				else
				{
					ExpandBoard(MineBoard, ShowBoard, ROW, COL, x, y);
					DisPlay(ShowBoard, ROW, COL);
					mine = Iswin(ShowBoard, ROW, COL);
				}
			}
			else
			{
				printf("������������������...\n");
			}
		
	}
	if (mine == m)
	{
		printf("��ϲ��ͨ���ˣ�\n");
		DisPlay(MineBoard, ROW, COL);
	}
}
