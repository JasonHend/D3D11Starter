#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "GameEntity.h"
#include <vector>
#include <DirectXMath.h>
#include "Camera.h"

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
	void LoadShaders();
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

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Constant buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Camera associated variables
	std::shared_ptr<Camera> currentCamera;
	std::vector<std::shared_ptr<Camera>> cameras;
};

