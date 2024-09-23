#pragma once
#include "../Headers/Model.h"


class Util
{
public:
	static std::string WideStringToString(const std::wstring& wstr);
	static void NormaliseVector3(glm::vec3& input);
};