#pragma once
#include "SimpleShader.h"
#include <memory>

class Material
{
public:
	// Constructor
	Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader);

	// Getters
	DirectX::XMFLOAT4 GetColor();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColor(DirectX::XMFLOAT4 newColor);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);

private:
	// Color along with both pixel and vertex shaders
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vShader;
	std::shared_ptr<SimplePixelShader> pShader;
};

