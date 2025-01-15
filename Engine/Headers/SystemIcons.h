#pragma once
#include "Component.h"


class SystemIcons {
private:
    inline static GLuint errorTextureID = 0;
    inline static GLuint folderIcon = 0;
    inline static GLuint fileIcon = 0;
    inline static GLuint backButtonIcon = 0;
    inline static GLuint placeholderIcon = 0;

    inline static const std::string fileFolderIconPath = "Textures/Engine/4.png";
    inline static const std::string fileFileIconPath = "Textures/Engine/1.png";
    inline static const std::string backButtonIconPath = "Textures/Engine/UpArrow.png";
    inline static const std::string placeholderIconPath = "Textures/Engine/placeholder.png";

    static GLuint LoadFileIconID(const char* path);
    static void InitializeErrorTexture();

public:
    static void Initialize();
    static void Shutdown();

    static ImTextureID GetFolderIcon() { return (void*)(intptr_t)folderIcon; }
    static ImTextureID GetFileIcon() { return (void*)(intptr_t)fileIcon; }
    static ImTextureID GetBackButtonIcon() { return (void*)(intptr_t)backButtonIcon; }
    static ImTextureID GetPlaceholderIcon() { return (void*)(intptr_t)placeholderIcon; }
};
