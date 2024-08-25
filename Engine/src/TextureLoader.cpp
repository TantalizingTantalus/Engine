#include "../Headers/TextureLoader.h"

TextureLoader::TextureLoader()
{
	std::cout << "Starting TextureLoader" << std::endl;
}

bool TextureLoader::ScanDirectory(const std::string& inPath)
{


	TextureLoader::targetDir = inPath;

	std::cout << "Searching " << TextureLoader::targetDir << " for any textures..." << std::endl;

	// Fail if does not exist
	if (!std::filesystem::exists(TextureLoader::targetDir))
	{
		std::cout << "Filesystem could not find path: " << TextureLoader::targetDir << "\nCheck the folder/directory exists and try again...\n" << std::endl;
		return false;
	}

	// Fail if not a folder/directory
	if (!std::filesystem::is_directory(TextureLoader::targetDir))
	{
		std::cout << "Path at " << TextureLoader::targetDir << " is not a directory, try again with folder path...\n" << std::endl;
		return false;
	}

	// Fail if empty
	if (std::filesystem::is_empty(TextureLoader::targetDir))
	{
		std::cout << "Directory at " << TextureLoader::targetDir << " is empty. No textures were loaded." << std::endl;
		return false;
	}

	// Cache contents to TextureBank, subsequently converting to usable stb_image texture
	for (auto& entry : std::filesystem::directory_iterator(TextureLoader::targetDir))
	{
		TextureLoader::TextureBank.push_back(entry.path());

		unsigned int textureID;
		int width, height, nrChannels;
		unsigned char* textureData = stbi_load(entry.path().string().c_str(), &width, &height, &nrChannels, 0);

		if (textureData)
		{
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
			glGenerateMipmap(GL_TEXTURE_2D);

			TextureIDs.push_back(textureID);
			stbi_image_free(textureData);
		}
		else
		{
			std::cout << "failed to parse textureData, reason: " << stbi_failure_reason() << std::endl;
		}
	}

	// Display the textures found in console
	std::cout << "Total " << TextureBank.size() << " textures found at " << TextureLoader::targetDir << std::endl;
	for (int i = 0; i < TextureBank.size(); i++)
	{
		std::cout << "\t - \t" << TextureBank[i].string() << std::endl;
	}


	// Handle Debug Mode
	if (TextureLoader::debugMode)
	{
		std::cout << "\n(Debug Mode) - Textures found:" << std::endl;
		for (std::filesystem::path& tex : TextureLoader::TextureBank)
		{
			std::cout << tex.filename().string() << std::endl;
		}
	}
	return true;
}

void TextureLoader::EmptyBankContents()
{
	TextureLoader::TextureBank.clear();
}