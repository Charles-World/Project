#pragma once
#include <string>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/random.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <iostream>

namespace bm = boost::multiprecision;

//����Ŀ��Ҫ����boost_1_58_0��

//RSA���ܹ���:
//���ѡ����������ȵ�����p��q��ʵ��Ӧ���У�����������Խ�󣬾�Խ���ƽ�
//����p��q�ĳ˻�n��n = pq
//����n��ŷ��������(n)
//���ѡ��һ������e��������1< e < ��(n)����e���(n) ����
//����e���ڦ�(n)��ģ��Ԫ��d��ʹ��de��1 mod ��(n)
//������Կ(e,n)��˽Կ(d,n)

//����ʱʹ��(e,n)�������ǣ����Ҫ����m����ô���ܽ��a=m ^ e % n
//����ʱʹ��(d,n)�������ǣ����Ҫ����a����ô���ܽ��m=a ^ d % n

//˵һ�£�Ϊʲô��������Խ�󣬾�Խ���ƽ⣬��Ϊ�����������֮�󣬵õ�����
//ֻ�ֽܷ�Ϊ������������Ҳ����˵��ʽ�ֽ�ֻ��һ��������ڴ����������Ҫ�ӽ���зֽ��
//�������Ƿǳ����ѣ�������˵һ��Ϊʲô����������������18�����һ���ֽ�Ϊ2��9��3��6
//��Ϊ2��9�е�9���ɷֽ�Ϊ3*3��Ȼ��ͱ��3*3*2��������3*6���������������������
//�������ǲ��ܽ��зֽ�ģ��������յĽ��ֻ�ֽܷ��������
//���Ե��������ܴ�ʱ���ֽ���ѶȾ������˺ܶ࣬ʹ�æ�(n)�ͺ��Ѽ��㣬�����Ͳ���ȷ��e�ķ�Χ
//�����Ϊŷ�������Ĺ�ϵ����������������������æ�(n)�ͻ�ܴ�����e�ķ�Χ�ͻ�����
struct Key
{
	bm::int1024_t pkey; //����ŷ�������е�n������ȡģ
	bm::int1024_t ekey; //���ǹ���
	bm::int1024_t dkey; //����˽��
};

class RSA
{
public:
	void printInfo(std::vector<bm::int1024_t>& ecrept_str); //������ӡ���ܺ���ַ���

	RSA(); //�������ɹ��ס�˽�׺�n

	Key getKey()  //��ȡ��Կ��˽Կ��n
	{
		return _key;
	}
	//���ļ����ܵĺ���
	void ecrept(const char* plain_file_in, const char* ecrept_file_out,
		bm::int1024_t ekey, bm::int1024_t pkey);
	//���ļ����ܵĺ���
	void decrept(const char* ecrept_file_in, const char* ecrept_file_out,
		bm::int1024_t dkey, bm::int1024_t pkey);
	//���ַ������ܵĺ���
	std::vector<bm::int1024_t> ecrept(std::string& str_in, bm::int1024_t ekey, bm::int1024_t pkey);
	//���ַ������ܵĺ���
	std::string decrept(std::vector<bm::int1024_t>& ecrept_str, bm::int1024_t dkey, bm::int1024_t pkey);

private:
	//�����㷨�ͽ����㷨ʵ�֣�����c ^ ekey % pkey������m ^ dkey % pkey
	bm::int1024_t ecrept(bm::int1024_t msg, bm::int1024_t key, bm::int1024_t pkey);
	bm::int1024_t produce_prime();        //���������ĺ���
	bool is_Bigprime(bm::int1024_t prime);//�ж��Ƿ�Ϊ�����ĺ���
	void produce_keys();                  //�������ɹ�Կ��˽Կ��n�ĺ���
	bm::int1024_t produce_pkey(bm::int1024_t prime1, bm::int1024_t prime2);//����n�ĺ���
	bm::int1024_t produce_orla(bm::int1024_t prime1, bm::int1024_t prime2);//����orlaֵ�ĺ���
	bm::int1024_t produce_ekey(bm::int1024_t orla);                        //���ɹ�Կ�ĺ���
	bm::int1024_t produce_gcd(bm::int1024_t ekey, bm::int1024_t orla);     //�������Լ���ĺ���
	bm::int1024_t exgcd(bm::int1024_t ekey, bm::int1024_t orla, bm::int1024_t& x, bm::int1024_t& y);//����ģ��Ԫ�صĺ���

	bm::int1024_t produce_dkey(bm::int1024_t ekey, bm::int1024_t orla);    //����˽Կ�ĺ���

	Key _key; //��Կ��˽Կ��n
};