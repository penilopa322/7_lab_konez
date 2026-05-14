#ifndef MODEL_H
#define MODEL_H


#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm.hpp>
#include <matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;


class Model
{
public:
    
    vector<Mesh> meshes;
    string directory;
        
    Model(string const& path) 
    {
        loadModel(path);
    }
       
    void Draw()
    {
        for (auto& mesh : meshes)
            mesh.Draw();
    }

private:
    void loadModel(string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ASSIMP error: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }
    void processNode(aiNode* node, const aiScene* scene) {
        // обрабатываем все меши текущего узла
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // рекурсивно обрабатываем дочерние узлы
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        // Проход по вершинам
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // позиция
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            // нормаль (если есть)
            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else {
                vertex.Normal = glm::vec3(0.0f);
            }
            vertices.push_back(vertex);
        }

        // Проход по граням (индексы)
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices);
    }

};

#endif


