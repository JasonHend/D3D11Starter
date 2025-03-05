#include "GameEntity.h"
#include "Graphics.h"
#include "Camera.h"
#include "Material.h"
#include <DirectXMath.h>

/// <summary>
/// Constructs a new game entity to be rendered
/// </summary>
/// <param name="mesh">Game entity's mesh</param>
GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	//Initialize mesh and transform
	this->mesh = mesh;
	transform = std::make_shared<Transform>();

	this->material = material;
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
/// Get shared pointer for a material
/// </summary>
/// <returns>Game entity's material</returns>
std::shared_ptr<Material> GameEntity::GetMaterial() { return material; }

/// <summary>
/// Sets the current meshes material
/// </summary>
/// <param name="material">new material</param>
void GameEntity::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }

/// <summary>
/// Sets up necessary buffers and handles drawing mesh to the screen
/// </summary>
/// <param name="constantBuffer"></param>
void GameEntity::Draw(Camera currentCam)
{
	// Set vertex and pixel shaders
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Map to the GPU
	std::shared_ptr<SimpleVertexShader> vShader = material->GetVertexShader();

	vShader->SetMatrix4x4("m4World", GetTransform()->GetWorldMatrix());
	vShader->SetMatrix4x4("m4View", currentCam.GetViewMatrix());
	vShader->SetMatrix4x4("m4Projection", currentCam.GetProjectionMatrix());

	vShader->CopyAllBufferData();

	// Set pixel shader information
	std::shared_ptr<SimplePixelShader> pShader = material->GetPixelShader();

	pShader->SetFloat4("colorTint", material->GetColor());

	pShader->CopyAllBufferData();
	// Call draw for the mesh itself
	mesh->Draw();
}