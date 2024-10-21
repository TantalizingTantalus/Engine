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

class Light : public Component
{
public:
	ImVec4 LightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	void ShowImGuiPanel() override
	{
		using namespace ImGui;
		if (CollapsingHeader("Light"))
		{
			PushTextWrapPos(GetWindowContentRegionMax().x);
			TextWrapped("Light Color:");
			ColorEdit3("##lightColor", (float*)&LightColor);
			PopTextWrapPos();

		}
		Separator();
	}
};