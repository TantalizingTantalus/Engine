
#include "../Headers/Backend.h"
#include <exception>



Backend Program;

int main()
{
	try
	{
		
		const bool RunResult = Program.Run();
		if (!RunResult)
		{
			std::cerr << "Ran into issues during Run()" << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Ran into issues at: " << e.what() << std::endl;
	}
	
}




