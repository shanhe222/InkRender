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
#include "Model.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include <windows.h>

// ���ڴ�С
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ��ʼ������ͷ
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ʱ�����
float deltaTime = 0.006f;  // ��ǰ֡����һ֡��ʱ���
float lastFrame = glfwGetTime();  // ��һ֡��ʱ��

// ����ͷ���ƿ���
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
    float yoffset = lastY - ypos; // ע���������෴�ģ���Ϊ y �����Ǵӵײ������������
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        cameraControlEnabled = !cameraControlEnabled;
        if (cameraControlEnabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // ��������ʼλ��
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

    // ������ɫ��
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Ƭ����ɫ��
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ��ɫ������
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
    // 1. ���ļ�·���ж�ȡ����/Ƭ����ɫ������
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ��֤ ifstream ��������׳��쳣��
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // ���ļ�
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // ��ȡ�ļ��Ļ������ݵ���������
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // �ر��ļ�������
        vShaderFile.close();
        fShaderFile.close();
        // ת����������string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // ����м�����ɫ��·������Ҳ������ͬ����
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

    // 2. ������ɫ��
    unsigned int vertex, fragment, geometry;
    int success;
    char infoLog[512];

    // ������ɫ��
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // ��ӡ�����������еĻ���
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Ƭ����ɫ��
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // ��ӡ�����������еĻ���
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ����м�����ɫ��·��������뼸����ɫ��
    if (geometryPath != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        // ��ӡ�����������еĻ���
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // ��ɫ������
    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    // ��ӡ���Ӵ�������еĻ���
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // ɾ����ɫ���������Ѿ����ӵ����ǵĳ������ˣ��Ѿ�������Ҫ��
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr) {
        glDeleteShader(geometry);
    }

    return ID;
}


// ��ʼ��īˮ�ֲ�����
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
            inkData[y * width + x] = 1.0f; // ���ӳ�ʼīˮŨ��
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, inkData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

// ȫ�ֱ��������ڴ洢�ı��ε�VAO��VBO
GLuint quadVAO = 0;
GLuint quadVBO;
GLuint quadEBO;

