#pragma once
#include "../Headers/Component.h"

class Light : public Component
{
public:
	ImVec4 LightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	void ShowImGuiPanel() override
	{
		if (ImGui::CollapsingHeader("Light"))
		{
			ImGui::Text("Light Color:");
			ImGui::ColorEdit3("##lightColor", (float*)&LightColor);

		}
		ImGui::Separator();
	}
};