#include "RSA.h"
#include <time.h>
#include <math.h>
#include <fstream>

RSA::RSA()//�������������������Կ��˽Կ��n��
{
	produce_keys();//����ִ����һ�����ĺ�������produce_keys()����װ��ԭ����
				   //RSA������ͨ��RSA()�������������Կ�׵�
}

//�������ļ������ݼ��ܵĺ���
void RSA::ecrept(const char* plain_file_in, const char* ecrept_file_out,
	bm::int1024_t ekey, bm::int1024_t pkey)
{
	std::ifstream fin(plain_file_in, std::ofstream::binary);//��Ҫ���ܵ��ļ�
	std::ofstream fout(ecrept_file_out, std::ofstream::binary); //����һ�����ܹ�����ļ�
	if (!fin.is_open())
	{
		std::cout << "open file failed!" << std::endl;
		return;
	}

	const int NUM = 256;    //һ������ȡ256�ַ�
	char buffer[NUM];       //��Ŷ�����ַ�
	bm::int1024_t buffer_out[NUM]; //��ż��ܹ�����ַ�
	int curNum;             //������ȡ���ļ�ʱ�������˶����ַ�
	while (!fin.eof())
	{
		fin.read(buffer, NUM);//��ȡ�ļ�����
		curNum = fin.gcount();//��ȡ��ȡ������

		for (int i = 0; i < curNum; ++i)
		{
			//��Ϊ���������������ܵģ�����Ҫ���ַ�ת��ASCII�룬ת��bm::int1024_t��ԭ��
			//��Ϊ���ܺͽ����㷨��д��һ��ģ�����Ҫת��ͬ���͵ģ���������Ӱ����
			buffer_out[i] = ecrept((bm::int1024_t)buffer[i], ekey, pkey);
		}
		//��Ϊ��д�ļ�ʱ��Ҫ��char*���ͣ�������Ҫǿ��ת����������Ӱ��д��Ľ��
		fout.write((char*)buffer_out, curNum * sizeof(bm::int1024_t));
	}

	fin.close();
	fout.close();
}
//���������ļ��ĺ���
void RSA::decrept(const char* ecrept_file_in, const char* plain_file_out,
	bm::int1024_t dkey, bm::int1024_t pkey)
{
	std::ifstream fin(ecrept_file_in, std::ofstream::binary); //���ܺ���ļ�
	std::ofstream fout(plain_file_out, std::ofstream::binary); //���ɽ��ܹ�����ļ�
	if (!fin.is_open())
	{
		std::cout << "open file failed!" << std::endl;
		return;
	}

	const int NUM = 256;       //һ�ζ�256����Ԫ
	bm::int1024_t buffer[NUM]; //�洢���������
	char buffer_out[NUM];      //�洢���ܹ��������
	int curNum;                //��ȡ����������
	while (!fin.eof())
	{
		fin.read((char*)buffer, NUM * sizeof(bm::int1024_t));
		//��Ϊ�ڼ��ܺ�ÿ���ַ�ռһ��bm::int1024_t�Ŀռ䣬����Ҫ��ԭ���������ַ�
		curNum = fin.gcount() / sizeof(bm::int1024_t);
		for (int i = 0; i < curNum; ++i)
		{
			buffer_out[i] = (char)ecrept(buffer[i], dkey, pkey);
		}
		fout.write(buffer_out, curNum);
	}

	fin.close();
	fout.close();
}
//�����ַ������ú���
std::vector<bm::int1024_t> RSA::ecrept(std::string& str_in, bm::int1024_t ekey, bm::int1024_t pkey)
{
	std::vector<bm::int1024_t> vecout;//ÿ����Ԫ�洢һ��bm::int1024_t���͵�����
	for (const auto& e : str_in)
	{
		vecout.push_back(ecrept((bm::int1024_t)e, ekey, pkey));//ÿ����һ���ַ������ͽ���һ��β��
	}
	return vecout;
}
//�����ַ������ú���
std::string RSA::decrept(std::vector<bm::int1024_t>& ecrept_str, bm::int1024_t dkey, bm::int1024_t pkey)
{
	std::string strout;
	for (const auto& e : ecrept_str)
	{
		strout.push_back((char)ecrept(e, dkey, pkey)); //ÿ����һ���ַ���β��һ��
	}
	return strout;
}
//��ӡ���ܹ����ַ����ĺ���
void RSA::printInfo(std::vector<bm::int1024_t>& ecrept_str)
{
	for (const auto& e : ecrept_str)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;
}

