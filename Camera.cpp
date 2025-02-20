#include "Camera.h"
#include "DirectXMath.h"
#include "Transform.h"

using namespace DirectX;

// Constructor
Camera::Camera(float aspectRatio, XMFLOAT3 position, float fovAngle)
{
    this->fovAngle = fovAngle;
    transform = std::make_shared<Transform>();
    transform->SetPosition(position);

    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix() { return view; }

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix() { return projection; }

std::shared_ptr<Transform> Camera::GetTransform() { return transform; }

/// <summary>
/// Updates the view matrix with new parameters called every update
/// </summary>
void Camera::UpdateViewMatrix()
{
    XMFLOAT3 position = transform->GetPosition();
    XMFLOAT3 forward = transform->GetForward();
    XMFLOAT3 up = transform->GetUp();

    // Use XMMatrixLookToLH to create a new matrix
    XMMATRIX viewMatrix = XMMatrixLookToLH(
        XMLoadFloat3(&position), 
        XMLoadFloat3(&forward),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    // Store in the view matrix
    XMStoreFloat4x4(&view, viewMatrix);
}

/// <summary>
/// Updates the projection matrix on initialization and window resize
/// </summary>
/// <param name="aspectRatio">Windows aspect ratio</param>
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngle), aspectRatio, 0.01f, 1000.0f));
}

void Camera::Update(float dt)
{
    // Handle input
    if (Input::KeyDown('W')) { transform->MoveRelative(0.0f, 0.0f, 0.8f * dt); }
    if (Input::KeyDown('S')) { transform->MoveRelative(0.0f, 0.0f, -0.8f * dt); }

    if (Input::KeyDown('A')) { transform->MoveRelative(-0.8f * dt, 0.0f, 0.0f); }
    if (Input::KeyDown('D')) { transform->MoveRelative(0.8f * dt, 0.0f, 0.0f); }

    if (Input::KeyDown(' ')) { transform->MoveAbsolute(0.0f, 0.8f * dt, 0.0f); }
    if (Input::KeyDown(VK_CONTROL)) { transform->MoveAbsolute(0.0f, -0.8f * dt, 0.0f); }

    // Mouse input
    if (Input::MouseLeftDown())
    {
        int cursorMovementX = Input::GetMouseXDelta();
        int cursorMovementY = Input::GetMouseYDelta();
        
        transform->Rotate(cursorMovementY * dt, cursorMovementX * dt, 0.0f);

        // Clamp movement in the Y
        XMFLOAT3 rotation = transform->GetPitchYawRoll();
        if (rotation.x > XMConvertToRadians(45)) rotation.x = XMConvertToRadians(45);
        if (rotation.x < XMConvertToRadians(-45)) rotation.x = XMConvertToRadians(-45);
        transform->SetRotation(rotation);
    }

    // Update view matrix every frame
    UpdateViewMatrix();
}
