#pragma once

class KeyManager
{
public:
	KeyManager();
	~KeyManager();

	void Initialize();
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	bool IsKeyDown(unsigned int);

	//private:
	bool m_keys[256] = { false, };
};

