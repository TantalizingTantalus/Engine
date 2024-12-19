// Transform.h
#pragma once  // or use include guards

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "../Headers/Component.h"
#include <vector>

class Transform : public Component
{
protected:
	glm::mat4 getLocalModelMatrix()
	{
		const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		// Y * X * Z
		const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

		// translation * rotation * scale 
		return glm::translate(glm::mat4(1.0f), position) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
	}
public:
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }; 
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	bool m_isDirty = true;

	void ShowImGuiPanel() override {
		using namespace ImGui;
		if (CollapsingHeader("Transform")) {
			PushTextWrapPos(GetWindowContentRegionMax().x);
			AlignTextToFramePadding();
			TextWrapped("Position");
			SameLine();
			if(InputFloat3("##Position", glm::value_ptr(position)))
			{
				setLocalPosition(position);
			}
			TextWrapped("Rotation");
			SameLine();
			if (InputFloat3("##Rotation", glm::value_ptr(rotation)))
			{
				setLocalRotation(rotation);
			}
			TextWrapped("Scale");
			SameLine();
			if (InputFloat3("##Scale", glm::value_ptr(scale)))
			{
				setLocalScale(scale);
			}

			Separator();
			/*TextWrapped("Model Matrix");
			TextWrapped("Row 1: %.3f %.3f %.3f %.3f", m_modelMatrix[0][0], m_modelMatrix[0][1], m_modelMatrix[0][2], m_modelMatrix[0][3]);
			TextWrapped("Row 2: %.3f %.3f %.3f %.3f", m_modelMatrix[1][0], m_modelMatrix[1][1], m_modelMatrix[1][2], m_modelMatrix[1][3]);
			TextWrapped("Row 3: %.3f %.3f %.3f %.3f", m_modelMatrix[2][0], m_modelMatrix[2][1], m_modelMatrix[2][2], m_modelMatrix[2][3]);
			TextWrapped("Row 4: %.3f %.3f %.3f %.3f", m_modelMatrix[3][0], m_modelMatrix[3][1], m_modelMatrix[3][2], m_modelMatrix[3][3]);*/

			PopTextWrapPos();
		}
		Separator();
	}

	bool DecomposeTransform()
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(m_modelMatrix);

		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		
		position = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		
#if 0
		Pdum3 = cross(Row[1], Row[2]);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		m_isDirty = true;

		return true;

	}


	// Ripped logic from The Cherno - Hazel Engine, Thanks my guy! Link -  https://github.com/TheCherno/Hazel/blob/1feb70572fa87fa1c4ba784a2cfeada5b4a500db/Hazelnut/src/EditorLayer.cpp#L325
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rot, glm::vec3& scl)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scl.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scl.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scl.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rot.y = asin(-Row[0][2]);
		if (cos(rot.y) != 0) {
			rot.x = atan2(Row[1][2], Row[2][2]);
			rot.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rot.x = atan2(-Row[2][0], Row[1][1]);
			rot.z = 0;
		}

		m_isDirty = true;

		return true;
	}

	bool DecomposeModelMatrix(const glm::mat4& modelMatrix, glm::vec3& translation, glm::vec3& rot, glm::vec3& scl)
	{
		
		glm::mat4 identityMatrix(1.0f);

		
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat orientation;  

		if (!glm::decompose(modelMatrix, scl, orientation, translation, skew, perspective))
			return false;  

		
		rot = glm::eulerAngles(orientation);

		
		rot = glm::degrees(rot);

		return true;  
	}

	void updatePositionMatrix()
	{
		if (m_isDirty)
			m_modelMatrix = glm::translate(glm::mat4(1.0f), position);
		

		m_isDirty = false;
	}

	void updateModelMatrix()
	{
		if (m_isDirty)
		{
			
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 rotationMatrix = glm::mat4(1.0f); // Identity by default

			
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // X-axis
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Z-axis

			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

			//  T * R * S
			m_modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

			m_isDirty = false;
		}
	}

	

	

	void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
	{
		m_modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
		m_isDirty = false;
	}

	void setModelMatrix(const glm::mat4& m_matrix)
	{
		m_modelMatrix = m_matrix;
		m_isDirty = true;
	}

	void setLocalPosition(const glm::vec3& newPosition)
	{
		position = newPosition;
		m_isDirty = true;
		updateModelMatrix();
	}

	void setLocalRotation(const glm::vec3& newRotation)
	{
		rotation = newRotation;
		m_isDirty = true;
		updateModelMatrix();
	}

	void setLocalScale(const glm::vec3& newScale)
	{
		scale = newScale;
		m_isDirty = true;
		updateModelMatrix();
	}

	const glm::vec3& getGlobalPosition() const
	{
		return m_modelMatrix[3];
	}

	const glm::vec3& getLocalPosition() 
	{
		DecomposeTransform();
		return position;
	}

	const glm::vec3& getLocalRotation() 
	{
		DecomposeModelMatrix(m_modelMatrix, position, rotation, scale);
		return rotation;
	}

	const glm::vec3& getLocalScale() const
	{
		return scale;
	}

	glm::mat4& getModelMatrix() 
	{
		return m_modelMatrix;
	}

	glm::vec3 getRight() const
	{
		return m_modelMatrix[0];
	}


	glm::vec3 getUp() const
	{
		return m_modelMatrix[1];
	}

	glm::vec3 getBackward() const
	{
		return m_modelMatrix[2];
	}

	glm::vec3 getForward() const
	{
		return -m_modelMatrix[2];
	}

	glm::vec3 getGlobalScale() const
	{
		return { glm::length(getRight()), glm::length(getUp()), glm::length(getBackward()) };
	}

	bool isDirty() const
	{
		return m_isDirty;
	}
};