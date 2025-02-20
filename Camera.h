#pragma once
#include "Transform.h"
#include "Input.h"
#include "DirectXMath.h"
#include <memory>

// Class to hold view projection and a transform
class Camera
{
public:
	// Constructor
	Camera(float aspectRatio, DirectX::XMFLOAT3 position, float fovAngle);

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	std::shared_ptr<Transform> GetTransform();
	void UpdateProjectionMatrix(float aspectRatio);
	void Update(float dt);

private:
	// Update matrices
	void UpdateViewMatrix();

	float fovAngle;
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

