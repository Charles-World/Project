#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
void menu()
{
	printf("******************************\n");
	printf("******欢迎来到猜数字游戏******\n");
	printf("************1.start***********\n");
	printf("************2.exit ***********\n");
	printf("******************************\n");
}
int game()
{
	int a, b, i;
	printf("您最多可以输入20次\n");
	printf("游戏开始\n");
	b = rand() % 100 + 1;
	for (i = 1; i < 21; i++)
	{
		printf("请输入一个数字>:");
		scanf("%d", &a);
		if (a < b)
		{
			printf("猜小了\n");
		}
		else if (a > b)
		{
			printf("猜大了\n");
		}
		else
		{
			printf("恭喜你，猜对了\n");
			printf("如果还想(不想)继续玩的话，请输入>:1(2)\n");
			break;
		}
	}
}
int main()
{

	int a;
	menu();
	printf("请输入>:");
	srand((unsigned int)time(NULL));
	do
	{ 
		scanf("%d", &a);
		switch (a)
		{
		case 1:
			game();
			break;
		case 2 :
			break;
		default :
			printf("选择错误\n");
			break;
	 }
	} while (a != 2);
	system("pause");
return 0;
}