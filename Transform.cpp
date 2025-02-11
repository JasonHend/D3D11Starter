#include "Transform.h"

using namespace DirectX;

/// <summary>
/// Creates new transform class
/// </summary>
Transform::Transform() :
	dirty(false),
	position(0.0f, 0.0f, 0.0f),
	pitchYawRoll(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f)
{
	//Initialize world matrix as identity
	XMStoreFloat4x4(&m4World, XMMatrixIdentity());
	XMStoreFloat4x4(&m4WorldInverseTranspose, XMMatrixIdentity());
}

/// <summary>
/// Overwrites raw position using float values
/// </summary>
/// <param name="x">New x</param>
/// <param name="y">New y</param>
/// <param name="z">New z</param>
void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	dirty = true;
}

/// <summary>
/// Overwrites raw position using DirectX float
/// </summary>
/// <param name="position">New position</param>
void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
	dirty = true;
}

/// <summary>
/// Overwrites raw rotation using float values
/// </summary>
/// <param name="pitch">Pitch rotation</param>
/// <param name="yaw">Yaw rotation</param>
/// <param name="roll">Roll rotation</param>
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	pitchYawRoll.x = pitch;
	pitchYawRoll.y = yaw;
	pitchYawRoll.z = roll;
	dirty = true;
}

/// <summary>
/// Overwrites raw rotation using DirectX float
/// </summary>
/// <param name="rotation">New rotation</param>
void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->pitchYawRoll = rotation;
	dirty = true;
}

/// <summary>
/// Overwrites raw scale using float values
/// </summary>
/// <param name="x">New x scale</param>
/// <param name="y">New y scale</param>
/// <param name="z">New z scale</param>
void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	dirty = true;
}

/// <summary>
/// Overwrites raw scale using DirectX float
/// </summary>
/// <param name="scale">New scale</param>
void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
	dirty = true;
}

//Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() { return m4World; }
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() { return m4WorldInverseTranspose; }

/// <summary>
/// Adjusts exsisting position using floats
/// </summary>
/// <param name="x">Added x position</param>
/// <param name="y">Added y position</param>
/// <param name="z">Added z position</param>
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	dirty = true;
}

/// <summary>
/// Adjust exsisting position using XMFLOAT3
/// </summary>
/// <param name="offset">Added positions</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	//Cant use compound operators on XMFLOAT3, use overload
	MoveAbsolute(offset.x, offset.y, offset.z);
}

/// <summary>
/// Adjusts exsisting rotation using float values
/// </summary>
/// <param name="pitch">Added pitch</param>
/// <param name="yaw">Added yaw</param>
/// <param name="roll">Added roll</param>
void Transform::Rotate(float pitch, float yaw, float roll)
{
	pitchYawRoll.x += pitch;
	pitchYawRoll.y += yaw;
	pitchYawRoll.z += roll;
	dirty = true;
}

/// <summary>
/// Adjusts exsisting rotation using XMFLOAT3 value
/// </summary>
/// <param name="rotation">Added rotation</param>
void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	//Cant use compound operators on XMFLOAT3, use overload
	Rotate(rotation.x, rotation.y, rotation.z);
}

/// <summary>
/// Adjusts exsiting scale using float values
/// </summary>
/// <param name="x">Added x scale</param>
/// <param name="y">Added y scale</param>
/// <param name="z">Added z scale</param>
void Transform::Scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
	dirty = true;
}

/// <summary>
/// Adjusts exsisting scale using XMFLOAT3
/// </summary>
/// <param name="scale">Added scale</param>
void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	//Cant use compound operators on XMFLOAT3, use overload
	Scale(scale.x, scale.y, scale.z);
}
