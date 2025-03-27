#pragma once
#include "SimpleShader.h"
#include "Camera.h"
#include <memory>

class Material
{
public:
	// Constructor
	Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader, DirectX::XMFLOAT2 scale, DirectX::XMFLOAT2 offset, float roughness);

	// Getters
	DirectX::XMFLOAT4 GetColor();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetScale();
	DirectX::XMFLOAT2 GetOffset();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetSRVs();

	// Setters
	void SetColor(DirectX::XMFLOAT4 newColor);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetScale(DirectX::XMFLOAT2 scale);
	void SetOffset(DirectX::XMFLOAT2 offset);

	// Add textures and sampler functions
	void AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string samplerName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	// Prepare material for drawing
	void PrepareMaterial(Camera currentCam);

private:
	// Color along with both pixel and vertex shaders
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vShader;
	std::shared_ptr<SimplePixelShader> pShader;

	// Textures and samplers
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

	// Scale and Offset
	DirectX::XMFLOAT2 scale;
	DirectX::XMFLOAT2 offset;

	// Roughness
	float roughness;
};

