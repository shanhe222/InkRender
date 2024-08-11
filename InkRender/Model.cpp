#define STB_IMAGE_IMPLEMENTATION
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <iostream>
#include <cstring>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::vector<Texture> inktextures, std::vector<Texture> brushtextures)
    : vertices(vertices), indices(indices), textures(textures), inktextures(inktextures), brushtextures(brushtextures) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 顶点位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // 顶点纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(unsigned int shaderProgram) {
    unsigned int textureUnit = 0;  // Tracking the current texture unit index

    // Bind regular textures
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);  // Activate the appropriate texture unit
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
        glUniform1i(glGetUniformLocation(shaderProgram, (textures[i].type + std::to_string(i + 1)).c_str()), textureUnit);
        textureUnit++;  // Increment the texture unit index
    }

    // Bind ink textures
    for (unsigned int i = 0; i < inktextures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);  // Activate the appropriate texture unit
        glBindTexture(GL_TEXTURE_2D, inktextures[i].id);
        glUniform1i(glGetUniformLocation(shaderProgram, (inktextures[i].type + std::to_string(i + 1)).c_str()), textureUnit);
        textureUnit++;  // Increment the texture unit index
    }
    for (unsigned int i = 0; i < brushtextures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);  // Activate the appropriate texture unit
        glBindTexture(GL_TEXTURE_2D, brushtextures[i].id);
        glUniform1i(glGetUniformLocation(shaderProgram, (brushtextures[i].type + std::to_string(i + 1)).c_str()), textureUnit);
        textureUnit++;  // Increment the texture unit index
    }


    // Bind the vertex array and draw the elements
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset to default texture unit
    glActiveTexture(GL_TEXTURE0);
}
void Mesh::getVerticesAndNormals(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals) const {
    for (const auto& vertex : vertices) {
        positions.push_back(vertex.Position);
        normals.push_back(vertex.Normal);
    }
}


Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::Draw(unsigned int shaderProgram) {
    for (auto& mesh : meshes) {
        mesh.Draw(shaderProgram);
    }
}
void Model::getVerticesAndNormals(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals) {
    for (const auto& mesh : meshes) {
        mesh.getVerticesAndNormals(positions, normals);
    }
}


void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    // 检查文件扩展名
    std::string extension = path.substr(path.find_last_of(".") + 1);
    if (extension != "FBX" && extension != "fbx" && extension != "obj") {
        std::cout << "ERROR::ASSIMP::Unsupported file format: " << extension << std::endl;
        return;
    }

    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of("/\\"));

    std::cout << "Model directory: " << directory << std::endl;

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    std::cout << "Processing node: " << node->mName.C_Str() << std::endl;

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::cout << "Processing mesh: " << mesh->mName.C_Str() << std::endl;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<Texture> inktextures;
    std::vector<Texture> brushtextures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 自动加载纹理
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    // 如果没有加载到纹理，手动加载纹理
    if (textures.empty()) {
        std::cout << "No textures found, manually loading texture..." << std::endl;
        Texture texture;
        texture.id = TextureFromFile("Basecolor.PNG", directory);
        texture.type = "texture_diffuse";
        texture.path = "megalodon.png";
        textures.push_back(texture);
    }

    // 如果没有加载到其他纹理，手动加载水墨纹理
    if (inktextures.empty()) {
        std::cout << "No textures found, manually loading ink texture..." << std::endl;
        Texture texture;
        texture.id = TextureFromFile("inktexture.png", "texture");
        texture.type = "texture_ink";
        texture.path = "texture/inktexture.png";
        inktextures.push_back(texture);
    }

    // 如果没有加载到其他纹理，手动加载水墨纹理
    if (brushtextures.empty()) {
        std::cout << "No textures found, manually loading ink texture..." << std::endl;
        Texture texture;
        texture.id = TextureFromFile("Brushstrokes.png", "texture");
        texture.type = "texture_Brushstrokes";
        texture.path = "texture/Brushstrokes.png";
        brushtextures.push_back(texture);
    }

    return Mesh(vertices, indices, textures, inktextures, brushtextures);
}


std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::cout << "Loading texture: " << str.C_Str() << std::endl;

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string& directory) {
    std::string filename = std::string(path);
    filename = directory +'/' + filename;

    std::cout << "Texture path: " << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout  << std::endl;
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
