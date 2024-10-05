#pragma once
#include "../Headers/Model.h"


class Util
{
public:
	static std::string WideStringToString(const std::wstring& wstr);
	static void NormaliseVector3(glm::vec3& input);
	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& position, glm::vec3& rotation, glm::vec3& scale);
    
};