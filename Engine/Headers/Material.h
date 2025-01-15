#pragma once
#include "Component.h"
#include "Model.h"


unsigned int TextureFromFile(const char* path, std::string directory, unsigned int placeholderID);


class Material : public Component
{
public:
	float m_Shininess = 2.5f;
	float m_SpecIntensity = 1.0f;
	std::string directory;

	// Model is tightly coupled to the material component
	// as of now.
	Model* pModel = nullptr;

	Material(Entity* pMod)
	{
		if (pMod->HasComponent<Model>())
		{
			pModel = &pMod->GetComponent<Model>();
			pModel->SetShader(pMod->GetComponent<Model>().GetShader());
		}
	}

	bool IsNormalMap(const std::filesystem::path& texturePath)
	{
		std::string filename = texturePath.filename().string();
		return filename.find("normal") != std::string::npos || filename.find("norm") != std::string::npos;
	}

	Texture OpenTextureDiag()
	{

		Texture selectedTexture;
		wchar_t fileName[MAX_PATH] = L"";
		std::string logMsg;

		// Initialize OPENFILENAME structure
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;  
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = sizeof(fileName) / sizeof(fileName[0]);  
		ofn.lpstrFilter = L"Any(.png, .jpeg, .jpg)\0*.png;*.jpeg;*.jpg\0.png\0*.jpeg\0.jpg\0*.png\0.jpg\0*.jpeg\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Display the Open dialog box
		if (GetOpenFileName(&ofn) == TRUE)
		{
			const std::wstring& wst = ofn.lpstrFile;
			std::string ofName(wst.begin(), wst.end());
			std::filesystem::path path(ofName);

			if (std::filesystem::exists(path))
			{
				selectedTexture.id = TextureFromFile(path.string().c_str(), directory, (int)SystemIcons::GetPlaceholderIcon());
				selectedTexture.path = path.string();

				return selectedTexture;
			}


		}
		else
		{
			selectedTexture.path = "";
			return selectedTexture;
		}
	}
	void Initialize(Model& inMod)
	{
		pModel = &inMod;
	}

	void ShowImGuiPanel() override
	{
		using namespace ImGui;

		if (CollapsingHeader("Material"))
		{
			PushTextWrapPos(GetWindowContentRegionMax().x);
			AlignTextToFramePadding();
			//TextWrapped(fmt::format("Shader ID: {}", pModel->GetShader().ID).c_str());
			SeparatorText("Shininess ");
			SliderFloat("##materialShininess", &m_Shininess, 0.1f, 32.0f, "%.2f");
			char textureBuffer[100];
			SeparatorText("Specular Intensity ");
			SliderFloat("##materialSpecIntensity", &m_SpecIntensity, 0.0f, 2.0f, "%.2f");
			SeparatorText("Textures");
			Spacing();
			if (CollapsingHeader("Textures"))
			{
				
				for (int i = 0; i < pModel->textures_loaded.size(); i++)
				{
					std::filesystem::path someFileName = pModel->textures_loaded[i].path;

					strcpy_s(textureBuffer, someFileName.filename().string().c_str());
					if (ImageButton(fmt::format("##{}label", i).c_str(), (ImTextureID)(intptr_t)pModel->textures_loaded[i].id, ImVec2(200, 200)))
					{
						std::string tempDir = std::filesystem::current_path().string();
						Texture tempTex = OpenTextureDiag();

						if (tempTex.path.empty())
						{
							return;
						}

						pModel->textures_loaded[i].id = tempTex.id;
						pModel->textures_loaded[i].path = tempTex.path;

						std::filesystem::current_path(tempDir);

					}
					TextWrapped(textureBuffer);
					Spacing();
				}
				Spacing();


				if (Button("Add Texture", ImVec2(200, 50)))
				{
					std::string tempDir = std::filesystem::current_path().string();
					Texture tempTex = OpenTextureDiag();
					if (tempTex.path.empty())
					{
						return;
					}
					if (IsNormalMap(tempTex.path))
					{
						tempTex.type = "material.texture_normal";

						pModel->hasNormal = true;
					}
					pModel->textures_loaded.push_back(tempTex);

					std::filesystem::current_path(tempDir);
				}
			}
			


			PopTextWrapPos();
		}
	}
private:

};