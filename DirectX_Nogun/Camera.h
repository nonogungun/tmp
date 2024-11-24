#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
    bool m_useMove = false;

    // ���� ���콺 ��ġ�� �ٶ󺸴� ������ ȸ����Ŵ 
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void SetAspectRatio(float aspect);

    XMMATRIX GetViewRow();
    XMMATRIX GetProjRow();
    XMFLOAT3 GetEyePos();

    XMVECTOR GetRightVector(); // ��� �Ǵ� �Լ�

private:

    XMFLOAT3 m_position = XMFLOAT3(0.0f, 0.0f, -5.0f);
    XMFLOAT3 m_viewDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 m_upDir = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 m_rightDir = XMFLOAT3(1.0f, 0.0f, 0.0f);

    float m_pitch = 0.0f;
    float m_yaw = 0.0f;
    float m_roll = 0.0f;

    float m_speed = 5.0f; // �����̴� �ӵ�

    // �������� �ɼǵ� ī�޶� Ŭ������ �̵�
    float m_projFovAngleY = 70.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;
    bool m_usePerspectiveProjection = true;
};


