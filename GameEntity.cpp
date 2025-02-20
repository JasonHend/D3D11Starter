#include "GameEntity.h"
#include "Graphics.h"
#include "BufferStructs.h"
#include "Camera.h"
#include <DirectXMath.h>

/// <summary>
/// Constructs a new game entity to be rendered
/// </summary>
/// <param name="mesh">Game entity's mesh</param>
GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	//Initialize mesh and transform
	this->mesh = mesh;
	transform = std::make_shared<Transform>();
}

/// <summary>
/// Get shared pointer for a transform
/// </summary>
/// <returns>Game entity's transform</returns>
std::shared_ptr<Transform> GameEntity::GetTransform() { return transform; }

/// <summary>
/// Get shared pointer for a mesh
/// </summary>
/// <returns>Game entity's mesh</returns>
std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }

/// <summary>
/// Sets up necessary buffers and handles drawing mesh to the screen
/// </summary>
/// <param name="constantBuffer"></param>
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer, Camera currentCam)
{
	//Copy constant buffer code from Game::Draw()
	//Do not clear back buffer in this context
	//Define vertex information to pass into constant buffer
	BufferStructs constBuffStruct;
	constBuffStruct.colorTint = DirectX::XMFLOAT4(0.0f, 0.3f, 0.7f, 1.0f);
	constBuffStruct.m4World = transform->GetWorldMatrix();
	constBuffStruct.m4View = currentCam.GetViewMatrix();
	constBuffStruct.m4Projection = currentCam.GetProjectionMatrix();

	//Copy data to the constant buffer and un map for GPU use
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &constBuffStruct, sizeof(constBuffStruct));
	Graphics::Context->Unmap(constBuffer.Get(), 0);

	//Call draw for the mesh itself
	mesh->Draw();
}