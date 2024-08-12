#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>  
#include <sstream>
#include <iostream>
#include <chrono>
#include "Model.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include <windows.h>

// 窗口大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 初始化摄像头
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间变量
float deltaTime = 0.006f;  // 当前帧与上一帧的时间差
float lastFrame = glfwGetTime();  // 上一帧的时间

// 摄像头控制开关
bool cameraControlEnabled = false;


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!cameraControlEnabled) {
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为 y 坐标是从底部往顶部增大的
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        cameraControlEnabled = !cameraControlEnabled;
        if (cameraControlEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // 重置鼠标初始位置
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window) {

    bool isSpacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime, isSpacePressed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime, isSpacePressed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime, isSpacePressed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime, isSpacePressed);
}

unsigned int loadShader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 着色器程序
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return ID;
}

unsigned int loadShader_geo(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
    // 1. 从文件路径中读取顶点/片段着色器代码
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // 保证 ifstream 对象可以抛出异常：
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // 打开文件
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // 读取文件的缓冲内容到数据流中
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // 关闭文件处理器
        vShaderFile.close();
        fShaderFile.close();
        // 转换数据流到string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // 如果有几何着色器路径，则也进行相同处理
        if (geometryPath != nullptr) {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();

    // 2. 编译着色器
    unsigned int vertex, fragment, geometry;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 如果有几何着色器路径，则编译几何着色器
    if (geometryPath != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        // 打印编译错误（如果有的话）
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // 着色器程序
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    // 打印链接错误（如果有的话）
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr) {
        glDeleteShader(geometry);
    }

    return ID;
}


// 初始化墨水分布纹理
GLuint initInkTexture(int width, int height) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::vector<float> inkData(width * height, 0.0f);
    for (int y = height / 4; y < 3 * height / 4; ++y) {
        for (int x = width / 4; x < 3 * width / 4; ++x) {
            inkData[y * width + x] = 1.0f; // 增加初始墨水浓度
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, inkData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

// 全局变量，用于存储四边形的VAO和VBO
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint quadEBO;

void createFullScreenQuad() {
    if (quadVAO == 0) {
        float vertices[] = {
            // Vertex Position   // Texture Coordinates
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  // Top Left
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // Bottom Left
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // Bottom Right
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f   // Top Right
        };
        unsigned int indices[] = {
            0, 1, 2,   // First Triangle
            0, 2, 3    // Second Triangle
        };


        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // 顶点位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // 纹理坐标属性
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}
void renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

int main() {
    // Print working dictionary
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring ws(buffer);
    std::string currentDir(ws.begin(), ws.end());

    std::string::size_type pos = currentDir.find_last_of("\\/");
    std::cout << "Current working directory: " << currentDir.substr(0, pos) << std::endl;



    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // set GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window object
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the current context
    glfwMakeContextCurrent(window);

    // Set the window size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Set the mouse position callback
    glfwSetCursorPosCallback(window, mouse_callback);
    // Set the mouse button callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Set the scroll callback
    glfwSetScrollCallback(window, scroll_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 编译和链接着色器程序
    unsigned int shaderProgram = loadShader("shader/vertshader.vs", "shader/fragshader.fs");
    // 获取uniform位置

    int edgeRangeLocation = glGetUniformLocation(shaderProgram, "edgeRange");
    int edgeThredLocation = glGetUniformLocation(shaderProgram, "edgeThred");
    int edgePowLocation = glGetUniformLocation(shaderProgram, "edgePow");
    int lightstrengthLocation = glGetUniformLocation(shaderProgram, "lightStrength");

    // 加载模型
    //Model ourModel("model/rock/cliff_rock1.fbx");
    //Model ourModel("model/Blue/blue.fbx");
    //Model ourModel("model/House/Chinese Brick Broken House.fbx");//房子
    //Model ourModel("model/SonowMountain/Snowy.obj");//雪山
    Model ourModel("model/Babydragon/Baby_dragon.fbx");//宝宝龙 
    //Model ourModel("model/ccity/city.fbx");
    //Model ourModel("model/shiba/1.FBX");//shiba dog
    //Model ourModel("model/reddragon/dragontrophy.fbx");//dragon head
    
    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // 初始化ImGui后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // 参数变量
    float edgeThreshold = 0.25f;
    float edgeRange = 3.5f;
    float edgeThred = 0.1f;
    float edgePow = 0.5f;
    // 参数变量
    bool mixValue = true;
    glm::vec3 lightPos(12.0f, 10.0f, 20.0f);
    float LightStrength = 0.5f;
    bool kEnabled = false;

    // 初始化墨水分布纹理
    GLuint inkTexture;
    glGenTextures(1, &inkTexture);
    glBindTexture(GL_TEXTURE_2D, inkTexture);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    

    // Initialize ink texture
    int width = 512, height = 512;
    GLuint diffinkTexture = initInkTexture(width, height);

    // Temporarily deprecate the particle system in this project for future expansion
    //unsigned int particleShaderProgram = loadShader("shader/particle.vert", "shader/particle.frag");
    unsigned int particleShaderProgram = loadShader_geo("shader/particle.vert", "shader/particle.frag", "shader/geometry.gs");

    ParticleSystem particleSystem(100000, particleShaderProgram);
    particleSystem.init();
    particleSystem.loadTexture("texture/Particlebrush.png");

    // Obtain vertex and normal data from the model and set up the particle system

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    ourModel.getVerticesAndNormals(positions, normals);
    particleSystem.setParticlesFromMesh(positions, normals);

    // Background quad
    // Compile and link the shader program
    unsigned int quatshaderProgram = loadShader("shader/quatvertshader.vs", "shader/quatfragshader.fs");
    // Set up the quad
    createFullScreenQuad();

    // Render to texture
    unsigned int screenShaderProgram = loadShader("shader/screenvertshader.vs", "shader/screenfragshader.fs");

    GLuint fbo, screentexture, depthRBO;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a texture attachment

    glGenTextures(1, &screentexture);
    glBindTexture(GL_TEXTURE_2D, screentexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screentexture, 0);

    // Create deep attachments
    glGenRenderbuffers(1, &depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);


    // Check if FBO is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    // Unbind texture and FBO
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    int frameCount = 0;
    double fps = 0;
    double previousTime = glfwGetTime();
    

    while (!glfwWindowShouldClose(window)) {

        //auto start = std::chrono::high_resolution_clock::now();
        // Calculate frame time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameCount++;
        if (currentFrame - previousTime >= 1.0) { // Update once per second
            fps = frameCount;
            frameCount = 0;
            previousTime = currentFrame;
        }

        if (deltaTime > 1.0f)
            deltaTime = 0.006f;

        processInput(window);

        // Bind FBO for rendering
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        

        // Set the projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        // Get the view matrix
        glm::mat4 view = camera.GetViewMatrix();
        // Set the model matrix
        glm::mat4 model = glm::mat4(1.0f);


        // Temporarily abandon the particle system waiting for future expansion in this project
        //particleSystem.update(deltaTime,projection,view,model);

        
        // start newImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window
        ImGui::Begin("Watercolor Parameters");
        ImGui::SliderFloat("Edge Range", &edgeRange, 1.0f, 10.0f);
        ImGui::SliderFloat("Edge Thred", &edgeThred, 0.0f, 1.0f);
        ImGui::SliderFloat("Edge Pow", &edgePow, 0.1f, 4.0f);
        ImGui::Checkbox("Enable brush texture", &mixValue);
        ImGui::Checkbox("Enable Curvature", &kEnabled);
        // Add sliders for light position and intensity
        ImGui::SliderFloat3("Light Position", &lightPos.x, -100.0f, 100.0f);
        ImGui::SliderFloat("Light Strength", &LightStrength, 0.0f, 1.0f);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::End();

        
        // Rendering instructions
        glClearColor(0.75f, 0.75f, 0.75f, 1.0f); // Set the background color to light gray
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        // Using shaders and rendering quadrilaterals
        glUseProgram(quatshaderProgram);
        glDepthMask(GL_FALSE); // Prohibit deep writing
        renderQuad();          // render paper background
        glDepthMask(GL_TRUE);  // Re enable deep writing
        

        //start = std::chrono::high_resolution_clock::now();

        // Activate shader
        glUseProgram(shaderProgram);

        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, diffinkTexture);

        // Set uniform variable
        glUniform1i(glGetUniformLocation(shaderProgram, "diffinkTexture"), 1);
        glUniform1f(glGetUniformLocation(shaderProgram, "diffusionRate"), 0.1f);
        glUniform2f(glGetUniformLocation(shaderProgram, "texelSize"), 1.0f / width, 1.0f / height);

      
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
        glUniform1i(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);
        glUniform1i(glGetUniformLocation(shaderProgram, "kEnabled"), kEnabled);

        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

       
        glUniform1f(edgeRangeLocation, edgeRange);  // 传入参数值
        glUniform1f(edgeThredLocation, edgeThred);  // 传入参数值
        glUniform1f(edgePowLocation, edgePow);  // 传入参数值
        glUniform1f(lightstrengthLocation, LightStrength);  // 传入参数值

        GLuint query;
        glGenQueries(1, &query);

       
        // 渲染模型
        ourModel.Draw(shaderProgram);
        
    


        // Unbind FBO and return to default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(screenShaderProgram);  // Use another shader to process textures
        glBindTexture(GL_TEXTURE_2D, screentexture);
        glUniform1i(glGetUniformLocation(screenShaderProgram, "textureSampler"), 0);

        // Get uniform location
        float currentTime = glfwGetTime();
        GLint timeLocation = glGetUniformLocation(screenShaderProgram, "time");
        glUniform1f(timeLocation, currentTime);

        renderQuad();  // Render a full screen quadrilateral to display textures


      

        glClear(GL_DEPTH_BUFFER_BIT);//Clear depth buffer

       

        // ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
        
    
    }

    // clear ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // clear GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    
    glfwTerminate();
    return 0;
}


