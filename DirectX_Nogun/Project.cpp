#include "Project.h"

#define device m_Display->m_device 
#define context m_Display->m_context 


void Project::SetClass(shared_ptr<Camera> cam,
	shared_ptr<KeyManager> key, shared_ptr<Display> dis)
{
	m_camera = cam;
	m_input = key;
	m_Display = dis;
}

void Project::SetPipelineState(const GraphicsPSO& pso)
{
	context->VSSetShader(pso.m_vertexShader.Get(), 0, 0);
	context->PSSetShader(pso.m_pixelShader.Get(), 0, 0);
	context->HSSetShader(pso.m_hullShader.Get(), 0, 0);
	context->DSSetShader(pso.m_domainShader.Get(), 0, 0);
	context->GSSetShader(pso.m_geometryShader.Get(), 0, 0);
	context->IASetInputLayout(pso.m_inputLayout.Get());
	context->RSSetState(pso.m_rasterizerState.Get());
	context->OMSetBlendState(pso.m_blendState.Get(), pso.m_blendFactor,
		0xffffffff);
	context->OMSetDepthStencilState(pso.m_depthStencilState.Get(),
		pso.m_stencilRef);
	context->IASetPrimitiveTopology(pso.m_primitiveTopology);
}

void Project::Initialize(int width, int height, HWND hwnd)
{
	Graphics::InitAllStates(device);

	// 공통으로 쓰이는 ConstBuffers
	Utility::CreateConstBuffer(device, m_globalConstsCPU, m_globalConstsGPU);


	int m_lightType = 1; // 0,1,2 : directional, point, spot
	for (int k = 0; k < MAX_LIGHTS; k++)
	{
		// 다른 조명 끄기
		if (k != m_lightType)
			m_globalConstsCPU.lights[k].strength = XMFLOAT3(0.0f, 0.0f, 0.0f);
		else m_globalConstsCPU.lights[k].strength = XMFLOAT3(100.0f, 100.0f, 100.0f);
	}


	InitSkyBox(L"../Assets/SkyBox/",
		L"DGarden_specularIBL.dds", L"",
		L"DGarden_diffuseIBL.dds", L"");
	//brdf이미지랑, specular이미지는 아직 없음.


	//Init Object 
	//auto mesh = MeshLoader::ReadFromFile("E:/Zmodel/Helmet/", "DamagedHelmet.gltf");
	//m_object = make_shared<Object>(device, context, mesh);
	//m_object->m_name = "tmp";
	//m_ObjectList.push_back(m_object);

	{
		auto skybox = MeshLoader::MakeBox(50.0f);
		std::reverse(skybox.indices.begin(), skybox.indices.end());
		//skybox.albedoTextureFilename = "cat.png";
		m_skybox = make_shared<Object>(device, context, vector{ skybox });
		m_skybox->m_name = "skybox";
	}


}

void Project::Update()
{
	if (m_camera->m_useMove)
	{
		if (m_input->m_keys[87])
			m_camera->MoveForward(0.01);
		if (m_input->m_keys[83])
			m_camera->MoveForward(-0.01);
		if (m_input->m_keys[68])
			m_camera->MoveRight(0.01);
		if (m_input->m_keys[65])
			m_camera->MoveRight(-0.01);
		//if (m_input->m_keys[70]) m_drawWire = !m_drawWire; // f키 
	}

	const XMMATRIX view = m_camera->GetViewRow();
	const XMMATRIX proj = m_camera->GetProjRow();
	const XMFLOAT3 eyeWorld = m_camera->GetEyePos();

	UpdateGlobalConstants(eyeWorld, view, proj, view);

	//XMMatrixScaling();
	for (auto& i : m_ObjectList) {
		XMMATRIX result = XMMatrixIdentity();

		//rotation
		XMMATRIX rot = XMMatrixRotationY(i->m_modelRotation.y);
		result = XMMatrixMultiply(result, rot);
		rot = XMMatrixRotationX(i->m_modelRotation.x);
		result = XMMatrixMultiply(result, rot);
		rot = XMMatrixRotationZ(i->m_modelRotation.z);
		result = XMMatrixMultiply(result, rot);

		//translation
		XMMATRIX world = XMMatrixTranslation(i->m_modelTranslation.x,
			i->m_modelTranslation.y, i->m_modelTranslation.z);
		result = XMMatrixMultiply(result, world);
		result = XMMatrixTranspose(result);
		i->m_VertexCnstCPU.world = result;

		XMMATRIX worldIT = world;
		worldIT = XMMatrixTranslation(0, 0, 0); //이게 필요한가?
		worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, worldIT));
		i->m_VertexCnstCPU.worldIT = worldIT;

		i->UpdateConstantBuffers(device, context);
	}
}

