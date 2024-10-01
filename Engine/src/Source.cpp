
#include "../Headers/Backend.h"
#include <exception>



Backend Program;

int main()
{
	try
	{
		
		Program.Initialize();
		Program.Update();
	}
	catch (std::exception& e)
	{
		std::cerr << "Ran into issues at: " << e.what();
	}
	
}




