#pragma once

#include "struct.h"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

class MeshLoader
{
private:
    std::string basePath; //fbx, gltf 파일 경로
    std::vector<MeshData> meshes;

    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;

    void Load(std::string basePath, std::string filename, bool revertNormals);
    void ProcessNode(aiNode* node, const aiScene* scene, XMMATRIX tr);
    MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::string ReadTexture(aiMaterial* material, aiTextureType type);

public:
    static vector<MeshData> ReadFromFile(std::string basePath,
        std::string filename, bool revertNormals = false);

	static MeshData MakeBox(const float scale);

	static MeshData MakeSquare(const float scale);

};

