#pragma once
#include <DirectXMath.h>

//Defines a simple constant buffer with a color and offset
struct BufferStructs
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 offset;
};