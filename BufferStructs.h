#pragma once
#include <DirectXMath.h>

// Defines a simple constant buffer
struct BufferStructs
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 m4World;
	DirectX::XMFLOAT4X4 m4View;
	DirectX::XMFLOAT4X4 m4Projection;
};