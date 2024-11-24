#pragma once

#include "Display.h"
#include "Camera.h"
#include "KeyManager.h"

#include "MeshLoader.h"

#include "Object.h"

#include "GraphicsCommon.h"

class Project
{
private:
	// 3D models 
	vector<shared_ptr<Object>> m_ObjectList; 

	shared_ptr<Object> m_skybox;

private:
	shared_ptr<Display> m_Display;
	shared_ptr<Camera> m_camera;
	shared_ptr<KeyManager> m_input;


	// 공통 texture 
	void InitSkyBox(wstring basePath, wstring envFilename,
		wstring specularFilename, wstring diffuseFilename,
		wstring brdfFilename);
	ComPtr<ID3D11ShaderResourceView> m_environmentSRV;
	ComPtr<ID3D11ShaderResourceView> m_diffuseSRV; // = irradiance
	ComPtr<ID3D11ShaderResourceView> m_specularSRV;
	ComPtr<ID3D11ShaderResourceView> m_brdfSRV;


	//공용 constant
	GlobalConstants m_globalConstsCPU;
	ComPtr<ID3D11Buffer> m_globalConstsGPU;

	void UpdateGlobalConstants(const XMFLOAT3& eyeWorld, const XMMATRIX& viewRow,
		const XMMATRIX& projRow, const XMMATRIX& refl);
	void SetGlobalConsts(ComPtr<ID3D11Buffer>& globalConstsGPU);

public:
	void SetClass(
		shared_ptr<Camera>,
		shared_ptr<KeyManager>,
		shared_ptr<Display>);

	void Initialize(int width, int height, HWND hwnd);
	void Update();
	void Render();

	//Imgui 
	void UpdateImgui();
	void HierarchyGUI();
	void ProjectGUI(filesystem::path RootPath);
	string m_filename = "";
	string m_basepath = "";

	//임시 pso
	void SetPipelineState(const GraphicsPSO& pso);
};

