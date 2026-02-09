#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include<vector>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "mesh.h"

const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 2.0f;	
float lastFrame = 0.0f;


float pitch = 0.0f;
float yaw = 0.0f;
float roll = 0.0f;

float rotationSpeed = 50.0f; 

glm::quat planeOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

bool useQuaternions = false;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);

struct Keyframe {
    float time;           
    glm::vec3 position;   
    glm::quat rotation;   
};

std::vector<Keyframe> flightPath;

void initFlightPath();

bool autoPilot = false;
glm::vec3 getInterpolatedPosition(float time) {
    if (flightPath.empty()) return glm::vec3(0.0f);
    
    float totalTime = flightPath.back().time;
    float currentTime = fmod(time, totalTime);

    for (int i = 0; i < flightPath.size() - 1; i++) {
        if (currentTime >= flightPath[i].time && currentTime <= flightPath[i+1].time) {
            
            float t = (currentTime - flightPath[i].time) / (flightPath[i+1].time - flightPath[i].time);

            // smoothing formula 
            t = t * t * (3.0f - 2.0f * t); 

            // mix and slerp
            return glm::mix(flightPath[i].position, flightPath[i+1].position, t);        }
    }
    return flightPath[0].position;
}

//the slerp thing 
glm::quat getInterpolatedRotation(float time) {
    if (flightPath.empty()) return glm::quat(1, 0, 0, 0);

    float totalTime = flightPath.back().time;
    float currentTime = fmod(time, totalTime);

    for (int i = 0; i < flightPath.size() - 1; i++) {
        if (currentTime >= flightPath[i].time && currentTime <= flightPath[i+1].time) {
            float t = (currentTime - flightPath[i].time) / (flightPath[i+1].time - flightPath[i].time);
            return glm::slerp(flightPath[i].rotation, flightPath[i+1].rotation, t);
        }
    }
    return flightPath[0].rotation;
}

int main()
{
    if(!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Plane Rotations", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    initFlightPath();

    Shader basicShader("shaders/basic.vert", "shaders/basic.frag");
    Shader phongShader("shaders/phong.vert", "shaders/phong.frag");

    Model planeModel("assets/plane /LooL.obj");

    // Skybox Setup
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


std::vector<std::string> faces {
        "assets/skybox/px.png", "assets/skybox/nx.png",
        "assets/skybox/py.png", "assets/skybox/ny.png",
        "assets/skybox/pz.png", "assets/skybox/nz.png"
    };
    
    unsigned int cubemapTexture = loadCubemap(faces);

    basicShader.use();
    basicShader.setInt("skybox", 0);

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        std::string modeStr = useQuaternions ? "QUATERNION" : "EULER (Gimbal Lock Demo)";
        std::string title = "PlaneRotation | Mode: " + modeStr + 
                        " | Pitch: " + std::to_string((int)pitch % 360) + 
                        " | Yaw: " + std::to_string((int)yaw % 360) + 
                        " | Roll: " + std::to_string((int)roll % 360);

        glfwSetWindowTitle(window, title.c_str());

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
    
        // 1. DRAW SKYBOX
        glDepthFunc(GL_LEQUAL); 
        basicShader.use();
  
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
        basicShader.setMat4("view", skyboxView);
        basicShader.setMat4("projection", projection);
        
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); 

        
        phongShader.use();
        glm::mat4 model = glm::mat4(1.0f);

        if (autoPilot) 
        {
            // AUTOPILOT MODE
            float time = glfwGetTime();
            glm::vec3 currentPos = getInterpolatedPosition(time);
            glm::quat currentRot = getInterpolatedRotation(time);

            model = glm::translate(model, currentPos);
            model *= glm::mat4_cast(currentRot);

            //view = glm::lookAt(currentPos + glm::vec3(0,5,20), currentPos, glm::vec3(0, 1, 0));
        }   
        else 
        {
            // MANUAL MODE
            model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));

            if (useQuaternions)
            {
                // Quaternion rotation
                model = model * glm::mat4_cast(planeOrientation);
            }
            else 
            {
                // Euler rotation (Demonstrates Gimbal Lock)
                model = glm::rotate(model, glm::radians(yaw),   glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(roll),  glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }

        model = glm::scale (model, glm::vec3(1.0f));
    
        phongShader.setMat4("projection", projection);
        phongShader.setMat4("view", view);
        phongShader.setMat4("model", model);

        phongShader.setVec3("lightPos", glm::vec3(20.0f, 5.0f, -10.0f)); 
        phongShader.setVec3("lightColor", glm::vec3(1.0f, 0.9f, 0.8f)); // Warm white
        phongShader.setVec3("viewPos", camera.Position);            
        planeModel.Draw(phongShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    // 1. ESCAPE TO CLOSE
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle Logic (Spacebar)
    static bool spaceWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceWasPressed) {
        useQuaternions = !useQuaternions;
        spaceWasPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) spaceWasPressed = false;

    float angle = rotationSpeed * deltaTime;

    if (useQuaternions) {
        // --- QUATERNION LOGIC (Resolves Gimbal Lock) ---
        glm::quat deltaQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        // Pitch (X-axis): Up/Down
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)) * deltaQuat;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(-angle), glm::vec3(1.0f, 0.0f, 0.0f)) * deltaQuat;

        // Roll (Z-axis): Left/Right
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) * deltaQuat;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(-angle), glm::vec3(0.0f, 0.0f, 1.0f)) * deltaQuat;

        // Yaw (Y-axis): Q/E
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) * deltaQuat;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            deltaQuat = glm::angleAxis(glm::radians(-angle), glm::vec3(0.0f, 1.0f, 0.0f)) * deltaQuat;

        planeOrientation = planeOrientation * deltaQuat;
        planeOrientation = glm::normalize(planeOrientation);
    } 
    else {
        // --- EULER LOGIC (Demonstrates Gimbal Lock) ---
        // Pitch: Up/Down
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += angle;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= angle;

        // Roll: Left/Right
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  roll += angle;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) roll -= angle;

        // Yaw: Q/E
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)     yaw += angle;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)     yaw -= angle;
    }

    // camera controls 
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);

    // Reset R key
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        pitch = 0.0f; yaw = 0.0f; roll = 0.0f;
        planeOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    static bool pWasPressed = false;
if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pWasPressed) {
    autoPilot = !autoPilot;
    pWasPressed = true;
}
if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pWasPressed = false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    float xpos = static_cast<float>(xPos);
    float ypos = static_cast<float>(yPos);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yOffset) * 5.0f);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    
    
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            
            GLenum format = GL_RGB;
            if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                        0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // Set texture parameters 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void initFlightPath() {
    flightPath.clear();
    
    // Keyframe 0: Center (Start)
    flightPath.push_back({0.0f, glm::vec3(0, 5, 0), glm::quat(1, 0, 0, 0)});

    // Keyframe 1: Wide Left Loop (Banking 45 degrees)
    // We move further out (25 units) and give it more time (4s)
    flightPath.push_back({4.0f, glm::vec3(-25, 10, -20), glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 0, 1))});

    // Keyframe 2: Back to Center
    flightPath.push_back({8.0f, glm::vec3(0, 5, 0), glm::quat(1, 0, 0, 0)});

    // Keyframe 3: Wide Right Loop (Banking -45 degrees)
    flightPath.push_back({12.0f, glm::vec3(25, 10, 20), glm::angleAxis(glm::radians(-45.0f), glm::vec3(0, 0, 1))});

    // Keyframe 4: Return to Start
    flightPath.push_back({16.0f, glm::vec3(0, 5, 0), glm::quat(1, 0, 0, 0)});
}