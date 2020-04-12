#include "info.h"

typedef struct userinfo  //�ȶ���һ��ͨѶ¼����Ҫ�������Ϣ�Ľṹ��
{
	char name[SIZE];
	char telphone[SIZE];
	char age[SIZE];
	char sex[SIZE];
	char addr[SIZE];
}userinfo;

typedef struct usercount   //����һ�����ڱ����û������ͼ�¼�����˶����û��Ľṹ�岢�Ҷ���һ��
{                          //������չ��̬����ı���
	userinfo *usernum;
	int size;
	int capacity;
}usercount;
usercount user_count;

void init()          //��ʼ������
{
	int i = 0;
	user_count.size = 0;
	user_count.capacity = 10;
	user_count.usernum = (userinfo*)malloc(sizeof(userinfo) * user_count.capacity);  //������̬���飬Ϊ���Ժ����ӱ�����û�����
																					 /*for (i = 0; i < user_count.capacity; i++)
																					 {
																					 memset(&user_count.usernum[i], 0x0, sizeof(userinfo));
																					 }*/
	memset(user_count.usernum, 0x0, sizeof(user_count.usernum));  //��ʼ������Ϊ0��Ҳ����������ķ���
	loading_user();     //������ϵ�ˣ����ϴε�д����û�����
}

int menu()        //��ӡһ���˵�
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
		while ((ch = getchar()) != '\n' && (ch != EOF))   //�޸�һ����ѭ��bug
		{
			;
		}
		if (choice > 0 && choice <= 8)
		{
			return choice;
		}
		else
		{
			printf("����������������룡\n");
		}
	}
}

void save_user()    //����������û����ļ��У��Է����������������
{
	int i = 0;
	FILE *sa = fopen("./long.txt", "w");    //�򿪵�ǰλ�õ��ļ���Ҳ����ʹ�þ���·��������
											//����д��Ϊд�ķ�ʽ�򿪣������a�ķ�ʽ�򿪣��� ��
	if (sa == NULL)                         //һ��bug��������ÿ���һ���û��������֮ǰ�ֱ���
	{                                       //һ��Ҳ����˵���ظ�����һ��֮ǰ��
		printf("����ʧ��!\n");
	}

	for (i = 0; i < user_count.size; i++)
	{
		fwrite(&user_count.usernum[i], sizeof(userinfo), 1, sa);
	}

	fclose(sa);                              //�����ͷţ���Ȼ������ļ���й©
	printf("����ɹ���\n");
}

void loading_user()                        //������ϵ��
{
	userinfo tmp = { 0 };
	FILE *load = fopen("./long.txt", "r");

	printf("��ʼ���أ�\n");
	if (load == NULL)
	{
		printf("����ʧ�ܣ�\n");
	}

	while (fread(&tmp, sizeof(userinfo), 1, load))    //��Ϊ����������������ʽ����ģ����Զ���ʱ��Ҳ��������
	{
		expand_user();
		user_count.usernum[user_count.size] = tmp;
		++user_count.size;
	}

	fclose(load);
	printf("���سɹ���������%d��\n", user_count.size);
}

void expand_user()          //��дһ����reallocһ�����ܵĺ���
{
	if (user_count.size >= user_count.capacity)
	{
		userinfo *tmp;
		int i = 0;

		user_count.capacity += 10;          //ÿ����չ10����ϵ��
		tmp = (userinfo*)malloc(sizeof(userinfo) * user_count.capacity);
		for (i = 0; i < user_count.size; i++)
		{
			tmp[i] = user_count.usernum[i];
		}

		free(user_count.usernum);
		user_count.usernum = tmp;
		//realloc(user_count.usernum, sizeof(userinfo) * user_count.capacity);  //��realloc��ʽ��չ��ϵ��
	}
}


