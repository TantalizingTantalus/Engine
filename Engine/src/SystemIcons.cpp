#include "../Headers/SystemIcons.h"

void SystemIcons::InitializeErrorTexture() {
    unsigned char errorData[4] = { 255, 0, 255, 255 }; // Magenta pixel
    glGenTextures(1, &errorTextureID);
    glBindTexture(GL_TEXTURE_2D, errorTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, errorData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint SystemIcons::LoadFileIconID(const char* path) {
    GLuint textureID;
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);

    if (!data) {
        spdlog::error(fmt::format("\nError loading icon at path:\n{}\nReason:\n{}", path, stbi_failure_reason()));
        return errorTextureID; // Use error texture
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return textureID;
}

void SystemIcons::Initialize() {
    InitializeErrorTexture();

    folderIcon = LoadFileIconID(fileFolderIconPath.c_str());
    fileIcon = LoadFileIconID(fileFileIconPath.c_str());
    backButtonIcon = LoadFileIconID(backButtonIconPath.c_str());
    placeholderIcon = LoadFileIconID(placeholderIconPath.c_str());
}

void SystemIcons::Shutdown() {
    glDeleteTextures(1, &folderIcon);
    glDeleteTextures(1, &fileIcon);
    glDeleteTextures(1, &backButtonIcon);
    glDeleteTextures(1, &placeholderIcon);
    glDeleteTextures(1, &errorTextureID);
}