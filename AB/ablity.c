
#include "info.h"

enum
{
	ADD = 1,
	DEL,
	MODIFY,
	FIND,
	PRINT,
	CLEAR,
	NAMERANK,
	EXIT,
};

int main()
{
	int choice = 0;

	init();
	do
	{
		choice = menu();
		switch (choice)
		{
		case ADD:
		{
			add_user();
			break;
		}
		case DEL:
		{
			del_user();
			break;
		}
		case MODIFY:
		{
			modify_user();
			break;
		}
		case FIND:
		{
			find_user();
			break;
		}
		case PRINT:
		{
			print_user();
			break;
		}
		case CLEAR:
		{
			clear_user();
			break;
		}
		case NAMERANK:
		{
			namerank_user();
			break;
		}
		case EXIT:
		{
			printf("Good baby!\n");
			break;
		}
		default:
		{
			break;
		}
		}
	} while (choice != EXIT);

	system("pause");
	return 0;
}
