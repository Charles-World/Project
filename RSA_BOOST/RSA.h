#pragma once
#include <string>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/random.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <iostream>

namespace bm = boost::multiprecision;

//本项目需要链接boost_1_58_0库

//RSA加密过程:
//随机选择两个不相等的质数p和q，实际应用中，这两个质数越大，就越难破解
//计算p和q的乘积n，n = pq
//计算n的欧拉函数φ(n)
//随机选择一个整数e，条件是1< e < φ(n)，且e与φ(n) 互质
//计算e对于φ(n)的模反元素d，使得de≡1 mod φ(n)
//产生公钥(e,n)，私钥(d,n)

//加密时使用(e,n)，方法是：如果要加密m，那么加密结果a=m ^ e % n
//解密时使用(d,n)，方法是：如果要解密a，那么解密结果m=a ^ d % n

//说一下，为什么两个质数越大，就越难破解，因为两个质数相乘之后，得到的数
//只能分解为那两个质数，也就是说因式分解只有一个结果，在大数的情况下要从结果中分解出
//两个数是非常困难，这里再说一下为什么两质数积，不能像18这个数一样分解为2和9或3和6
//因为2和9中的9还可分解为3*3，然后就变成3*3*2，不就是3*6，所以它可由其它数组成
//而质数是不能进行分解的，所以最终的结果只能分解出两个数
//所以当两质数很大时，分解的难度就增加了很多，使得φ(n)就很难计算，进而就不能确定e的范围
//其次因为欧拉函数的关系，如果两数都是质数，所得φ(n)就会很大，所以e的范围就会增大
struct Key
{
	bm::int1024_t pkey; //就是欧拉函数中的n，用来取模
	bm::int1024_t ekey; //就是公匙
	bm::int1024_t dkey; //就是私匙
};

class RSA
{
public:
	void printInfo(std::vector<bm::int1024_t>& ecrept_str); //用来打印加密后的字符串

	RSA(); //用来生成公匙、私匙和n

	Key getKey()  //获取公钥、私钥和n
	{
		return _key;
	}
	//给文件加密的函数
	void ecrept(const char* plain_file_in, const char* ecrept_file_out,
		bm::int1024_t ekey, bm::int1024_t pkey);
	//给文件解密的函数
	void decrept(const char* ecrept_file_in, const char* ecrept_file_out,
		bm::int1024_t dkey, bm::int1024_t pkey);
	//给字符串加密的函数
	std::vector<bm::int1024_t> ecrept(std::string& str_in, bm::int1024_t ekey, bm::int1024_t pkey);
	//给字符串解密的函数
	std::string decrept(std::vector<bm::int1024_t>& ecrept_str, bm::int1024_t dkey, bm::int1024_t pkey);

private:
	//加密算法和解密算法实现，加密c ^ ekey % pkey，解密m ^ dkey % pkey
	bm::int1024_t ecrept(bm::int1024_t msg, bm::int1024_t key, bm::int1024_t pkey);
	bm::int1024_t produce_prime();        //产生素数的函数
	bool is_Bigprime(bm::int1024_t prime);//判断是否为素数的函数
	void produce_keys();                  //用来生成公钥、私钥和n的函数
	bm::int1024_t produce_pkey(bm::int1024_t prime1, bm::int1024_t prime2);//生成n的函数
	bm::int1024_t produce_orla(bm::int1024_t prime1, bm::int1024_t prime2);//生成orla值的函数
	bm::int1024_t produce_ekey(bm::int1024_t orla);                        //生成公钥的函数
	bm::int1024_t produce_gcd(bm::int1024_t ekey, bm::int1024_t orla);     //计算最大公约数的函数
	bm::int1024_t exgcd(bm::int1024_t ekey, bm::int1024_t orla, bm::int1024_t& x, bm::int1024_t& y);//计算模反元素的函数

	bm::int1024_t produce_dkey(bm::int1024_t ekey, bm::int1024_t orla);    //生成私钥的函数

	Key _key; //公钥、私钥和n
};