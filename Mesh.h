#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

//Class that creates both index and vertex buffers for a mesh
//Mesh will be allowed to use both buffers created, meaning it will be able to draw the geometry using the buffers
class Mesh
{
public:
	// Constructor
	Mesh(Vertex* vertices, size_t numVertices, unsigned int* indices, size_t numIndices, const char* meshName);
	Mesh(const char* parameter);

	// Destructor
	~Mesh();

	// Functions to return vertex and index buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	// Functions to return the count of vertices and indices
	unsigned int GetVertexCount();
	unsigned int GetIndexCount();

	// Function to return the name of the mesh
	const char* GetMeshName();
	
	// Draw function to set the buffers and draw the geometry
	void Draw();

	// Takes vertices and calculates tangent data
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

private:
	//ComPtrs for vertex and index buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//Integers for vertices and indices
	unsigned int numVertices;
	unsigned int numIndices;

	//Name for the mesh
	const char* meshName;
};