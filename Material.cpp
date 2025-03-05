#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader)
{
	this->colorTint = colorTint;
	this->vShader = vShader;
	this->pShader = pShader;
}

// Getters
DirectX::XMFLOAT4 Material::GetColor() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pShader; }

// Setters
void Material::SetColor(DirectX::XMFLOAT4 newColor) { colorTint = newColor; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader) { vShader = vertexShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader) { pShader = pixelShader; }