//�������ܻ���ܵ��㷨
bm::int1024_t RSA::ecrept(bm::int1024_t msg, bm::int1024_t ekey, bm::int1024_t pkey)
{
	//�ڼ���ʱ��Ҫ����ģ�����㣬��Ϊ��a ^ b % c���ǻ��ڴ���������
	//���ֱ�ӱ������㣬��ôЧ�ʾ�̫���ˣ����Ի�������㷨�����Ż�
	//(a +/- b) % c = ((a % c +/- b % c)) % c
	//(a * b) % c = ((a % c) * (b % c)) % c
	//a^b % c = (a * a * a����a) % c
	//= ((a%c)*(a%c)*(a%c)*����*(a%c)) % c
	//= ((a % c) ^ b) % c
	//Ȼ��ʼ֤��
	//�����κ�һ��������ģ�����㣺a^b % c
	//����bչ�����Ķ�������ʽ
	//b = b0 * 2 ^ 0 + b1 * 2 ^ 1 + ... + bn * 2 ^ n 
	//���� a ^ b = a ^ (b0 * 2 ^ 0 + b1 * 2 ^ 1 + ... + bn * 2 ^ n )
	//           = a ^ (b0 * 2 ^ 0) * a ^ (b1 * 2 ^ 1) * ... * a ^ (bn * 2 ^ n)
	//��Ϊ���ǵ�b0��bn��Ӧ����b�� ������ϵ����Ϊ0����1�����biΪ0����ôa��ָ��Ϊ0
	//��ô��һ���ֵΪ1����Ϊ���������˵���ʽ�����Խ������ı䣬���Կ��԰���ЩΪbiΪ0���������
	//���� a ^ b = a ^ (bi * 2 ^ i) * ... * a ^ (bn * 2 ^ n), bi != 0
	//           = a ^ (2 ^ i) * ... * a ^ (2 ^ n)
	//���� a ^ b % c = (a ^ (2 ^ i % c) * ... * a ^ (2 ^ n % c)) % c
	//������ֱ���A���棬�ͱ���� a ^ b % c = (Ai * .. * An) % c
	//����A0 = a ^ (2 ^ 0) % c��A1 = a ^ (2 ^ 1) % c
	//    A(n - 1) = a ^ (2(n - 1)) % c��An = a ^ ( 2 ^ n) % c
	//���Կ��� An = (a ^ (2(n - 1))) ^ 2 % c
	//         An = (a ^ (2(n - 1)) * a ^ (2(n - 1))) % c
	//��Ϊ(a * b) % c = ((a % c) * (a % c)) % c
	//���� An = (a ^ (2(n - 1)) % c * a ^ (2(n - 1)) % c) % c
	//���� An = (A(n - 1) * A(n - 1)) % c
	//Ҳ����˵ÿһ���ֵ������������ǰ��ֵ�����㣬�㷨������ǰһ��ֵ��ƽ��ȡ��
	//����Ҫע����ǣ�������Ĭ��ÿһ���ֵ����Ϊ1ʱ���㷨��Ҳ���ǽ�b������չ����ÿһλ����Ϊ0)
	//����һ������¿϶������Ϊ0��λ���������������м���ʱ�����Ƕ����õ�һ����Ϊ0��λ����������ÿһ���ֵ
	//Ȼ���м�ֵΪ0��λҲ��ǰһ��ֵ��ƽ����Ȼ��ȡ�����ַ������㣬��Ȼ��һ���ֵ��������������ֵ�Ǵ��
	//������һ����Ϊ0��λ����������һ���ֵ�����㣬���Բ�Ϊ0��λ��ֵ����ȷ��
	//��Ϊ0��λ�����Ǹ�������Ҫ�����۳ˣ���Ϊ1���κ����������Ǹ���

	bm::int1024_t msg_out = 1; //Ҫ�������յļ��ܺ�����ݵı�������Ϊ1��ԭ���ǵ�һλ��Ϊ0��λ
							   //���ǿ���ֱ�Ӽ������������Ҫ��ǰһ�ֻҪ��֤���������ֵ���������
	bm::int1024_t a = msg;     //��Ҫ���ܵ�����
	bm::int1024_t c = pkey;    //nֵ
	bm::int1024_t b = ekey;    //��Կ
	while (b)                  //ֻҪ����һλ��Ϊ0���ʹ���û�м������
	{
		if (b & 1)             //ֻ�۳˲�Ϊ0��λ
		{
			msg_out = (msg_out * a) % c;  //����ķ�����ǰһ��ֵ��ƽ����Ȼ��ȡ��
		}
		a = (a * a) % c;       //ǰһ���ֵ
		b >>= 1;               //�۳�һ�Σ�����һλ
	}
	return msg_out;
}

