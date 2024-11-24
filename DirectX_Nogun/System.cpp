
#include "System.h"

//전역 : System 객체 주소 
System* SystemInstance = NULL;

System::System()
{
	SystemInstance = this;
}
System::~System()
{
	SystemInstance = NULL;
}

int main()
{
	System sys;
	int width = 1280;
	int height = 720;
	sys.Initialize(width, height);
	sys.Run();

}

extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void System::Initialize(int width, int height)
{
	//Win API Init
	// WinClass Register > CreateWindow > ShowWindow
	WNDCLASS wc = { };
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	//wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); //검은 배경
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"NoGunEngine";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = GetModuleHandle(NULL);
	RegisterClass(&wc);

	m_Screenwidth = width;
	m_Screenheight = height;
	RECT wr = { 0, 0, m_Screenwidth, m_Screenheight };
	AdjustWindowRect(&wr, WS_OVERLAPPED, false);

	m_hwnd = CreateWindowEx(
		WS_EX_APPWINDOW, L"NoGunEngine", L"이딴게 엔진",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		500,        // 시작 위치.x
		300,        // 시작 위치.y
		wr.right,  //가로 방향 해상도
		wr.bottom, //세로 방향 해상도
		NULL, NULL, wc.hInstance, NULL);

	ShowWindow(m_hwnd, SW_SHOWDEFAULT);

	InitOtherClass();


	// 콘솔창이 렌더링 창을 덮는 것을 방지
	SetForegroundWindow(m_hwnd);
}

void System::InitOtherClass()
{
	m_Input = std::make_shared<KeyManager>();
	m_Input->Initialize();

	m_camera = std::make_shared<Camera>();

	m_Display = make_shared<Display>();
	m_Display->Initialize(m_Screenwidth, m_Screenheight, m_hwnd);


	//Project
	m_Project = std::make_shared<Project>();
	m_Project->SetClass(m_camera, m_Input, m_Display);
	m_Project->Initialize(m_Screenwidth, m_Screenheight, m_hwnd);

}

void System::Run()
{
	//message loop
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// 키보드 입력을 확인하고 문자를 전달한다.
			TranslateMessage(&msg);
			// 프로시저로 전달
			DispatchMessage(&msg);
		}
		else
		{
			//esc키 눌리면 종료 
			if (m_Input->IsKeyDown(VK_ESCAPE)) break;

			//게임 코드 실행
			//FPS();

			//Imgui 코드
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();


			//1번 창
			//ImGui::SetNextWindowPos(ImVec2(0, 0));
			//ImGui::SetNextWindowSize(ImVec2(250, 300));
			ImGui::Begin("Scene Control", NULL);
			ImGui::Text("  GUI  ");
			//추가적으로 사용할 GUI 
			m_Project->UpdateImgui();
			ImGui::End();


			//2번 창 : Hierarchy 
			ImGui::Begin("Hierarchy ", NULL);
			m_Project->HierarchyGUI();
			ImGui::End();


			//3번 창 : Project
			ImGui::Begin("Project");
			m_Project->ProjectGUI("..\\Assets");
			ImGui::End();


			ImTextureID texID = (ImTextureID)(m_Display->m_backBufferSRV.Get());
			if (ImGui::Begin("Test")) {
				ImGui::Image(texID, ImVec2(256, 256));  // 텍스처 크기 설정
			}
			ImGui::End();

			//Project Render 
			m_Project->Update();
			m_Project->Render();

			ImGui::Render(); //GPU 데이터 보냄 NewFrame을 닫을때 사용 
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			// Update and Render additional Platform Windows
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}


			m_Display->m_swapChain->Present(1, 0);
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return SystemInstance->MessageSender(hwnd, msg, wParam, lParam);
	}
}
LRESULT System::MessageSender(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == 'F') m_camera->m_useMove = !m_camera->m_useMove;

		m_Input->KeyDown((unsigned int)wParam);
		break;
	case WM_KEYUP:
		m_Input->KeyUp((unsigned int)wParam);
		break;
	case WM_MOUSEMOVE:
	{
		//SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
		//GetCursorPos(&pt);

		// wParam : 가상 키, lParam : 커서 좌표
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
	}
	break;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			const RECT* rect = (RECT*)lParam;
			SetWindowPos(hwnd, NULL, rect->left, rect->top,
				rect->right - rect->left, rect->bottom - rect->top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}


void System::OnMouseMove(WPARAM key, int mouseX, int mouseY)
{
	// 마우스 커서의 위치를 NDC로 변환
	// 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)

	// NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
	float x = mouseX * 2.0f / m_Screenwidth - 1.0f;
	float y = -mouseY * 2.0f / m_Screenheight + 1.0f;

	// 커서가 화면 밖으로 나갔을 경우 범위 조절
	x = std::clamp(x, -1.0f, 1.0f);
	y = std::clamp(y, -1.0f, 1.0f);

	// 카메라 시점 회전
	m_camera->UpdateMouse(x, y);

}