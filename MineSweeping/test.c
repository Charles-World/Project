#define _CRT_SECURE_NO_WARNINGS 1
#include "game.h"

void game()
{
	int n = 0;
	int m = 0;
	int MineBoard[ROWS][COLS] = { 0 };
	int ShowBoard[ROWS][COLS] = { 0 };

	InitBoard(MineBoard, ROWS, COLS,'0');
	InitBoard(ShowBoard, ROWS, COLS, '*');
	DisPlay(ShowBoard, ROW, COL);
	printf("��ѡ����Ϸģʽ:>\n");
	menu2();
	do
	{
		scanf("%d", &n);
		switch (n)//�ǵ�һ��Ҫbreak������Ȼÿ�ζ���79����
		{
		case 1:
			m = EasyMode;
			break;
		case 2:
			m = MediumMode;
			break;
		case 3:
			m = DifficultMode;
			break;
		default:
			printf("�����������������...\n");
		}
	} while (n != (1 || 2 || 3));
	SetMine(MineBoard, ROW, COL, m);
	//DisPlay(MineBoard, ROW, COL);
	DisPlay(ShowBoard, ROW, COL);
	FindMine(MineBoard, ShowBoard, ROW, COL, m);
}

int main()
{
	int input = 0;
	
	srand((unsigned int) time(NULL));
	do
	{
		menu();
		printf("��ѡ��:>\n");
  	    scanf("%d", &input);
		switch (input)
		{
		case 1:
			game();
			break;
		case 2:
			printf("��Ϸ�����˳���...\n");
			break;
		default:
			printf("�����������������...\n");
			break;
		}
	} while (input != 2);

	system("pause");
	return 0;
}