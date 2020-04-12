#include "RSA.h"


void testRandom()
{
	boost::random::mt19937 gen(time(nullptr));
	std::cout << "random" << std::endl;
	boost::random::uniform_int_distribution<bm::cpp_int> dist(0, bm::cpp_int(1) << 768);
	std::cout << dist(gen) << std::endl;
}

void testBigInter()
{
	bm::cpp_int a("361583405834065348628057364011369546453213168134145671380500516385473891506316583");
	std::cout << a << std::endl;
	std::cout << a / 2 << std::endl;
	std::cout << a % 2 << std::endl;
	std::cout << a % 9 << std::endl;
}

void testString()
{
	RSA rsa;
	Key key = rsa.getKey();
	std::string strin;
	std::cout << "输入加密信息" << std::endl;
	std::cin >> strin;
	std::vector<bm::int1024_t> strecrept = rsa.ecrept(strin, key.ekey, key.pkey);
	std::string strdecrept = rsa.decrept(strecrept, key.dkey, key.pkey);
	std::cout << "加密信息" << std::endl;
	rsa.printInfo(strecrept);
	std::cout << "解密信息" << std::endl;
	std::cout << strdecrept << std::endl;
}

void testFile()
{
	RSA rsa;
	Key key = rsa.getKey();
	rsa.ecrept("plain_inp.txt", "eplain_out.txt", key.ekey, key.pkey);
	rsa.decrept("eplain_out.txt", "dplain_out.txt", key.dkey, key.pkey);
}

int main()
{
	//testString();
	testFile();
	//testBigInter();
	//testRandom();

	system("pause");
	return 0;
}