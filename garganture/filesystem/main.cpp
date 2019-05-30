#include <iostream>
using namespace std;






void CommandInterpreter(int argc, char ** argv)
{
	if (argc <= 2) return;

	auto option = std::string(argv[1]);

	if (option == "-q")
	{

	}
	else if (option == "-i")
	{

	}
	else
	{
		cout << "DO NOT SET OPTION" << endl;
	}
}


int main(int argc, char* argv[])
{
	CommandInterpreter(argc, argv);

#ifdef _DEBUG
	char a;
	cin >> a;
#endif

	return 0;
}