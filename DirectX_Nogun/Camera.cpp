#include "Camera.h"

#include <iostream>

//void Camera::init(int width, int height)
//{
//    //m_width = width;
//    //m_height = height;
//}

XMFLOAT3 Camera::GetEyePos()
{
    return m_position;
}

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
}

//view 행렬 만드는 부분
XMMATRIX Camera::GetViewRow()
{
    // rotation값으로 회전 행렬을 만들기
    float pitch = XMConvertToRadians(m_pitch);
    float yaw = XMConvertToRadians(m_yaw);
    float roll = XMConvertToRadians(m_roll);
    XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    //기본 forward 벡터와 곱해서 회전된 벡터를 계산.
    XMVECTOR direction = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0); // +Z forward
    direction = XMVector3Transform(direction, rotationMat);

    XMVECTOR Eye = XMLoadFloat3(&m_position);
    XMVECTOR location_vec = XMLoadFloat3(&m_position);
    XMVECTOR At = direction + location_vec;
    XMVECTOR Up = XMLoadFloat3(&m_upDir);

    return XMMatrixLookAtLH(Eye, At, Up);
}

XMMATRIX Camera::GetProjRow()
{
    float screenAspect = (float)800 / (float)600;
    float fieldOfView = 3.141592654f / 4.0f;

    return XMMatrixPerspectiveFovLH(XMConvertToRadians(70), screenAspect, m_nearZ, m_farZ); //첫번째 인수 원근감 
    return XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, m_nearZ, m_farZ);
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
    if (!m_useMove) return;

    float sensitive = 30.0f;
    m_yaw = mouseNdcX * XM_2PI * sensitive;
    m_pitch = -mouseNdcY * XM_PIDIV2 * sensitive;

    float pitch = XMConvertToRadians(m_pitch);
    float yaw = XMConvertToRadians(m_yaw);
    float roll = XMConvertToRadians(m_roll);
    XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    XMVECTOR viewDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // +Z forward
    viewDir = XMVector3Transform(viewDir, rotationMat);

    //right Dir
    XMVECTOR upDir = XMLoadFloat3(&m_upDir);
    XMVECTOR rightDir = XMVector3Cross(upDir, viewDir);

    XMStoreFloat3(&m_viewDir, viewDir);
    XMStoreFloat3(&m_rightDir, rightDir);
}

XMVECTOR Camera::GetRightVector()
{
    float pitch = XMConvertToRadians(m_pitch);
    float yaw = XMConvertToRadians(m_yaw);
    float roll = XMConvertToRadians(m_roll);

    XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    XMVECTOR direction = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // +X right
    direction = XMVector3Transform(direction, rotationMat);
    return direction;
}

void Camera::MoveForward(float dt)
{
    //std::cout << tmp << std::endl;

    XMFLOAT3 viewdir;
    viewdir.x = m_viewDir.x * m_speed * dt;
    viewdir.y = m_viewDir.y * m_speed * dt;
    viewdir.z = m_viewDir.z * m_speed * dt;

    m_position.x += viewdir.x;
    m_position.y += viewdir.y;
    m_position.z += viewdir.z;
    //std::cout << m_position.x << ", " <<
    //    m_position.y << ", " << m_position.x << std::endl;
}

void Camera::MoveRight(float dt)
{
    XMFLOAT3 dir;
    dir.x = m_rightDir.x * m_speed * dt;
    dir.y = m_rightDir.y * m_speed * dt;
    dir.z = m_rightDir.z * m_speed * dt;

    m_position.x += dir.x;
    m_position.y += dir.y;
    m_position.z += dir.z;
}
