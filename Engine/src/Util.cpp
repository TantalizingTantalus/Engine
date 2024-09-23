#include "../Headers/Util.h"

std::string Util::WideStringToString(const std::wstring& wstr)
{
	// Convert wstring to string
	std::string str(wstr.begin(), wstr.end());
	return str;
}

void Util::NormaliseVector3(glm::vec3& input)
{
	float len = input.length();
	if (len != 0)
	{
		input.x /= len;
		input.y /= len;
		input.z /= len;
	}
}