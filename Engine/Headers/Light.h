#pragma once
#include "../Headers/Component.h"

class TestComponent : public Component
{
public:
	std::string compName = "";
	void ShowImGuiPanel() override
	{
		using namespace ImGui;
		std::string headerLabel = fmt::format("Test Component##{}", compName);
		if (CollapsingHeader(headerLabel.c_str()))
		{

			std::string s = fmt::format("This is a test component. {}", compName);

			PushTextWrapPos(GetWindowContentRegionMax().x);
			TextWrapped(s.c_str());
			PopTextWrapPos();
		}
		Separator();
	}
};

enum LightType
{
	DIRECTIONAL,
	POINTLIGHT,
	SPOT, 
	COUNT
};



class Light : public Component
{
public:
	ImVec4 LightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	float lightIntensity = 1.0f;
	LightType lightType = LightType::POINTLIGHT;
	
	
	void ShowImGuiPanel() override
	{
		using namespace ImGui;
		if (CollapsingHeader("Light"))
		{
			PushTextWrapPos(GetWindowContentRegionMax().x);
			TextWrapped("Light Type:");
			const char* lightTypeNames[] = { "Directional", "Point", "Spot" };
			if (Combo("##LightType", reinterpret_cast<int*>(&lightType), lightTypeNames, LightType::COUNT))
			{
				switch (lightType)
				{
				case LightType::DIRECTIONAL:
					lightType = LightType::DIRECTIONAL;
					break;
				case LightType::POINTLIGHT:
					lightType = LightType::POINTLIGHT;
					break;
				case LightType::SPOT:
					lightType = LightType::SPOT;
					break;
				}
			}
			Spacing();
			TextWrapped("Light Color:");
			ColorEdit3("##lightColor", (float*)&LightColor, ImGuiColorEditFlags_PickerHueWheel);
			TextWrapped("Intensity:");
			InputFloat("##lightIntensity", &lightIntensity);
			
			PopTextWrapPos();

		}
		Separator();
	}
};