void Project::UpdateGlobalConstants(const XMFLOAT3& eyeWorld,
	const XMMATRIX& viewRow, const XMMATRIX& projRow, const XMMATRIX& refl)//= XMMatrixIdentity())
{
	m_globalConstsCPU.eyeWorld = eyeWorld;
	m_globalConstsCPU.view = XMMatrixTranspose(viewRow);
	m_globalConstsCPU.proj = XMMatrixTranspose(projRow);
	m_globalConstsCPU.invProj = XMMatrixTranspose(XMMatrixInverse(nullptr, projRow));
	//for mirror 
	m_globalConstsCPU.viewProj = XMMatrixTranspose(XMMatrixMultiply(viewRow, projRow));

	//m_globalConstsCPU.strengthIBL = 0.0f;

	Utility::UpdateBuffer(device, context, m_globalConstsCPU, m_globalConstsGPU);
}


void Project::Render()
{
	context->RSSetViewports(1, &m_Display->m_viewport);

	//context->VSSetSamplers(0, 1, &m_samplerState); //height mapping에서 쓰임
	context->PSSetSamplers(0, UINT(Graphics::sampleStates.size()),
		Graphics::sampleStates.data());


	//"Common.hlsli"에서 register(t10)부터 시작
	vector<ID3D11ShaderResourceView*> commonSRVs = {
		m_environmentSRV.Get(), m_specularSRV.Get(), m_diffuseSRV.Get(), m_brdfSRV.Get() };
	context->PSSetShaderResources(10, UINT(commonSRVs.size()), commonSRVs.data());


	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(m_Display->m_backBufferRTV.Get(), clearColor);
	context->ClearDepthStencilView(m_Display->m_DepthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	context->OMSetRenderTargets(1, m_Display->m_backBufferRTV.GetAddressOf(),
		m_Display->m_DepthStencilView.Get());
	//context->OMSetDepthStencilState(m_Display->m_DepthStencilState.Get(), 0);


	SetPipelineState(Graphics::defaultSolidPSO);
	SetGlobalConsts(m_globalConstsGPU);
	for (auto& i : m_ObjectList) {
		i->Render(context);
	}

	SetPipelineState(Graphics::skyboxSolidPSO);
	m_skybox->Render(context);

}

void Project::UpdateImgui()
{
	if (ImGui::Button("Create Box"))
	{
		auto mesh = MeshLoader::MakeBox(1.0);
		mesh.albedoTextureFilename = "cat.png";
		shared_ptr<Object> obj = make_shared<Object>(device, context, vector{ mesh });
		obj->m_name = "Box";
		m_ObjectList.push_back(obj);
	}

	ImGui::Text("\n");

	if (ImGui::Button("Import"))
	{
		auto mesh = MeshLoader::ReadFromFile(m_basepath, m_filename);
		shared_ptr<Object> obj = make_shared<Object>(device, context, mesh);
		obj->m_name = "3d Model";
		m_ObjectList.push_back(obj);
	}
	ImGui::Text("%s", m_basepath.c_str());
	ImGui::Text("%s", m_filename.c_str());

}


void Project::HierarchyGUI()
{
	//List Box 
	ImGui::BeginListBox("Hierarchy");
	static int current_idx = -1;
	for (int i = 0; i < m_ObjectList.size(); i++)
	{
		if (ImGui::Selectable(m_ObjectList[i]->m_name.c_str(), true,
			ImGuiSelectableFlags_AllowDoubleClick))
		{
			current_idx = i;
		}
	}
	ImGui::EndListBox();

	// Object Property 
	ImGui::Text("ListBox Index : %i", current_idx);
	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode("Inspector"))
	{
		if (current_idx >= 0)
		{
			m_ObjectList[current_idx]->ImguiUpdate();
		}
		ImGui::TreePop();
	}

}

void Project::ProjectGUI(filesystem::path RootPath)
{
	for (const auto& file : filesystem::directory_iterator(RootPath)) {
		std::string filename = file.path().filename().string();
		if (file.is_directory())
		{
			// 디렉토리
			if (ImGui::TreeNode(filename.c_str()))
			{
				ProjectGUI(file.path().string());
				ImGui::TreePop();
			}
		}
		else
		{
			// 파일 표시 (아이콘은 추가할 수 있음)
			if (ImGui::Selectable(filename.c_str()))
			{
				// 파일 클릭 시 행동
				m_filename = filename;

				m_basepath = file.path().string();
				m_basepath.replace(m_basepath.find(filename),
					filename.length(), "");
			}
		}
	}

}


void Project::InitSkyBox(wstring basePath,
	wstring envFilename, wstring specularFilename,
	wstring diffuseFilename, wstring brdfFilename)
{
	Utility::CreateDDSTexture(device, (basePath + envFilename).c_str(), true, m_environmentSRV);
	Utility::CreateDDSTexture(device, (basePath + specularFilename).c_str(), true, m_specularSRV);
	Utility::CreateDDSTexture(device, (basePath + diffuseFilename).c_str(), true, m_diffuseSRV);
	//fasle = not cubemap 
	Utility::CreateDDSTexture(device, (basePath + brdfFilename).c_str(), false, m_brdfSRV);
}

void Project::SetGlobalConsts(ComPtr<ID3D11Buffer>& globalConstsGPU)
{
	// 쉐이더와 일관성 유지 register(b1)
	context->VSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
	context->PSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
	context->GSSetConstantBuffers(1, 1, globalConstsGPU.GetAddressOf());
}