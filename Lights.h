#pragma once
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#include "DirectXMath.h"

/// <summary>
/// Struct of any light passed into a pixel shader
/// </summary>
struct Light {
	int type;
	DirectX::XMFLOAT3 direction;
	float range;
	DirectX::XMFLOAT3 position;
	float intensity;
	DirectX::XMFLOAT3 color;
	float spotInnerAngle;
	float spotOuterAngle;
	DirectX::XMFLOAT2 padding;
};