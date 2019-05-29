#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	cout << argc << endl;

#ifdef _DEBUG
	char a;
	cin >> a;
#endif

	return 0;
}