#pragma once

#include "Display.h"
#include "Camera.h"
#include "KeyManager.h"

#include "Object.h"
#include "MeshLoader.h"

// 유니티에서 이름을 따와서 Project라는 이름으로 만들어야겠다. 
class Tmp
{
private:
	//ComPtr<ID3D11Buffer> vertexConstantBuffer;
	////ComPtr<ID3D11Buffer> pixelConstantBuffer;
	//VertexConstant m_vertexcCnst; 
	//ComPtr<ID3D11Texture2D> m_texture;
	//ComPtr<ID3D11ShaderResourceView> m_textureResourceView;

	ComPtr<ID3D11VertexShader> m_vertexshader;
	ComPtr<ID3D11InputLayout> m_layout;
	ComPtr<ID3D11PixelShader> m_pixelshader;

	ID3D11SamplerState* m_samplerState;


	shared_ptr<Display> m_Display;
	shared_ptr<Camera> m_camera;
	shared_ptr<KeyManager> m_input;

	shared_ptr<Object> m_object; 


public:
	void SetClass(shared_ptr<Camera>, 
		shared_ptr<KeyManager>, 
		shared_ptr<Display>);

	void Initialize(int width, int height, HWND hwnd);
	void Update();
	void Render();

};

