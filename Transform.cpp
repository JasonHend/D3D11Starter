#include "Transform.h"
#include "DirectXMath.h"

using namespace DirectX;

/// <summary>
/// Creates new transform class
/// </summary>
Transform::Transform() :
	dirtyMatrices(false),
	dirtyVectors(false),
	position(0.0f, 0.0f, 0.0f),
	pitchYawRoll(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	up(0.0f, 1.0f, 0.0f),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f)
{
	// Initialize world matrix as identity
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
	dirtyMatrices = true;
}

/// <summary>
/// Overwrites raw position using DirectX float
/// </summary>
/// <param name="position">New position</param>
void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
	dirtyMatrices = true;
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
	dirtyMatrices = true;
	dirtyVectors = true;
}

/// <summary>
/// Overwrites raw rotation using DirectX float
/// </summary>
/// <param name="rotation">New rotation</param>
void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->pitchYawRoll = rotation;
	dirtyMatrices = true;
	dirtyVectors = true;
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
	dirtyMatrices = true;
}

/// <summary>
/// Overwrites raw scale using DirectX float
/// </summary>
/// <param name="scale">New scale</param>
void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
	dirtyMatrices = true;
}

// Getters
DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

// Each matrix get should clean the matrix first
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() 
{ 
	CleanMatrices();
	return m4World; 
}
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() 
{ 
	CleanMatrices();
	return m4WorldInverseTranspose; 
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	CleanVectors();
	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	CleanVectors();
	return up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	CleanVectors();
	return forward;
}

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
	dirtyMatrices = true;
}

/// <summary>
/// Adjust exsisting position using XMFLOAT3
/// </summary>
/// <param name="offset">Added positions</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	// Cant use compound operators on XMFLOAT3, use overload
	MoveAbsolute(offset.x, offset.y, offset.z);
}

/// <summary>
/// Moves entity based on current rotation parameters
/// </summary>
/// <param name="x">x offset</param>
/// <param name="y">y offset</param>
/// <param name="z">z offset</param>
void Transform::MoveRelative(float x, float y, float z)
{
	// Place variables into a vector
	XMFLOAT3 moveOffset(x, y, z);
	XMVECTOR vectorOffset = XMLoadFloat3(&moveOffset);

	// Get current rotation into a quaternion
	XMVECTOR quaternionRotation(XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));

	// Rotate and store in final offset
	XMVECTOR finalOffset = XMVector3Rotate(vectorOffset, quaternionRotation);

	// Load into rotation
	XMStoreFloat3(&position, XMLoadFloat3(&position) + finalOffset);

	// Update matrices
	dirtyMatrices = true;
}

/// <summary>
/// Moves entity based on curent rotation parameters
/// </summary>
/// <param name="offset">Offset vector</param>
void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	// Call overload
	MoveRelative(offset.x, offset.y, offset.z);
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
	dirtyMatrices = true;
	dirtyVectors = true;
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
	dirtyMatrices = true;
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

/// <summary>
/// Cleans up both matrices whenever data is changed
/// </summary>
void Transform::CleanMatrices()
{
	// Check if necessary to update matrix
	if (!dirtyMatrices)
		return;

	// Matrix for translation, rotation, and scale
	XMMATRIX m4Translation = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX m4Rotation = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
	XMMATRIX m4Scale = XMMatrixScaling(scale.x, scale.y, scale.z);

	// Combine into world matrix
	XMMATRIX world = m4Scale * m4Rotation * m4Translation;

	// Store the matrices
	XMStoreFloat4x4(&m4World, world);
	XMStoreFloat4x4(&m4WorldInverseTranspose,
		XMMatrixInverse(0, XMMatrixTranspose(world)));

	// Matrices are now clean
	dirtyMatrices = false;
}

void Transform::CleanVectors()
{
	// Check if vectors should be updated
	if (!dirtyVectors)
		return;

	// Current rotation for quaternion
	XMVECTOR quaternionRotation(XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));

	// Rotate all 3 vectors
	XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), quaternionRotation));
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), quaternionRotation));
	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), quaternionRotation));

	dirtyVectors = false;
}
