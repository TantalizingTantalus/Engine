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

bool Util::DecomposeTransform(const glm::mat4& transform, glm::vec3& position, glm::vec3& rotation, glm::vec3& scale)
{
    // Extract translation
    position = glm::vec3(transform[3]);

    // Extract scale
    scale = glm::vec3(
        glm::length(transform[0]),
        glm::length(transform[1]),
        glm::length(transform[2])
    );

    // Remove scale from the matrix
    glm::mat4 rotationMatrix = transform;
    rotationMatrix[0] /= scale.x;
    rotationMatrix[1] /= scale.y;
    rotationMatrix[2] /= scale.z;

    // Extract rotation (this is tricky and depends on the order of rotation)
    rotation.y = glm::atan(rotationMatrix[0][2], rotationMatrix[2][2]);
    rotation.x = glm::asin(-rotationMatrix[1][2]);
    rotation.z = glm::atan(rotationMatrix[1][0], rotationMatrix[1][1]);

    return true;
}