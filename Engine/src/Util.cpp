#include "../Headers/Util.h"

std::string Util::WideStringToString(const std::wstring& wstr)
{
	// Convert wstring to string
	std::string str(wstr.begin(), wstr.end());
	return str;
}