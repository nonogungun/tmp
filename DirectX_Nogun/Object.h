#pragma once

#include "MeshLoader.h"

#include "Utility.h"
#include "Display.h"


using namespace DirectX;

class Object
{
private:
	bool m_isVisible = true;
	
	std::vector<shared_ptr<Mesh>> m_meshes;

	//component
	// Animator, Physic 
public:
	std::string m_name;
	
	XMFLOAT3 m_modelTranslation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_modelRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_modelScaling =     XMFLOAT3(1.0f, 1.0f,1.0f);


	VertexConstant m_VertexCnstCPU;
	ComPtr<ID3D11Buffer> m_VertexCnstGPU;

	PixelConstant m_materialConstsCPU;
	ComPtr<ID3D11Buffer> m_materialConstsGPU;


	Object(){}
	~Object(){}
	Object(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const std::vector<MeshData>& meshes);
	void Initialize(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context,
		const std::vector<MeshData>& meshes);

	// For assimp 
	Object(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const std::string& basePath, const std::string& filename);
	void Initialize(ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& context, 
		const std::string& basePath,
		const std::string& filename);

	void Render(ComPtr<ID3D11DeviceContext>& context);

	void UpdateConstantBuffers(ComPtr<ID3D11Device>& device, 
		ComPtr<ID3D11DeviceContext>& context);

	//void UpdateWorldRow(const XMMATRIX& worldRow);



	virtual void ImguiUpdate(); 

};

