#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> cardinal = { 1,2,3,4,5};

	int total_elements = 1;
	for_each(cardinal.begin(), cardinal.end(), 
			[&total_elements](int i){ total_elements *= i; } );
	
	cout << total_elements << endl;	
	return 0;
}
