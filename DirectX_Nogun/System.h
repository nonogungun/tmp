#pragma once

#include <Windows.h>
#include <wrl.h>
#include <memory>
#include <algorithm>

#include "Display.h"
#include "KeyManager.h"
#include "Camera.h"
#include "Project.h"

using Microsoft::WRL::ComPtr;
using std::shared_ptr;

class System
{
private:
	HWND m_hwnd;
	int m_Screenwidth, m_Screenheight;

	// Other Class 
	shared_ptr<Display> m_Display;
	shared_ptr<KeyManager> m_Input;
	shared_ptr<Camera> m_camera;


  	//project
	shared_ptr<Project> m_Project;

public:
	System();
	~System();

	void Initialize(int width, int height);
	void InitOtherClass();
	void Run();
	LRESULT CALLBACK MessageSender(HWND hwnd,
		UINT msg, WPARAM wParam, LPARAM lParam);

	//마우스 움직임
	void OnMouseMove(WPARAM key, int mouseX, int mouseY);

};