#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader, DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 offset)
{
	this->colorTint = colorTint;
	this->vShader = vShader;
	this->pShader = pShader;
	this->scale = scale;
	this->offset = offset;
}

// Getters
DirectX::XMFLOAT4 Material::GetColor() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pShader; }
DirectX::XMFLOAT2 Material::GetScale() { return scale; }
DirectX::XMFLOAT2 Material::GetOffset() { return offset; }

// Setters
void Material::SetColor(DirectX::XMFLOAT4 newColor) { colorTint = newColor; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader) { vShader = vertexShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader) { pShader = pixelShader; }
void Material::SetScale(DirectX::XMFLOAT2 scale) { this->scale = scale; }
void Material::SetOffset(DirectX::XMFLOAT2 offset) { this->offset = offset; }

/// <summary>
/// Adds a texture shader resource value to the srv map
/// </summary>
/// <param name="shaderVariableName">shader name</param>
/// <param name="srv">shader resource view</param>
void Material::AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ shaderVariableName, srv });
}

/// <summary>
/// Adds a sampler to the sampler map
/// </summary>
/// <param name="samplerName">sampler name</param>
/// <param name="sampler">sampler</param>
void Material::AddSampler(std::string samplerName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ samplerName, sampler });
}

/// <summary>
/// Sets the srvs and samplers before drawing
/// </summary>
void Material::PrepareMaterial()
{
	pShader->SetFloat2("scale", scale);
	pShader->SetFloat2("offset", offset);

	for (auto& t : textureSRVs) { pShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pShader->SetSamplerState(s.first.c_str(), s.second); }
}
