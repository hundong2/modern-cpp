#include <iostream>
#include <vector>
using namespace std;

vector<int> cardinal = { 5, 1, 2, 3, 4 };
template <typename T>
struct product{
	product(T& storage) : value(storage){}
	template<typename V>
	void operator()(V& v){
		value *= v;
	}
	T& value;
};

int main()
{
	int total_elements = 1;
	for_each(cardinal.begin(), cardinal.end(), product<int>(total_elements));
	cout << total_elements << endl;
	
	return 0;
}
