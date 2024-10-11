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

		// translation * rotation * scale (also know as TRS matrix)
		return glm::translate(glm::mat4(1.0f), position) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
	}
public:
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }; //In degrees
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
	bool m_isDirty = true;

	void ShowImGuiPanel() override {
		if (ImGui::CollapsingHeader("Transform Component")) {
			ImGui::InputFloat3("Position", glm::value_ptr(position));
			ImGui::InputFloat3("Rotation", glm::value_ptr(rotation));
			ImGui::InputFloat3("Scale", glm::value_ptr(scale));
		}
	}

	bool DecomposeTransform()
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(m_modelMatrix);

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
		position = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
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

		//// Extract translation
		//position = glm::vec3(m_modelMatrix[3]);

		//// Extract scale
		//scale = glm::vec3(
		//	glm::length(m_modelMatrix[0]),
		//	glm::length(m_modelMatrix[1]),
		//	glm::length(m_modelMatrix[2])
		//);

		//// Remove scale from the matrix
		//glm::mat4 rotationMatrix = m_modelMatrix;
		//rotationMatrix[0] /= scale.x;
		//rotationMatrix[1] /= scale.y;
		//rotationMatrix[2] /= scale.z;

		//// Extract rotation (this is tricky and depends on the order of rotation)
		//rotation.z = rotationMatrix[1].z;
		//rotation.x = rotationMatrix[1].x;
		//rotation.y = rotationMatrix[1].y;

		//m_isDirty = true;

		//return true;
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
		// Identity matrix to check for errors
		glm::mat4 identityMatrix(1.0f);

		// Decompose the matrix into translation, rotation (quaternion), and scale
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat orientation;  // This will hold the rotation in quaternion form

		// Use glm::decompose to extract the values from the model matrix
		if (!glm::decompose(modelMatrix, scl, orientation, translation, skew, perspective))
			return false;  // Decomposition failed

		// Convert quaternion to Euler angles (in radians)
		rot = glm::eulerAngles(orientation);

		// Optional: Convert the Euler angles to degrees
		rot = glm::degrees(rot);

		return true;  // Decomposition successful
	}

	void DecomposeMM()
	{

		// Decompose the matrix after manipulation
		//glm::vec3 newPosition, newRotation, newScale;
		//DecomposeTransform(m_modelMatrix, position, rotation, scale);

		// Update object's internal transformation data
		setLocalPosition(position);
		setLocalRotation(rotation);
		setLocalScale(scale);

		// Recompute model matrix to apply the new transformation
		m_isDirty = true;
		//computeModelMatrix();
	}

	void computeModelMatrix()
	{
		m_modelMatrix = getLocalModelMatrix();
		
		m_isDirty = false;
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
	}

	void setLocalRotation(const glm::vec3& newRotation)
	{
		rotation = newRotation;
		m_isDirty = true;
	}

	void setLocalScale(const glm::vec3& newScale)
	{
		scale = newScale;
		m_isDirty = true;
	}

	const glm::vec3& getGlobalPosition() const
	{
		return m_modelMatrix[3];
	}

	const glm::vec3& getLocalPosition() const
	{
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