void add_user()          //�����ϵ��
{
	expand_user();
	printf("�����һ�����û���\n");
	printf("������Ҫ����˵�����:\n");
	scanf("%s", user_count.usernum[user_count.size].name);
	printf("������Ҫ����˵ĵ绰����:\n");
	scanf("%s", user_count.usernum[user_count.size].telphone);
	printf("����������˵����䣺\n");
	scanf("%s", user_count.usernum[user_count.size].age);
	printf("����������˵��Ա�\n");
	scanf("%s", user_count.usernum[user_count.size].sex);
	printf("������Ҫ����˵ĵ�ַ��\n");
	scanf("%s", user_count.usernum[user_count.size].addr);
	printf("��ӳɹ���\n");
	++user_count.size;
	save_user();
}

void print_user()   //��ӡ��ϵ��
{
	int i = 0;

	printf("��ӡ�����û���Ϣ��\n");
	for (i = 0; i < user_count.size; i++)
	{
		printf("[%d] %s %s %s %s %s\n", i + 1,
			user_count.usernum[i].name,
			user_count.usernum[i].telphone,
			user_count.usernum[i].age,
			user_count.usernum[i].sex,
			user_count.usernum[i].addr);
	}
	printf("����ӡ��%d���û���Ϣ\n", user_count.size);
}

void del_user()             //ɾ����ϵ��               
{
	char user[SIZE] = { 0 };
	int i = 0;

	printf("ɾ���û���\n");
	printf("������Ҫɾ�����û�������\n");
	scanf("%s", user);
	for (i = 0; i < user_count.size; i++)
	{
		if (strcmp(user, user_count.usernum[i].name) == 0)
		{
			printf("%s", user_count.usernum[i].name);
			user_count.usernum[i] = user_count.usernum[user_count.size];
		}
	}
	printf("ɾ���ɹ���\n");
	user_count.size--;
	save_user();
}

void modify_user()                 //�޸���ϵ��
{
	char user[SIZE] = { 0 };
	int i = 0;

	printf("�޸�һ���û�\n");
	printf("������Ҫ�޸��û���������\n");
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
			printf("�������޸Ĺ������֣�\n");
			scanf("%s", user_count.usernum[i].name);
			printf("�������޸Ĺ����ֻ��ţ�\n");
			scanf("%s", user_count.usernum[i].telphone);
			printf("�������޸Ĺ������䣺\n");
			scanf("%s", user_count.usernum[user_count.size].age);
			printf("�������޸Ĺ����Ա�\n");
			scanf("%s", user_count.usernum[user_count.size].sex);
			printf("�������޸Ĺ��ĵ�ַ��\n");
			scanf("%s", user_count.usernum[user_count.size].addr);
			printf("[%d] %s %s %s %s %s\n", i,
				user_count.usernum[i].name,
				user_count.usernum[i].telphone,
				user_count.usernum[i].age,
				user_count.usernum[i].sex,
				user_count.usernum[i].addr);
			printf("�޸ĳɹ���\n");
		}
	}
	save_user();
}

void find_user()                    //������ϵ��
{
	char user[SIZE] = { 0 };
	int i = 0;
	int count = 0;

	printf("����һ���û���\n");
	printf("������Ҫ���ҵ��û���������\n");
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
	printf("���ҳɹ�,���ҵ���%d�������\n", count);
}

void namerank_user()               //ð������
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
	printf("����ɹ���\n");
	save_user();
}


void clear_user()           //���������ϵ��
{
	int i = 0;
	char a[SIZE] = { 0 };

	printf("��ȷ��Ҫȫ�������(y/n)\n");
	scanf("%s", a);
	if (strcmp("y", a) == 0)
	{
		/*for (i = 0; i < user_count.capacity; i++)
		{
		memset(&user_count.usernum[i], NULL, sizeof(userinfo));
		}*/
		memset(user_count.usernum, NULL, sizeof(user_count.usernum));  //�м����ųɿյĲŻ��������
		user_count.size = 0;
		user_count.capacity = 10;
	}
	printf("��ճɹ���\n");
	save_user();                                      //���ļ����
}
