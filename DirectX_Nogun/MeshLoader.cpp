#include "MeshLoader.h"

//#include <DirectXMesh.h>

vector<MeshData> MeshLoader::ReadFromFile(std::string basePath,
    std::string filename, bool revertNormals)
{
    MeshLoader Loader;
    Loader.Load(basePath, filename, revertNormals);
    vector<MeshData>& meshes = Loader.meshes;

    // Normalize vertices : 이거 안해주면 원본 크기로 복사된다. 
    XMFLOAT3 vmin(1000, 1000, 1000);
    XMFLOAT3 vmax(-1000, -1000, -1000);
    for (auto& mesh : meshes) {
        for (auto& v : mesh.vertices) {
            vmin.x = XMMin(vmin.x, v.position.x);
            vmin.y = XMMin(vmin.y, v.position.y);
            vmin.z = XMMin(vmin.z, v.position.z);
            vmax.x = XMMax(vmax.x, v.position.x);
            vmax.y = XMMax(vmax.y, v.position.y);
            vmax.z = XMMax(vmax.z, v.position.z);
        }
    }
    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float dl = XMMax(XMMax(dx, dy), dz);
    float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
        cz = (vmax.z + vmin.z) * 0.5f;

    for (auto& mesh : meshes) {
        for (auto& v : mesh.vertices) {
            v.position.x = (v.position.x - cx) / dl;
            v.position.y = (v.position.y - cy) / dl;
            v.position.z = (v.position.z - cz) / dl;
        }
    }

    return meshes;
}


std::string GetExtension(const std::string filename) {
    std::string ext(std::filesystem::path(filename).extension().string());
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

void MeshLoader::Load(std::string basePath, std::string filename, bool revertNormals)
{
    if (GetExtension(filename) == ".gltf") {
        m_isGLTF = true;
        m_revertNormals = revertNormals;
    }


    this->basePath = basePath;

    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(
        this->basePath + filename,
        aiProcess_ConvertToLeftHanded |
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_FixInfacingNormals |
        aiProcess_PreTransformVertices |
        aiProcess_ValidateDataStructure
    );

    if (!pScene) std::cout << "Failed to read File : " << basePath + filename << std::endl;

    XMMATRIX tr = XMMatrixIdentity();
    ProcessNode(pScene->mRootNode, pScene, tr);

    /*
    //Tangent 계산 
    for (auto& m : this->meshes) {

        vector<XMFLOAT3> positions(m.vertices.size());
        vector<XMFLOAT3> normals(m.vertices.size());
        vector<XMFLOAT2> texcoords(m.vertices.size());
        vector<XMFLOAT3> tangents(m.vertices.size());
        vector<XMFLOAT3> bitangents(m.vertices.size());

        for (size_t i = 0; i < m.vertices.size(); i++) {
            auto& v = m.vertices[i];
            positions[i] = v.position;
            normals[i] = v.normal;
            texcoords[i] = v.texture;
        }

        //<DirectXMesh.h>
        ComputeTangentFrame(m.indices.data(), m.indices.size() / 3,
            positions.data(), normals.data(), texcoords.data(),
            m.vertices.size(), tangents.data(),
            bitangents.data());

        for (size_t i = 0; i < m.vertices.size(); i++) {
            m.vertices[i].tangent = tangents[i];
        }
    }
    */
}

void MeshLoader::ProcessNode(aiNode* node, const aiScene* scene, XMMATRIX tr)
{
    //std::cout << node->mName.C_Str() << " : " << node->mNumMeshes << " "
    //         << node->mNumChildren << std::endl;

    XMFLOAT4X4 mf;
    ai_real* temp = &node->mTransformation.a1;
    float* mTemp = &mf._11;
    for (int t = 0; t < 16; t++) {
        mTemp[t] = float(temp[t]);
    }

    XMMATRIX m = XMLoadFloat4x4(&mf);
    m = XMMatrixMultiply(XMMatrixTranspose(m), tr);


    for (UINT i = 0; i < node->mNumMeshes; i++) {

        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProcessMesh(mesh, scene);

        for (auto& v : newMesh.vertices)
        {
            XMVECTOR pos = XMLoadFloat3(&v.position);
            pos = XMVector3Transform(pos, m);
            XMStoreFloat3(&v.position, pos);
        }

        meshes.push_back(newMesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        this->ProcessNode(node->mChildren[i], scene, m);
    }
}

MeshData MeshLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<VertexType> vertices;
    std::vector<uint32_t> indices;
    //Vertex 
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
        VertexType vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;



        vertex.normal.x = mesh->mNormals[i].x;
        if (m_isGLTF) {
            //gltf면 normal좌표계 변환 
            vertex.normal.y = mesh->mNormals[i].z;
            vertex.normal.z = -mesh->mNormals[i].y;
        }
        else {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        //Normal 뒤집어주기
        if (m_revertNormals) {
            vertex.normal.x *= -1.0f;
            vertex.normal.y *= -1.0f;
            vertex.normal.z *= -1.0f;
        }


        if (mesh->mTextureCoords[0]) {
            vertex.texture.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texture.y = (float)mesh->mTextureCoords[0][i].y;
        }
        vertices.push_back(vertex);
    }

    //Indice 
    for (UINT i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    MeshData newMesh;
    newMesh.vertices = vertices;
    newMesh.indices = indices;

    // Texture 읽기 
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        //PBR : aiTextureType_BASE_COLOR, Not PBR : aiTextureType_DIFFUSE
        aiTextureType type = material->GetTextureCount(aiTextureType_BASE_COLOR) ?
            aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;
        newMesh.albedoTextureFilename = ReadTexture(material, type);

        newMesh.emissiveTextureFilename =
            ReadTexture(material, aiTextureType_EMISSIVE);
        newMesh.heightTextureFilename =
            ReadTexture(material, aiTextureType_HEIGHT);
        newMesh.normalTextureFilename =
            ReadTexture(material, aiTextureType_NORMALS);
        newMesh.metallicTextureFilename =
            ReadTexture(material, aiTextureType_METALNESS);
        newMesh.roughnessTextureFilename =
            ReadTexture(material, aiTextureType_DIFFUSE_ROUGHNESS);
        newMesh.aoTextureFilename =
            ReadTexture(material, aiTextureType_AMBIENT_OCCLUSION);
    }
    return newMesh;

}

std::string MeshLoader::ReadTexture(aiMaterial* material, aiTextureType type)
{
    using namespace std;

    if (material->GetTextureCount(type) > 0)
    {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);


        std::cout << filesystem::path(filepath.C_Str()).filename().string()
            << std::endl;


        string fullPath = this->basePath
            + filesystem::path(filepath.C_Str()).filename().string();
        std::cout << fullPath << std::endl;

        return fullPath;
    }
    else
    {
        return "";
    }

}

