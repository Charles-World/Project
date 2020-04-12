#include "info.h"

typedef struct userinfo  //先定义一个通讯录中需要保存的信息的结构体
{
	char name[SIZE];
	char telphone[SIZE];
	char age[SIZE];
	char sex[SIZE];
	char addr[SIZE];
}userinfo;

typedef struct usercount   //创建一个用于保存用户个数和记录保存了多少用户的结构体并且定义一个
{                          //用于扩展动态数组的变量
	userinfo *usernum;
	int size;
	int capacity;
}usercount;
usercount user_count;

void init()          //初始化数组
{
	int i = 0;
	user_count.size = 0;
	user_count.capacity = 10;
	user_count.usernum = (userinfo*)malloc(sizeof(userinfo) * user_count.capacity);  //创建动态数组，为了以后增加保存的用户数量
																					 /*for (i = 0; i < user_count.capacity; i++)
																					 {
																					 memset(&user_count.usernum[i], 0x0, sizeof(userinfo));
																					 }*/
	memset(user_count.usernum, 0x0, sizeof(user_count.usernum));  //初始化数组为0，也可以用上面的方法
	loading_user();     //加载联系人，把上次的写入的用户读入
}

int menu()        //打印一个菜单
{
	int choice = 0;

	printf("***************************************\n");
	printf("*************   *1.ADD*   *************\n");
	printf("*************   *2.DEF*   *************\n");
	printf("*************   *3.MODIFY*   **********\n");
	printf("*************   *4.FIND*   ************\n");
	printf("*************   *5.PRINT*   ***********\n");
	printf("*************   *6.CLEAR*   ***********\n");
	printf("*************   *7.NAMERANK*   ********\n");
	printf("*************   *8.EXIT*   ************\n");
	printf("***************************************\n");
	while (1)
	{
		char ch;

		scanf("%d", &choice);
		while ((ch = getchar()) != '\n' && (ch != EOF))   //修复一个死循环bug
		{
			;
		}
		if (choice > 0 && choice <= 8)
		{
			return choice;
		}
		else
		{
			printf("输入错误，请重新输入！\n");
		}
	}
}

void save_user()    //保存输入的用户到文件中，以防程序结束用真接清空
{
	int i = 0;
	FILE *sa = fopen("./long.txt", "w");    //打开当前位置的文件，也可以使用绝对路径，这里
											//尽量写成为写的方式打开，如果以a的方式打开，会 有
	if (sa == NULL)                         //一个bug，就是你每添加一个用户它都会把之前又保存
	{                                       //一遍也就是说会重复保存一遍之前的
		printf("保存失败!\n");
	}

	for (i = 0; i < user_count.size; i++)
	{
		fwrite(&user_count.usernum[i], sizeof(userinfo), 1, sa);
	}

	fclose(sa);                              //必须释放，不然会出现文件的泄漏
	printf("保存成功！\n");
}

void loading_user()                        //加载联系人
{
	userinfo tmp = { 0 };
	FILE *load = fopen("./long.txt", "r");

	printf("开始加载：\n");
	if (load == NULL)
	{
		printf("加载失败！\n");
	}

	while (fread(&tmp, sizeof(userinfo), 1, load))    //因为上面是以这样的形式保存的，所以读的时候也得这样读
	{
		expand_user();
		user_count.usernum[user_count.size] = tmp;
		++user_count.size;
	}

	fclose(load);
	printf("加载成功共加载了%d条\n", user_count.size);
}

void expand_user()          //编写一个和realloc一样功能的函数
{
	if (user_count.size >= user_count.capacity)
	{
		userinfo *tmp;
		int i = 0;

		user_count.capacity += 10;          //每次扩展10个联系人
		tmp = (userinfo*)malloc(sizeof(userinfo) * user_count.capacity);
		for (i = 0; i < user_count.size; i++)
		{
			tmp[i] = user_count.usernum[i];
		}

		free(user_count.usernum);
		user_count.usernum = tmp;
		//realloc(user_count.usernum, sizeof(userinfo) * user_count.capacity);  //以realloc形式扩展联系人
	}
}


void add_user()          //添加联系人
{
	expand_user();
	printf("请添加一个新用户！\n");
	printf("请输入要添加人的姓名:\n");
	scanf("%s", user_count.usernum[user_count.size].name);
	printf("请输入要添加人的电话号码:\n");
	scanf("%s", user_count.usernum[user_count.size].telphone);
	printf("请输入添加人的年龄：\n");
	scanf("%s", user_count.usernum[user_count.size].age);
	printf("请输入添加人的性别：\n");
	scanf("%s", user_count.usernum[user_count.size].sex);
	printf("请输入要添加人的地址：\n");
	scanf("%s", user_count.usernum[user_count.size].addr);
	printf("添加成功！\n");
	++user_count.size;
	save_user();
}

