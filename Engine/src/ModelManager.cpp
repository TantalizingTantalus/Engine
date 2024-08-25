#include "../Headers/ModelManager.h"

//ModelManager::ModelManager()
//{
//	std::cout << "Initializing ModelManager" << std::endl;
//}
//
//void ModelManager::LoadModel(const std::string& path)
//{
//    Assimp::Importer importer;
//    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
//
//    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
//        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
//        return;
//    }
//
//    std::cout << "Model loaded successfully" << std::endl;
//}