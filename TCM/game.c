#include "game.h"

int main()
{
	int input = 0;
	srand((unsigned int)time(NULL));
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
			printf("�˳���Ϸ��\n");
			break;
		default:
			printf("�����������������롣\n");
			break;
		}
	} while (input != 2); //ֻҪ������2���ͼ�����
	system("pause");
	return 0;
}