MeshData MeshLoader::MakeBox(const float scale)
{
    vector<XMFLOAT3> positions;
    vector<XMFLOAT3> colors;
    vector<XMFLOAT3> normals;
    vector<XMFLOAT2> texcoords; // 텍스춰 좌표

    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, -1.0f * scale));

    colors.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));

    // 아랫면
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, 1.0f * scale));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, -1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, -1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, -1.0f, 0.0f));
    normals.push_back(XMFLOAT3(0.0f, -1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));

    // 앞면
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale));
    colors.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, -1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, -1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, -1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, -1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));

    // 뒷면
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, 1.0f * scale));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 1.0f));
    colors.push_back(XMFLOAT3(0.0f, 1.0f, 1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    normals.push_back(XMFLOAT3(0.0f, 0.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));

    // 왼쪽
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, 1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(-1.0f * scale, -1.0f * scale, -1.0f * scale));
    colors.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));
    colors.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));
    normals.push_back(XMFLOAT3(-1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(-1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(-1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(-1.0f, 0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));

    // 오른쪽
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, 1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, -1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, -1.0f * scale));
    positions.push_back(XMFLOAT3(1.0f * scale, 1.0f * scale, 1.0f * scale));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 1.0f));
    colors.push_back(XMFLOAT3(1.0f, 0.0f, 1.0f));
    normals.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    normals.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 0.0f));
    texcoords.push_back(XMFLOAT2(1.0f, 1.0f));
    texcoords.push_back(XMFLOAT2(0.0f, 1.0f));


    MeshData meshData;
    for (size_t i = 0; i < positions.size(); i++) {
        VertexType v;
        v.position = positions[i];
        v.normal = normals[i];
        v.texture = texcoords[i];
        meshData.vertices.push_back(v);
    }

    meshData.indices = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };


    return meshData;
}


MeshData MeshLoader::MakeSquare(const float scale)
{
    vector<VertexType> vertices(4);
    vector<unsigned int> indices(6);

    vertices[0].position = XMFLOAT3(-1.0f * scale, -1.0f * scale, 0.0f * scale);
    vertices[1].position = XMFLOAT3(-1.0f * scale, 1.0f * scale, 0.0f * scale);
    vertices[2].position = XMFLOAT3(1.0f * scale, 1.0f * scale, 0.0f * scale);
    vertices[3].position = XMFLOAT3(1.0f * scale, -1.0f * scale, 0.0f * scale);

    vertices[0].texture = XMFLOAT2(0.0f, 1.0f);
    vertices[1].texture = XMFLOAT2(0.0f, 0.0f);
    vertices[2].texture = XMFLOAT2(1.0f, 0.0f);
    vertices[3].texture = XMFLOAT2(1.0f, 1.0f);

    // 수직 방향 
    vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);


    //수평 방향 -> Normal Mapping에서 쓰인다. 
    vertices[0].tangent = XMFLOAT3(0.0f, 0.1f, 0.0f);
    vertices[1].tangent = XMFLOAT3(0.0f, 0.1f, 0.0f);
    vertices[2].tangent = XMFLOAT3(0.0f, 0.1f, 0.0f);
    vertices[3].tangent = XMFLOAT3(0.0f, 0.1f, 0.0f);


    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

    MeshData model;
    model.vertices = vertices;
    model.indices = indices;

    return model;
}