bm::int1024_t RSA::produce_prime()
{
	boost::random::mt19937 gen(time(nullptr));  //���������������͵���������Ƚ��г�ʼ��
												//���ڿ���������ķ�ΧΪ2 ~ 1<<111
	boost::random::uniform_int_distribution<bm::int1024_t> dist(2, bm::int1024_t(1) << 111);
	bm::int1024_t prime = 0;    //�����������������

	while (1)
	{
		prime = dist(gen);
		if (is_Bigprime(prime)) //�ж��Ƿ�Ϊ����
		{
			break;
		}
	}
	return prime;
}
//�����ж��Ƿ�Ϊ�����ĺ���
bool RSA::is_Bigprime(bm::int1024_t digit)
{
	boost::random::mt11213b gen(time(nullptr)); //һ�������������������
	if (miller_rabin_test(digit, 25, gen))      //boost�����ڲ����Ƿ�Ϊ�����ĺ���
	{                                           //��һ����������Ҫ�����Ե�����
		if (miller_rabin_test((digit - 1) / 2, 25, gen));
		{                                       //�ڶ��������ǲ��Դ�������Ϊ�����Ĳ��Ժ���ͨ��������ͬ
			return true;                        //�����ȡ��ͨ���Է�����Ч��̫�ͣ�����������һЩ�������㷨�����в���
		}                                       //miller_rabin����㷨�ó��Ľ�������Ϊfalse����ô˵����������Ǻ���
	}                                           //������Ϊtrue����ô˵���������������������Ϊ�˽��ͳ���Ŀ�����
	return false;                               //�������25�Σ��õ���ȷ�Ľ��
}                                               //������������һ����������������ɣ���������ǿ�ѡ��
												//����������ʹ���Դ����������������ش�һ�����������������Ա�֤����������жϽ������ȷ��
												//����������ж���ɺ󣬵ó��Ľ������Ϊ����������Ҫ�ٲ������������1��2���Ƿ�Ϊ����
												//�����ʱ����������жϽ����Ϊtrue����ô˵����������ǰ�ȫ��
												//�������ɹ�Կ��˽Կ��n
void RSA::produce_keys()
{
	bm::int1024_t prime1 = produce_prime();
	bm::int1024_t prime2 = produce_prime(); //��������������������n
	while (prime1 == prime2)                //������������������������ͬ��
	{
		prime2 = produce_prime();
	}
	_key.pkey = produce_pkey(prime1, prime2);//����n
	bm::int1024_t orla = produce_orla(prime1, prime2); //�õ�ŷ������ֵ
	_key.ekey = produce_ekey(orla);          //������Կ
	_key.dkey = produce_dkey(_key.ekey, orla);//����˽Կ
}

bm::int1024_t RSA::produce_pkey(bm::int1024_t prime1, bm::int1024_t prime2)
{
	std::cout << "��Կ:" << prime1 * prime2 << std::endl; //n = prime1 * prime2
	return prime1 * prime2;
}
bm::int1024_t RSA::produce_orla(bm::int1024_t prime1, bm::int1024_t prime2)
{
	//ŷ��������С��x������������x���ʵ�������Ŀ
	//��Ϊŷ�������ǻ��Ժ��������Ԧ�(mn) = ��(m) * ��(n)
	//���m�����������(m) = m - 1
	//���Ԧ�(mn) = (m - 1) * (n - 1)
	//����orla = (prime1 - 1) * (prime2 - 1)
	std::cout << "orla:" << (prime1 - 1) * (prime2 - 1) << std::endl;
	return (prime1 - 1) * (prime2 - 1);
}
//���ɹ�Կ�ĺ���
bm::int1024_t RSA::produce_ekey(bm::int1024_t orla)
{
	// ��Կe��������1< e < ��(n)����e���(n) ���ʡ�
	bm::int1024_t ekey;   //������Ź�Կ
	srand(time(nullptr)); //�������������Կ����������ȫ���Ƚ��г�ʼ��
	while (1)
	{
		ekey = rand() % orla; //����һ�����ڵ���0��С��orlaֵ�Ĺ�Կ
		if (ekey > 1 && produce_gcd(ekey, orla) == 1) //Ҫ��Կ�������1������orlaֵ����
		{
			break;
		}
	}
	std::cout << "����Կ:" << ekey << std::endl;
	return ekey;
}
//�����ж������Ƿ��ʣ�Ҳ�����ԼΪ1
bm::int1024_t RSA::produce_gcd(bm::int1024_t ekey, bm::int1024_t orla)
{
	//ʹ��շת��������������Լ��
	//շת��������㷨�ǣ���a��bȡģ��Ȼ���������c��Ȼ����b��cȡģ�����d��������b�ܵ�a��λ����
	//c�ܵ�b��λ���ϣ�d�����������b���λ����0ʱ����ʾa�������Լ��
	bm::int1024_t residual;
	while (residual = ekey % orla)
	{
		ekey = orla;
		orla = residual;
	}
	return orla;
}

