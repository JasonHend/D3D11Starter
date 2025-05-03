#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "GameEntity.h"
#include <vector>
#include <DirectXMath.h>
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();

	// ImGui usage
	void UpdateUIContext(float deltaTime);
	void CustomizeUIContext();
	bool showDemo;
	bool vsync;
	float* backgroundColor;
	float* colorTint;
	float* offset;
	
	// Smart pointers for meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	// Smart pointer for game entities
	std::vector<std::shared_ptr<GameEntity>> entities;

	// Smart pointer for materials
	std::vector<std::shared_ptr<Material>> materials;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Camera associated variables
	std::shared_ptr<Camera> currentCamera;
	std::vector<std::shared_ptr<Camera>> cameras;

	// Lighting variables
	DirectX::XMFLOAT3 ambientLight;
	Light directionalLight;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light spotLight;
	std::vector<Light> lights;

	// Skybox
	std::shared_ptr<Sky> skybox;

	// Data for shadow mapping
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	// Data for post processing
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV;

	// Blur
	std::shared_ptr<SimplePixelShader> blurPS;
	std::shared_ptr<int> blurRadius;
};

