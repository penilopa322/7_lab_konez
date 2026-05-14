#define GLEW_DLL
#define GLFW_DLL

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Model.h"
#include "Mesh.h"

class Shader {
public:
    GLuint ID;
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode, fragmentCode;
        std::ifstream vFile(vertexPath), fFile(fragmentPath);
        if (!vFile.is_open() || !fFile.is_open()) {
            std::cerr << "ERROR: cannot open shader files" << std::endl;
            return;
        }
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        vFile.close(); fFile.close();
        vertexCode = vStream.str();
        fragmentCode = fStream.str();
        const char* vCode = vertexCode.c_str();
        const char* fCode = fragmentCode.c_str();

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vCode, NULL);
        glCompileShader(vs);
        checkCompileErrors(vs, "VERTEX");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fCode, NULL);
        glCompileShader(fs);
        checkCompileErrors(fs, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vs);
        glAttachShader(ID, fs);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    void use() { glUseProgram(ID); }
    void setVec4(const char* name, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
    }
    void setVec3(const char* name, float x, float y, float z) {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }
    void setVec3(const char* name, const glm::vec3& value) {
        glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
    }
    void setFloat(const char* name, float value) {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }
    void setMat4(const char* name, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(mat));
    }
private:
    void checkCompileErrors(GLuint shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "Shader compilation error (" << type << "):\n" << infoLog << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "Program linking error:\n" << infoLog << std::endl;
            }
        }
    }
};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = 400.0f, lastY = 400.0f;

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processCameraInput(GLFWwindow* window, float deltaTime) {
    float cameraSpeed = 3.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "ERROR: could not start GLFW3." << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 800, "Lab7 - Model Animation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: glewInit failed" << std::endl;
        return 1;
    }
    glEnable(GL_DEPTH_TEST);

    Model myModel("Lab_3_VAR_25.obj");
    std::cout << "Loaded meshes: " << myModel.meshes.size() << std::endl;
    for (size_t i = 0; i < myModel.meshes.size(); ++i)
        std::cout << "Mesh " << i << " vertices: " << myModel.meshes[i].vertices.size() << std::endl;

    Shader shader("vertex.glsl", "fragment.glsl");

    float part1_y = 0.0f;   
    float part2_x = 0.0f;   
    float part3_y = 0.0f;   

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processCameraInput(window, deltaTime);

        float speed = 2.0f * deltaTime;
        // управление меш 0
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) part1_y += speed;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) part1_y -= speed;
        // управление меш 1
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) part2_x += speed;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) part2_x -= speed;
        // ууправление меш 3
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) part3_y += speed;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) part3_y -= speed;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", cameraPos);

        glm::vec3 lightPos(3.0f, 4.0f, 5.0f);
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        shader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        shader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        shader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        shader.setFloat("material.shininess", 32.0f);

        if (myModel.meshes.size() > 0) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, part1_y, 0.0f));
            shader.setMat4("model", model);
            myModel.meshes[0].Draw();
        }

        if (myModel.meshes.size() > 1) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(part2_x, 0.0f, 0.0f));
            shader.setMat4("model", model);
            myModel.meshes[1].Draw();
        }
        // меш 2(индекс 2) – неподвижный
        if (myModel.meshes.size() > 2) {
            shader.setMat4("model", glm::mat4(1.0f));
            myModel.meshes[2].Draw();
        }

        if (myModel.meshes.size() > 3) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, part3_y, 0.0f));
            shader.setMat4("model", model);
            myModel.meshes[3].Draw();
        }
        for (size_t i = 4; i < myModel.meshes.size(); ++i) {
            shader.setMat4("model", glm::mat4(1.0f));
            myModel.meshes[i].Draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}