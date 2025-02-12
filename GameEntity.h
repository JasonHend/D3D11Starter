#pragma once
#include <memory>
#include "Transform.h"
#include "Mesh.h"

//Overall class of what is rendered
class GameEntity
{
public:
	//Constructor
	GameEntity(std::shared_ptr<Mesh> mesh);

	//Getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	//Draw
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};