void print_user()   //打印联系人
{
	int i = 0;

	printf("打印所有用户信息！\n");
	for (i = 0; i < user_count.size; i++)
	{
		printf("[%d] %s %s %s %s %s\n", i + 1,
			user_count.usernum[i].name,
			user_count.usernum[i].telphone,
			user_count.usernum[i].age,
			user_count.usernum[i].sex,
			user_count.usernum[i].addr);
	}
	printf("共打印了%d条用户信息\n", user_count.size);
}

void del_user()             //删除联系人               
{
	char user[SIZE] = { 0 };
	int i = 0;

	printf("删除用户！\n");
	printf("请输入要删除的用户姓名：\n");
	scanf("%s", user);
	for (i = 0; i < user_count.size; i++)
	{
		if (strcmp(user, user_count.usernum[i].name) == 0)
		{
			printf("%s", user_count.usernum[i].name);
			user_count.usernum[i] = user_count.usernum[user_count.size];
		}
	}
	printf("删除成功！\n");
	user_count.size--;
	save_user();
}

void modify_user()                 //修改联系人
{
	char user[SIZE] = { 0 };
	int i = 0;

	printf("修改一个用户\n");
	printf("请输入要修改用户的姓名：\n");
	scanf("%s", user);
	for (i = 0; i < user_count.size; i++)
	{
		if (strcmp(user, user_count.usernum[i].name) == 0)
		{
			printf("[%d] %s %s %s %s %s\n", i,
				user_count.usernum[i].name,
				user_count.usernum[i].telphone,
				user_count.usernum[i].age,
				user_count.usernum[i].sex,
				user_count.usernum[i].addr);
			printf("请输入修改过的名字：\n");
			scanf("%s", user_count.usernum[i].name);
			printf("请输入修改过的手机号：\n");
			scanf("%s", user_count.usernum[i].telphone);
			printf("请输入修改过的年龄：\n");
			scanf("%s", user_count.usernum[user_count.size].age);
			printf("请输入修改过的性别：\n");
			scanf("%s", user_count.usernum[user_count.size].sex);
			printf("请输入修改过的地址：\n");
			scanf("%s", user_count.usernum[user_count.size].addr);
			printf("[%d] %s %s %s %s %s\n", i,
				user_count.usernum[i].name,
				user_count.usernum[i].telphone,
				user_count.usernum[i].age,
				user_count.usernum[i].sex,
				user_count.usernum[i].addr);
			printf("修改成功！\n");
		}
	}
	save_user();
}

void find_user()                    //查找联系人
{
	char user[SIZE] = { 0 };
	int i = 0;
	int count = 0;

	printf("查找一个用户！\n");
	printf("请输入要查找的用户的姓名：\n");
	scanf("%s", user);
	for (i = 0; i < user_count.size; i++)
	{
		if (strcmp(user, user_count.usernum[i].name) == 0)
		{
			printf("[%d] %s %s %s %s %s\n", i,
				user_count.usernum[i].name,
				user_count.usernum[i].telphone,
				user_count.usernum[i].age,
				user_count.usernum[i].sex,
				user_count.usernum[i].addr);
			count++;
		}
	}
	printf("查找成功,共找到了%d条结果！\n", count);
}

void namerank_user()               //冒泡排序
{
	int i = 0;
	int j = 0;
	char tmp[SIZE];

	for (i = 0; i < user_count.size; i++)
	{
		for (j = 0; j < user_count.size; j++)
		{
			if (strcmp(user_count.usernum[j].name, user_count.usernum[j + 1].name) > 0)
			{
				strcpy(tmp, user_count.usernum[j].name);
				strcpy(user_count.usernum[j].name, user_count.usernum[j + 1].name);
				strcpy(user_count.usernum[j + 1].name, tmp);
			}
		}
	}
	printf("排序成功！\n");
	save_user();
}


void clear_user()           //清除所有联系人
{
	int i = 0;
	char a[SIZE] = { 0 };

	printf("您确认要全部清除吗？(y/n)\n");
	scanf("%s", a);
	if (strcmp("y", a) == 0)
	{
		/*for (i = 0; i < user_count.capacity; i++)
		{
		memset(&user_count.usernum[i], NULL, sizeof(userinfo));
		}*/
		memset(user_count.usernum, NULL, sizeof(user_count.usernum));  //中间必须放成空的才会真正清除
		user_count.size = 0;
		user_count.capacity = 10;
	}
	printf("清空成功！\n");
	save_user();                                      //让文件清空
}
