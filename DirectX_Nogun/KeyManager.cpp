#include "KeyManager.h"


KeyManager::KeyManager()
{
}

KeyManager::~KeyManager()
{
}

void KeyManager::Initialize()
{
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}
}

void KeyManager::KeyDown(unsigned int wParam)
{
	m_keys[wParam] = true;
}

void KeyManager::KeyUp(unsigned int wParam)
{
	m_keys[wParam] = false;
}

bool KeyManager::IsKeyDown(unsigned int wParam)
{
	//GetAsyncKeyState()�ε� ��������
	//���� �޼��� ���ν������� wParam�� �޾ƿ��� ����
	return m_keys[wParam];
}
