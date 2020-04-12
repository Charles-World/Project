#include "RSA.h"
#include <time.h>
#include <math.h>
#include <fstream>

RSA::RSA()//这个函数是用来产生公钥、私钥和n的
{
	produce_keys();//用来执行这一操作的函数，用produce_keys()来封装的原因是
				   //RSA类中是通过RSA()这个函数来产生钥匙的
}

//用来给文件中数据加密的函数
void RSA::ecrept(const char* plain_file_in, const char* ecrept_file_out,
	bm::int1024_t ekey, bm::int1024_t pkey)
{
	std::ifstream fin(plain_file_in, std::ofstream::binary);//需要加密的文件
	std::ofstream fout(ecrept_file_out, std::ofstream::binary); //生成一个加密过后的文件
	if (!fin.is_open())
	{
		std::cout << "open file failed!" << std::endl;
		return;
	}

	const int NUM = 256;    //一次最多读取256字符
	char buffer[NUM];       //存放读入的字符
	bm::int1024_t buffer_out[NUM]; //存放加密过后的字符
	int curNum;             //用来获取读文件时，读到了多少字符
	while (!fin.eof())
	{
		fin.read(buffer, NUM);//读取文件数据
		curNum = fin.gcount();//获取读取的数量

		for (int i = 0; i < curNum; ++i)
		{
			//因为加密是用整数加密的，所以要将字符转成ASCII码，转成bm::int1024_t的原因
			//因为加密和解密算法是写在一起的，所以要转成同类型的，不过不会影响结果
			buffer_out[i] = ecrept((bm::int1024_t)buffer[i], ekey, pkey);
		}
		//因为在写文件时，要用char*类型，所以需要强制转换，并不会影响写入的结果
		fout.write((char*)buffer_out, curNum * sizeof(bm::int1024_t));
	}

	fin.close();
	fout.close();
}
//用来解密文件的函数
void RSA::decrept(const char* ecrept_file_in, const char* plain_file_out,
	bm::int1024_t dkey, bm::int1024_t pkey)
{
	std::ifstream fin(ecrept_file_in, std::ofstream::binary); //加密后的文件
	std::ofstream fout(plain_file_out, std::ofstream::binary); //生成解密过后的文件
	if (!fin.is_open())
	{
		std::cout << "open file failed!" << std::endl;
		return;
	}

	const int NUM = 256;       //一次读256个单元
	bm::int1024_t buffer[NUM]; //存储读入的数据
	char buffer_out[NUM];      //存储解密过后的数据
	int curNum;                //获取读到的数量
	while (!fin.eof())
	{
		fin.read((char*)buffer, NUM * sizeof(bm::int1024_t));
		//因为在加密后，每个字符占一个bm::int1024_t的空间，所以要还原读到多少字符
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
//加密字符串所用函数
std::vector<bm::int1024_t> RSA::ecrept(std::string& str_in, bm::int1024_t ekey, bm::int1024_t pkey)
{
	std::vector<bm::int1024_t> vecout;//每个单元存储一个bm::int1024_t类型的数据
	for (const auto& e : str_in)
	{
		vecout.push_back(ecrept((bm::int1024_t)e, ekey, pkey));//每加密一下字符串，就进行一次尾插
	}
	return vecout;
}
//加密字符串所用函数
std::string RSA::decrept(std::vector<bm::int1024_t>& ecrept_str, bm::int1024_t dkey, bm::int1024_t pkey)
{
	std::string strout;
	for (const auto& e : ecrept_str)
	{
		strout.push_back((char)ecrept(e, dkey, pkey)); //每解密一个字符就尾插一次
	}
	return strout;
}
//打印加密过后字符串的函数
void RSA::printInfo(std::vector<bm::int1024_t>& ecrept_str)
{
	for (const auto& e : ecrept_str)
	{
		std::cout << e << " ";
	}
	std::cout << std::endl;
}

//用来加密或解密的算法
bm::int1024_t RSA::ecrept(bm::int1024_t msg, bm::int1024_t ekey, bm::int1024_t pkey)
{
	//在加密时需要进行模幂运算，因为像a ^ b % c都是基于大数的运算
	//如果直接暴力计算，那么效率就太慢了，所以会用这个算法进行优化
	//(a +/- b) % c = ((a % c +/- b % c)) % c
	//(a * b) % c = ((a % c) * (b % c)) % c
	//a^b % c = (a * a * a……a) % c
	//= ((a%c)*(a%c)*(a%c)*……*(a%c)) % c
	//= ((a % c) ^ b) % c
	//然后开始证明
	//对于任何一个整数的模幂运算：a^b % c
	//对于b展开它的二进制形式
	//b = b0 * 2 ^ 0 + b1 * 2 ^ 1 + ... + bn * 2 ^ n 
	//所以 a ^ b = a ^ (b0 * 2 ^ 0 + b1 * 2 ^ 1 + ... + bn * 2 ^ n )
	//           = a ^ (b0 * 2 ^ 0) * a ^ (b1 * 2 ^ 1) * ... * a ^ (bn * 2 ^ n)
	//因为我们的b0到bn对应的是b的 二进制系数，为0或者1，如果bi为0，那么a的指数为0
	//那么那一项的值为1，因为它们是连乘的形式，所以结果不会改变，所以可以把这些为bi为0的清除掉，
	//所以 a ^ b = a ^ (bi * 2 ^ i) * ... * a ^ (bn * 2 ^ n), bi != 0
	//           = a ^ (2 ^ i) * ... * a ^ (2 ^ n)
	//所以 a ^ b % c = (a ^ (2 ^ i % c) * ... * a ^ (2 ^ n % c)) % c
	//将这项分别用A代替，就变成了 a ^ b % c = (Ai * .. * An) % c
	//其中A0 = a ^ (2 ^ 0) % c，A1 = a ^ (2 ^ 1) % c
	//    A(n - 1) = a ^ (2(n - 1)) % c，An = a ^ ( 2 ^ n) % c
	//可以看出 An = (a ^ (2(n - 1))) ^ 2 % c
	//         An = (a ^ (2(n - 1)) * a ^ (2(n - 1))) % c
	//因为(a * b) % c = ((a % c) * (a % c)) % c
	//所以 An = (a ^ (2(n - 1)) % c * a ^ (2(n - 1)) % c) % c
	//所以 An = (A(n - 1) * A(n - 1)) % c
	//也就是说每一项的值都可以用它的前项值来计算，算法就是用前一项值的平方取余
	//不过要注意的是，这是在默认每一项的值都不为1时的算法（也就是将b二进制展开后，每一位都不为0)
	//但是一般情况下肯定会出现为0的位，所以在真正进行计算时，我们都是用第一个不为0的位来计算下面每一项的值
	//然后将中间值为0的位也按前一项值的平方，然后取余这种方法来算，虽然这一项的值用这个方法来算的值是错的
	//但是下一个不为0的位，可以用上一项的值来计算，所以不为0的位的值是正确的
	//而为0的位，我们根本不需要进行累乘，因为1乘任何数都等于那个数

	bm::int1024_t msg_out = 1; //要保存最终的加密后的数据的变量，赋为1的原因是第一位不为0的位
							   //我们可以直接计算出来，不需要用前一项，只要保证计算出来的值不变就行了
	bm::int1024_t a = msg;     //需要加密的数据
	bm::int1024_t c = pkey;    //n值
	bm::int1024_t b = ekey;    //公钥
	while (b)                  //只要还有一位不为0，就代表没有计算完毕
	{
		if (b & 1)             //只累乘不为0的位
		{
			msg_out = (msg_out * a) % c;  //计算的方法，前一项值的平方，然后取余
		}
		a = (a * a) % c;       //前一项的值
		b >>= 1;               //累乘一次，清零一位
	}
	return msg_out;
}

bm::int1024_t RSA::produce_prime()
{
	boost::random::mt19937 gen(time(nullptr));  //用来产生大数类型的随机数，先进行初始化
												//用于控制随机数的范围为2 ~ 1<<111
	boost::random::uniform_int_distribution<bm::int1024_t> dist(2, bm::int1024_t(1) << 111);
	bm::int1024_t prime = 0;    //用来保存产生的素数

	while (1)
	{
		prime = dist(gen);
		if (is_Bigprime(prime)) //判断是否为素数
		{
			break;
		}
	}
	return prime;
}
//用来判断是否为素数的函数
bool RSA::is_Bigprime(bm::int1024_t digit)
{
	boost::random::mt11213b gen(time(nullptr)); //一个大数的随机数生成器
	if (miller_rabin_test(digit, 25, gen))      //boost中用于测试是否为素数的函数
	{                                           //第一个参数就是要被测试的素数
		if (miller_rabin_test((digit - 1) / 2, 25, gen));
		{                                       //第二个参数是测试次数，因为大数的测试和普通数并不相同
			return true;                        //如果采取普通测试方法，效率太低，所以是用了一些其它的算法来进行测试
		}                                       //miller_rabin这个算法得出的结果，如果为false，那么说明这个大数是合数
	}                                           //如果结果为true，那么说明这个大数可能是质数，为了降低出错的可能性
	return false;                               //建议测试25次，得到正确的结果
}                                               //第三个参数是一个大数的随机数生成，这个参数是可选项
												//但并不建议使用自带的生成器，建议重传一个其它的生成器，以保证这个函数的判断结果是正确的
												//当这个函数判断完成后，得出的结果可能为素数，所以要再测试这个大数减1除2后，是否为素数
												//如果这时对这个数的判断结果还为true，那么说明这个素数是安全的
												//用来生成公钥、私钥和n
void RSA::produce_keys()
{
	bm::int1024_t prime1 = produce_prime();
	bm::int1024_t prime2 = produce_prime(); //产生两个素数用来生成n
	while (prime1 == prime2)                //产生的这两个素数不能是相同的
	{
		prime2 = produce_prime();
	}
	_key.pkey = produce_pkey(prime1, prime2);//生成n
	bm::int1024_t orla = produce_orla(prime1, prime2); //得到欧拉函数值
	_key.ekey = produce_ekey(orla);          //产生公钥
	_key.dkey = produce_dkey(_key.ekey, orla);//产生私钥
}

bm::int1024_t RSA::produce_pkey(bm::int1024_t prime1, bm::int1024_t prime2)
{
	std::cout << "公钥:" << prime1 * prime2 << std::endl; //n = prime1 * prime2
	return prime1 * prime2;
}
bm::int1024_t RSA::produce_orla(bm::int1024_t prime1, bm::int1024_t prime2)
{
	//欧拉函数是小于x的正整数中与x互质的数的数目
	//因为欧拉函数是积性函数，所以φ(mn) = φ(m) * φ(n)
	//如果m是质数，则φ(m) = m - 1
	//所以φ(mn) = (m - 1) * (n - 1)
	//所以orla = (prime1 - 1) * (prime2 - 1)
	std::cout << "orla:" << (prime1 - 1) * (prime2 - 1) << std::endl;
	return (prime1 - 1) * (prime2 - 1);
}
//生成公钥的函数
bm::int1024_t RSA::produce_ekey(bm::int1024_t orla)
{
	// 公钥e的条件是1< e < φ(n)，且e与φ(n) 互质。
	bm::int1024_t ekey;   //用来存放公钥
	srand(time(nullptr)); //用来生成随机公钥，这样更安全，先进行初始化
	while (1)
	{
		ekey = rand() % orla; //生成一个大于等于0，小于orla值的公钥
		if (ekey > 1 && produce_gcd(ekey, orla) == 1) //要求公钥必需大于1，且与orla值互质
		{
			break;
		}
	}
	std::cout << "加密钥:" << ekey << std::endl;
	return ekey;
}
//用来判断两数是否互质，也就最大公约为1
bm::int1024_t RSA::produce_gcd(bm::int1024_t ekey, bm::int1024_t orla)
{
	//使用辗转相除法来计算最大公约数
	//辗转相除法的算法是，让a和b取模，然后算出余数c，然后让b和c取模，算出d，就像是b跑到a的位置上
	//c跑到b的位置上，d是余数，最后当b这个位置是0时，表示a就是最大公约数
	bm::int1024_t residual;
	while (residual = ekey % orla)
	{
		ekey = orla;
		orla = residual;
	}
	return orla;
}

//获取模反元素，也就是私钥
bm::int1024_t RSA::exgcd(bm::int1024_t ekey, bm::int1024_t orla, bm::int1024_t& x, bm::int1024_t& y)
{
	//因为模反元素d要求(公钥e)，ed≡1 mod φ(n)，所以如果我们用暴力搜索的方法，那么效率就会差很多
	//所以利用扩展欧几里德公式就可以更容易得结果，效率更好
	//扩展欧几里德公式，说的是在用辗转相除求出最大公约数(gcd)的同时，还可以求出一组ax + by = gcd的解
	//现在我们假设公钥e是a，n是b，ed≡1 mod φ(n)可以写成，ed - 1 = kφ(n)，所以ed - kφ(n) = 1
	//但是因为k是未知数，而且我们并不是很关心k的值，所以也可以写成ed + kφ(n) = 1
	//所以d就相当于扩展欧几里德公式中的x，k就相当于y，因为e和φ(n)是已知的，所以可以求出d和k
	//ax + by = gcd，在这个式子中，当b等于0时，ax = gcd，此时我们返回a的值，a就是所求的最大公约
	//因为在求最大公约数时，用的辗转相除法,所以b的值会在每次计算过程中进行改变,直到最后b的值等式变成了0，才表示a是最大公约数
	//辗转相除法的算法是，让a和b取模，然后算出余数c，然后让b和c取模，算出d，就像是b跑到a的位置上
	//c跑到b的位置上，d是余数，最后当b这个位置是0时，表示a就是最大公约数)
	//故此时x = 1(ed + kφ(n) = 1)因为y可以是任意值，所以，简单的一组解即为(1, 0)
	//这是在最后一层得到的，并不是我们想要的解，我们要的是最初a和b的值，所求的解
	//因为根据辗转相除法，递归的时候，在获取最终结果之前，已经求出了最终状态，b 和 a%b，改变位置后，b就是a，a%b取余就是b
	//因为我们知道最终状态一定会求得一组解(1,0)，假设这组解是(x1, y1)，那么在它的上一层，就知道有这么一组解
	//所以bx1 + (a % b)y1 = 1，我们是可以得到这个式子的
	//而这里 a % b = a - [a / b] * b， 其中[a / b]表示向下取整， 带入上述等式
	//bx1 + (a - [a / b] * b)y1 = 1
	//bx1 + ay1 - [a / b] * by1 = 1
	//ay1 + b(x1 - [a / b]y1) = 1
	//所以可以得到x = y1，y = x1 - [a / b]y1
	//这样将(x1, y1)带入，就得到了这一层的x和y的解
	//同样道理可以继续得到更上一层的解，到最后就能得到初始状态中的(x, y)解
	//ed + kφ(n) = 1，所以d就是x，k就是y(y没什么用)，就可以得到e的模反元素d

	if (orla == 0) //它就是b，当orla等于0时，说明到最终状态了
	{
		x = 1;    //始解
		y = 0;
		return ekey;
	}

	bm::int1024_t ret = exgcd(orla, ekey % orla, x, y); //先得到始解，才能根据始解往回溯，得到最终解

	bm::int1024_t x1 = x, y1 = y;   //得到始解后，根据公式向上递推
	x = y1;
	y = x1 - (ekey / orla) * y1;
	return ret;
}
//用来产生私钥的函数
bm::int1024_t RSA::produce_dkey(bm::int1024_t ekey, bm::int1024_t orla)
{
	bm::int1024_t x, y;
	exgcd(ekey, orla, x, y); //返回的x值就是私钥，只不过可能为负数，所以要使用一个算法
							 //当它是负数时，转换为等值的正数，如果是正数那么就不会变
	std::cout << "解密钥:" << (x % orla + orla) % orla << std::endl;
	return (x % orla + orla) % orla;
}