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
	//GetAsyncKeyState()로도 구현가능
	//나는 메세지 프로시저에서 wParam을 받아오는 구조
	return m_keys[wParam];
}
