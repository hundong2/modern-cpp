#include <iostream>
#include <vector>

using namespace std;

struct mod
{
	mod(int m) : modules(m) {}
	int operator()(int v) { return v % modules; }
	int modules;
};


void test1()
{
	vector<int> in = { 1,2,3,4,5};
	vector<int> out(5, 1);
	for_each(out.begin(), out.end(), [](int i){ cout << i << endl; });
	int my_mod = 8;
	transform(in.begin(), in.end(), out.begin(),
			[my_mod](int v) -> int { return v % my_mod; });
}

void test2()
{
	vector<int> in = {1,2,3,4,5};
	vector<int> out(5);
	int my_mod = 8;
	transform(in.begin(), in.end(), out.begin(), mod(my_mod));
}
int main()
{
	test1();
}
