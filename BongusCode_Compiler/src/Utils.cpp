#include "Utils.h"
#include <iostream>
#include <filesystem>

void Utils::PrintCurrentWorkingDirectory(void)
{
	std::cout << "Current working directory:\n" << std::filesystem::current_path() << std::endl;
}