void createFullScreenQuad() {
    if (quadVAO == 0) {
        float vertices[] = {
            // ����λ��      // ��������
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  // ���Ͻ�
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // ���½�
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // ���½�
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f   // ���Ͻ�
        };
        unsigned int indices[] = {
            0, 1, 2,   // ��һ��������
            0, 2, 3    // �ڶ���������
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // ����λ������
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // ������������
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
    // ��ӡ��ǰ����Ŀ¼
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring ws(buffer);
    std::string currentDir(ws.begin(), ws.end());

    std::string::size_type pos = currentDir.find_last_of("\\/");
    std::cout << "Current working directory: " << currentDir.substr(0, pos) << std::endl;



    // ��ʼ��GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // ����GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ����һ�����ڶ���
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Loading", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ���õ�ǰ������
    glfwMakeContextCurrent(window);

    // ���ô��ڴ�С�ص�
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // �������ص�
    glfwSetCursorPosCallback(window, mouse_callback);
    // ������갴ť�ص�
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // ���ù��ֻص�
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ������Ȳ���
    glEnable(GL_DEPTH_TEST);

    // �����������ɫ������
    unsigned int shaderProgram = loadShader("shader/vertshader.vs", "shader/fragshader.fs");
    // ��ȡuniformλ��

    int edgeRangeLocation = glGetUniformLocation(shaderProgram, "edgeRange");
    int edgeThredLocation = glGetUniformLocation(shaderProgram, "edgeThred");
    int edgePowLocation = glGetUniformLocation(shaderProgram, "edgePow");
    int lightstrengthLocation = glGetUniformLocation(shaderProgram, "lightStrength");

    // ����ģ��
    //Model ourModel("model/rock/cliff_rock1.fbx");
    //Model ourModel("model/Blue/blue.fbx");
    //Model ourModel("model/House/Chinese Brick Broken House.fbx");//����
    //Model ourModel("model/SonowMountain/Snowy.obj");//ѩɽ
    Model ourModel("model/Babydragon/Baby_dragon.fbx");//������ 
    //Model ourModel("model/ccity/city.fbx");
    //Model ourModel("model/shiba/1.FBX");//shiba dog
    //Model ourModel("model/reddragon/dragontrophy.fbx");//dragon head
    
    // ��ʼ��ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // ��ʼ��ImGui���
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // ��������
    float edgeThreshold = 0.25f;
    float edgeRange = 3.5f;
    float edgeThred = 0.1f;
    float edgePow = 0.5f;
    // ��������
    bool mixValue = true;
    glm::vec3 lightPos(12.0f, 10.0f, 20.0f);
    float LightStrength = 0.5f;
    bool kEnabled = false;

    // ��ʼ��īˮ�ֲ�����
    GLuint inkTexture;
    glGenTextures(1, &inkTexture);
    glBindTexture(GL_TEXTURE_2D, inkTexture);

    // �����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    

    // ��ʼ��īˮ����
    int width = 512, height = 512;
    GLuint diffinkTexture = initInkTexture(width, height);

    // ��������ϵͳ
    //unsigned int particleShaderProgram = loadShader("shader/particle.vert", "shader/particle.frag");
    unsigned int particleShaderProgram = loadShader_geo("shader/particle.vert", "shader/particle.frag", "shader/geometry.gs");

    ParticleSystem particleSystem(100000, particleShaderProgram);
    particleSystem.init();
    particleSystem.loadTexture("texture/Particlebrush.png");

    // ��ȡģ�͵Ķ���ͷ������ݲ���������ϵͳ
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    ourModel.getVerticesAndNormals(positions, normals);
    particleSystem.setParticlesFromMesh(positions, normals);

    //�����ı���
    // �����������ɫ������
    unsigned int quatshaderProgram = loadShader("shader/quatvertshader.vs", "shader/quatfragshader.fs");
    // �����ı���
    createFullScreenQuad();

    //��Ⱦ������
    unsigned int screenShaderProgram = loadShader("shader/screenvertshader.vs", "shader/screenfragshader.fs");

    GLuint fbo, screentexture, depthRBO;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // ����һ��������
    glGenTextures(1, &screentexture);
    glBindTexture(GL_TEXTURE_2D, screentexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screentexture, 0);

    // ������ȸ���
    glGenRenderbuffers(1, &depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);


    // ���FBO�Ƿ�����
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    // ��������FBO
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    int frameCount = 0;
    double fps = 0;
    double previousTime = glfwGetTime();
    // ��Ⱦѭ��
    while (!glfwWindowShouldClose(window)) {


        // ����֡ʱ��
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameCount++;
        if (currentFrame - previousTime >= 1.0) { // ÿ�����һ��
            fps = frameCount;
            frameCount = 0;
            previousTime = currentFrame;
        }

        if (deltaTime > 1.0f)
            deltaTime = 0.006f;

        processInput(window);

        // ��FBO������Ⱦ
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        

        // ����ͶӰ����
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        // ��ȡ�۲����
        glm::mat4 view = camera.GetViewMatrix();
        // ����ģ�;���
        glm::mat4 model = glm::mat4(1.0f);

        // ��������ϵͳ
        particleSystem.update(deltaTime,projection,view,model);

        
        // ��ʼ�µ�ImGui֡
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ����ImGui����
        ImGui::Begin("Watercolor Parameters");
        ImGui::SliderFloat("Edge Range", &edgeRange, 1.0f, 10.0f);
        ImGui::SliderFloat("Edge Thred", &edgeThred, 0.0f, 1.0f);
        ImGui::SliderFloat("Edge Pow", &edgePow, 0.1f, 4.0f);
        ImGui::Checkbox("Enable brush texture", &mixValue);
        // ʹ�����ʸ�ѡ��
        ImGui::Checkbox("Enable Curvature", &kEnabled);
        // Add sliders for light position and intensity
        ImGui::SliderFloat3("Light Position", &lightPos.x, -100.0f, 100.0f);
        ImGui::SliderFloat("Light Strength", &LightStrength, 0.0f, 1.0f);
        ImGui::Text("FPS: %.1f", fps);
        ImGui::End();

        
        // ��Ⱦָ��
        glClearColor(0.75f, 0.75f, 0.75f, 1.0f); // ���ñ�����ɫΪǳ��ɫ
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       
        // ʹ����ɫ������Ⱦ�ı���
        glUseProgram(quatshaderProgram);
        glDepthMask(GL_FALSE); // ��ֹ���д��
        renderQuad();          // ��Ⱦȫ���ı���
        glDepthMask(GL_TRUE);  // �����������д��

        

        // ������ɫ��
        glUseProgram(shaderProgram);

        // ��īˮ����
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, diffinkTexture);

        // ����uniform����
        glUniform1i(glGetUniformLocation(shaderProgram, "diffinkTexture"), 1);
        glUniform1f(glGetUniformLocation(shaderProgram, "diffusionRate"), 0.1f);
        glUniform2f(glGetUniformLocation(shaderProgram, "texelSize"), 1.0f / width, 1.0f / height);

        // ����uniform����
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.31f);
        glUniform1i(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);
        glUniform1i(glGetUniformLocation(shaderProgram, "kEnabled"), kEnabled);

        // ���ݾ������ɫ��
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // ����uniform����
        glUniform1f(edgeRangeLocation, edgeRange);  // �������ֵ
        glUniform1f(edgeThredLocation, edgeThred);  // �������ֵ
        glUniform1f(edgePowLocation, edgePow);  // �������ֵ
        glUniform1f(lightstrengthLocation, LightStrength);  // �������ֵ


        // ��Ⱦģ��
        ourModel.Draw(shaderProgram);
        
       
        // ��Ⱦ����ϵͳ
        //particleSystem.render(projection, view, model);


        // ���FBO�󶨣��ص�Ĭ��֡����
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(screenShaderProgram);  // ʹ����һ����ɫ������������
        glBindTexture(GL_TEXTURE_2D, screentexture);
        glUniform1i(glGetUniformLocation(screenShaderProgram, "textureSampler"), 0); // ȷ����ɫ����ʹ�õ��� textureSampler

        // ��ȡuniformλ��
        float currentTime = glfwGetTime();
        GLint timeLocation = glGetUniformLocation(screenShaderProgram, "time");
        glUniform1f(timeLocation, currentTime);

        renderQuad();  // ��Ⱦһ��ȫ���ı�������ʾ����


        // ��Ⱦָ��

        glClear(GL_DEPTH_BUFFER_BIT);//�����Ȼ�����

       

        // ��ȾImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // ��������������ѯIO�¼�
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    
    }

    // ����ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // ����GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    // �ͷ�GLFW��Դ
    glfwTerminate();
    return 0;
}


