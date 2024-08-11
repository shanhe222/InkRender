#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<Texture> inktextures;
    std::vector<Texture> brushtextures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, 
        std::vector<Texture> inktextures, std::vector<Texture> brushtextures);
    void Draw(unsigned int shaderProgram);
    void getVerticesAndNormals(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals) const; // 新增方法


private:
    unsigned int VBO, EBO;
    void setupMesh();
};

class Model {
public:
    Model(const std::string& path);
    void Draw(unsigned int shaderProgram);
    void getVerticesAndNormals(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals); // 新增方法

  
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory);
};

#endif // MODEL_H
