#pragma once

#include <glad/glad.h>
#include "glm/glm.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <vector>
#include "stb_image.h"


class TextureLoader
{

public:
	TextureLoader();
	bool ScanDirectory(const std::string& inPath);
	const bool IsBankEmpty() const { return (data.size() > 0) ? false : true; }
	void EmptyBankContents();
	std::vector<unsigned char*> GetTextures() const { return data; }
	std::vector<unsigned int> TextureIDs;

private:
	std::filesystem::path targetDir;
	std::vector<std::filesystem::path> TextureBank;
	std::vector<unsigned char*> data;
	

	// Debug tools
	bool debugMode = false;
};



