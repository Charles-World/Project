#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
void menu()
{
	printf("******************************\n");
	printf("******��ӭ������������Ϸ******\n");
	printf("************1.start***********\n");
	printf("************2.exit ***********\n");
	printf("******************************\n");
}
int game()
{
	int a, b, i;
	printf("������������20��\n");
	printf("��Ϸ��ʼ\n");
	b = rand() % 100 + 1;
	for (i = 1; i < 21; i++)
	{
		printf("������һ������>:");
		scanf("%d", &a);
		if (a < b)
		{
			printf("��С��\n");
		}
		else if (a > b)
		{
			printf("�´���\n");
		}
		else
		{
			printf("��ϲ�㣬�¶���\n");
			printf("�������(����)������Ļ���������>:1(2)\n");
			break;
		}
	}
}
int main()
{

	int a;
	menu();
	printf("������>:");
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
			printf("ѡ�����\n");
			break;
	 }
	} while (a != 2);
	system("pause");
return 0;
}