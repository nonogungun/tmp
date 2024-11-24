#include "Object.h"


Object::Object(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context,
    const std::vector<MeshData>& meshes)
{
    Initialize(device, context, meshes);
}

Object::Object(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context,
    const std::string& basePath,
    const std::string& filename)
{
    Initialize(device, context, basePath, filename);
}

void Object::Initialize(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context,
    const std::string& basePath,
    const std::string& filename)
{
    auto meshes = MeshLoader::ReadFromFile(basePath, filename);

    Initialize(device, context, meshes);
}

void Object::Initialize(ComPtr<ID3D11Device>& device,
                        ComPtr<ID3D11DeviceContext>& context,
                        const std::vector<MeshData>& meshes)
{
    m_VertexCnstCPU.world = XMMatrixIdentity();
    Utility::CreateConstBuffer(device, m_VertexCnstCPU, m_VertexCnstGPU);

    Utility::CreateConstBuffer(device, m_materialConstsCPU, m_materialConstsGPU);

    for (const auto& meshData : meshes) {
        auto newMesh = std::make_shared<Mesh>();

        Utility::CreateVertexBuffer(device, meshData.vertices, newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.vertices.size());
        newMesh->stride = UINT(sizeof(VertexType));
        Utility::CreateIndexBuffer(device, meshData.indices,newMesh->indexBuffer);


        if (!meshData.albedoTextureFilename.empty()) {
            Utility::CreateTexture(device, context, meshData.albedoTextureFilename, 
                true, newMesh->albedoTexture, newMesh->albedoSRV);
        }
        else
        {
            Utility::CreateTexture(device, context, "Default.png",
                true, newMesh->albedoTexture, newMesh->albedoSRV);
        }

        m_meshes.push_back(newMesh);
    }
}

void Object::UpdateConstantBuffers(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context)
{
    if (m_isVisible) {
        Utility::UpdateBuffer(device, context, m_VertexCnstCPU,
            m_VertexCnstGPU);
        Utility::UpdateBuffer(device, context, m_materialConstsCPU,
            m_materialConstsGPU);
    }
}

void Object::ImguiUpdate()
{
    ImGui::Text(m_name.c_str());
    ImGui::Text("Translation");
    ImGui::DragFloat("X", &m_modelTranslation.x, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Y", &m_modelTranslation.y, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Z", &m_modelTranslation.z, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Text("\nRotation");
    ImGui::DragFloat("Axis X", &m_modelRotation.x, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Axis Y", &m_modelRotation.y, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Axis Z", &m_modelRotation.z, 0.005f, -20.f, 20.0f,
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
}


//vertex buffer, texture SRV, Constant 적용
void Object::Render(ComPtr<ID3D11DeviceContext>& context)
{
    if (m_isVisible) {
        for (const auto& mesh : m_meshes) {
            context->VSSetConstantBuffers(
                0, 1, m_VertexCnstGPU.GetAddressOf());

            context->PSSetConstantBuffers(
                0, 1, m_materialConstsGPU.GetAddressOf());

            //context->VSSetShaderResources(0, 1, mesh->heightSRV.GetAddressOf());
            
            // 물체 렌더링할 때 여러가지 텍스춰 사용 (t0 부터시작)
            //vector<ID3D11ShaderResourceView*> resViews = {
            //    mesh->albedoSRV.Get(), mesh->normalSRV.Get(), mesh->aoSRV.Get(),
            //    mesh->metallicRoughnessSRV.Get(), mesh->emissiveSRV.Get() };
            //context->PSSetShaderResources(0, UINT(resViews.size()), resViews.data());

            context->PSSetShaderResources(0, 1, mesh->albedoSRV.GetAddressOf());

            context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                &mesh->stride, &mesh->offset);

            context->IASetIndexBuffer(mesh->indexBuffer.Get(),
                DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed(mesh->indexCount, 0, 0);
        }
    }
}