//��ȡģ��Ԫ�أ�Ҳ����˽Կ
bm::int1024_t RSA::exgcd(bm::int1024_t ekey, bm::int1024_t orla, bm::int1024_t& x, bm::int1024_t& y)
{
	//��Ϊģ��Ԫ��dҪ��(��Կe)��ed��1 mod ��(n)��������������ñ��������ķ�������ôЧ�ʾͻ��ܶ�
	//����������չŷ����¹�ʽ�Ϳ��Ը����׵ý����Ч�ʸ���
	//��չŷ����¹�ʽ��˵��������շת���������Լ��(gcd)��ͬʱ�����������һ��ax + by = gcd�Ľ�
	//�������Ǽ��蹫Կe��a��n��b��ed��1 mod ��(n)����д�ɣ�ed - 1 = k��(n)������ed - k��(n) = 1
	//������Ϊk��δ֪�����������ǲ����Ǻܹ���k��ֵ������Ҳ����д��ed + k��(n) = 1
	//����d���൱����չŷ����¹�ʽ�е�x��k���൱��y����Ϊe�ͦ�(n)����֪�ģ����Կ������d��k
	//ax + by = gcd�������ʽ���У���b����0ʱ��ax = gcd����ʱ���Ƿ���a��ֵ��a������������Լ
	//��Ϊ�������Լ��ʱ���õ�շת�����,����b��ֵ����ÿ�μ�������н��иı�,ֱ�����b��ֵ��ʽ�����0���ű�ʾa�����Լ��
	//շת��������㷨�ǣ���a��bȡģ��Ȼ���������c��Ȼ����b��cȡģ�����d��������b�ܵ�a��λ����
	//c�ܵ�b��λ���ϣ�d�����������b���λ����0ʱ����ʾa�������Լ��)
	//�ʴ�ʱx = 1(ed + k��(n) = 1)��Ϊy����������ֵ�����ԣ��򵥵�һ��⼴Ϊ(1, 0)
	//���������һ��õ��ģ�������������Ҫ�Ľ⣬����Ҫ�������a��b��ֵ������Ľ�
	//��Ϊ����շת��������ݹ��ʱ���ڻ�ȡ���ս��֮ǰ���Ѿ����������״̬��b �� a%b���ı�λ�ú�b����a��a%bȡ�����b
	//��Ϊ����֪������״̬һ�������һ���(1,0)�������������(x1, y1)����ô��������һ�㣬��֪������ôһ���
	//����bx1 + (a % b)y1 = 1�������ǿ��Եõ����ʽ�ӵ�
	//������ a % b = a - [a / b] * b�� ����[a / b]��ʾ����ȡ���� ����������ʽ
	//bx1 + (a - [a / b] * b)y1 = 1
	//bx1 + ay1 - [a / b] * by1 = 1
	//ay1 + b(x1 - [a / b]y1) = 1
	//���Կ��Եõ�x = y1��y = x1 - [a / b]y1
	//������(x1, y1)���룬�͵õ�����һ���x��y�Ľ�
	//ͬ��������Լ����õ�����һ��Ľ⣬�������ܵõ���ʼ״̬�е�(x, y)��
	//ed + k��(n) = 1������d����x��k����y(yûʲô��)���Ϳ��Եõ�e��ģ��Ԫ��d

	if (orla == 0) //������b����orla����0ʱ��˵��������״̬��
	{
		x = 1;    //ʼ��
		y = 0;
		return ekey;
	}

	bm::int1024_t ret = exgcd(orla, ekey % orla, x, y); //�ȵõ�ʼ�⣬���ܸ���ʼ�������ݣ��õ����ս�

	bm::int1024_t x1 = x, y1 = y;   //�õ�ʼ��󣬸��ݹ�ʽ���ϵ���
	x = y1;
	y = x1 - (ekey / orla) * y1;
	return ret;
}
//��������˽Կ�ĺ���
bm::int1024_t RSA::produce_dkey(bm::int1024_t ekey, bm::int1024_t orla)
{
	bm::int1024_t x, y;
	exgcd(ekey, orla, x, y); //���ص�xֵ����˽Կ��ֻ��������Ϊ����������Ҫʹ��һ���㷨
							 //�����Ǹ���ʱ��ת��Ϊ��ֵ�������������������ô�Ͳ����
	std::cout << "����Կ:" << (x % orla + orla) % orla << std::endl;
	return (x % orla + orla) % orla;
}