#include "Mesh.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"

//Constructor to create both the vertex and index buffer
Mesh::Mesh(Vertex* vertices, size_t numVertices, unsigned int* indices, size_t numIndices)
{
	//Took inspiration from the demo code for passing in the number of vertices and indices
	//Thought to use sizeof(arr)/sizeof(arr[0]) but would not be correct as it would include size of a pointer and not the full array
	//Initialization of both integer counts
	this->numVertices = (unsigned int)numVertices;
	this->numIndices = (unsigned int)numIndices;

	//Creation of vertex buffer
	{
		//Vertex buffer description
		D3D11_BUFFER_DESC vertexDescription = {};
		vertexDescription.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDescription.ByteWidth = sizeof(Vertex) * (unsigned int)numVertices;
		vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDescription.CPUAccessFlags = 0;
		vertexDescription.MiscFlags = 0;
		vertexDescription.StructureByteStride = 0;

		//Structure to hold the vertex data
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices;

		//Create the vertex buffer
		Graphics::Device->CreateBuffer(&vertexDescription, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	//Creation of index buffer
	{
		//Index buffer description
		D3D11_BUFFER_DESC indexDescription = {};
		indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
		indexDescription.ByteWidth = sizeof(unsigned int) * (unsigned int)numIndices;
		indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDescription.CPUAccessFlags = 0;
		indexDescription.MiscFlags = 0;
		indexDescription.StructureByteStride = 0;

		//Structure to hold initial index data
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices;

		//Create the index buffer
		Graphics::Device->CreateBuffer(&indexDescription, &initialIndexData, indexBuffer.GetAddressOf());
	}
}

//Deconstructor
Mesh::~Mesh()
{
	//Can be empty, as everything is handled through smart pointers
}

//Public getters
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() { return vertexBuffer; }
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() { return indexBuffer; }
unsigned int Mesh::GetVertexCount() { return numVertices; }
unsigned int Mesh::GetIndexCount() { return numIndices; }

//Functions
//Draws the current mesh
void Mesh::Draw()
{
	//Set the buffers in the input assembler stage
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Tell the graphics API to draw the mesh (Direct3D)
	Graphics::Context->DrawIndexed(numIndices, 0, 0);
}