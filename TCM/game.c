#include "game.h"

int main()
{
	int input = 0;
	srand((unsigned int)time(NULL));
	do
	{
		menu();
		printf("请选择:>\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			game();
			break;
		case 2:
			printf("退出游戏中\n");
			break;
		default:
			printf("输入有误，请重新输入。\n");
			break;
		}
	} while (input != 2); //只要不输入2，就继续玩
	system("pause");
	return 0;
}
