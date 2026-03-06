#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "headers/stb_image.h"

#include "headers/shader.h"
#include "headers/camera.h"
#include "headers/model.h"
#include "headers/IKChain.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = SCR_WIDTH * 0.5f;
float lastY = SCR_HEIGHT * 0.5f;
bool firstMouse = true;

IKChain chain;
glm::vec3 target(2.0f, -2.0f, 1.0f);
bool scriptedMode = true;
bool scriptedPaused = false;
float scriptedTime = 0.0f;
float scriptedDuration = 10.0f;

const std::vector<glm::vec3> splineControlPoints = {
    glm::vec3(1.8f, -1.0f,  1.6f),
    glm::vec3(2.8f, -2.2f,  0.8f),
    glm::vec3(1.4f, -3.4f, -0.5f),
    glm::vec3(-0.6f, -2.8f, -1.4f),
    glm::vec3(-1.4f, -1.3f, -0.4f),
    glm::vec3(0.4f, -0.8f,  1.2f),
    glm::vec3(2.2f, -1.6f,  1.7f)
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
glm::mat4 makeBoneTransform(const glm::vec3& start, const glm::vec3& end, float modelLength = 5.0f);
glm::vec3 pickOnPlane(double mouseX, double mouseY, int winWidth, int winHeight, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& planePoint, const glm::vec3& planeNormal, bool& ok);
float easeInOutCubic(float t);
glm::vec3 sampleSplineLoop(const std::vector<glm::vec3>& points, float t01);
void updateScriptedTarget();

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IK Arm Model", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);

    Shader modelShader("src/model.vs", "src/model.fs");

    Model baseModel("herewego/base.obj");
    Model upperArmModel("herewego/UpperArm.obj");
    Model lowerArmModel("herewego/LowerArm.obj");
    Model handModel("herewego/Hand.obj");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        updateScriptedTarget();
        chain.solveFABRIK(target, 25, 0.0008f);

        glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int fbW = 0;
        int fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        float aspect = (fbH > 0) ? (static_cast<float>(fbW) / static_cast<float>(fbH)) : 1.0f;

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setVec3("lightDir", glm::normalize(glm::vec3(-0.4f, -1.0f, -0.2f)));

        glm::mat4 baseM = glm::translate(glm::mat4(1.0f), chain.joints[0]);
        baseM = glm::scale(baseM, glm::vec3(1.1f, 0.25f, 1.1f));
        modelShader.setMat4("model", baseM);
        modelShader.setVec3("baseColor", glm::vec3(0.35f, 0.40f, 0.45f));
        baseModel.Draw(modelShader);

        glm::mat4 upperM = makeBoneTransform(chain.joints[0], chain.joints[1]);
        modelShader.setMat4("model", upperM);
        modelShader.setVec3("baseColor", glm::vec3(0.8f, 0.5f, 0.2f));
        upperArmModel.Draw(modelShader);

        glm::mat4 lowerM = makeBoneTransform(chain.joints[1], chain.joints[2]);
        modelShader.setMat4("model", lowerM);
        modelShader.setVec3("baseColor", glm::vec3(0.2f, 0.65f, 0.85f));
        lowerArmModel.Draw(modelShader);

        glm::mat4 handM = makeBoneTransform(chain.joints[2], chain.joints[3]);
        modelShader.setMat4("model", handM);
        modelShader.setVec3("baseColor", glm::vec3(0.85f, 0.25f, 0.3f));
        handModel.Draw(modelShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mPressed)
    {
        scriptedMode = !scriptedMode;
        mPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        mPressed = false;

    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
        scriptedPaused = !scriptedPaused;
        spacePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spacePressed = false;

    static bool lbracketPressed = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && !lbracketPressed)
    {
        scriptedDuration = std::min(25.0f, scriptedDuration + 1.0f);
        lbracketPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_RELEASE)
        lbracketPressed = false;

    static bool rbracketPressed = false;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS && !rbracketPressed)
    {
        scriptedDuration = std::max(3.0f, scriptedDuration - 1.0f);
        rbracketPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_RELEASE)
        rbracketPressed = false;

    if (!scriptedMode)
    {
        float targetSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            target.x -= targetSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            target.x += targetSpeed;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            target.y += targetSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            target.y -= targetSpeed;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            target.z += targetSpeed;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            target.z -= targetSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        target = glm::vec3(2.0f, -2.0f, 1.0f);
        scriptedTime = 0.0f;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        firstMouse = true;
        return;
    }

    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)mods;
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
        return;

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int winW = 0;
    int winH = 0;
    glfwGetWindowSize(window, &winW, &winH);
    if (winW <= 0 || winH <= 0)
        return;

    int fbW = 0;
    int fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    float aspect = (fbH > 0) ? static_cast<float>(fbW) / static_cast<float>(fbH)
                             : static_cast<float>(winW) / static_cast<float>(winH);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // Primary interaction plane: XY plane through root (z = root.z).
    bool ok = false;
    glm::vec3 hit = pickOnPlane(
        mouseX, mouseY, winW, winH, view, projection,
        chain.initialRootPosition, glm::vec3(0.0f, 0.0f, 1.0f), ok);

    // Fallback plane: XZ plane through root (y = root.y) if primary is parallel/misses.
    if (!ok)
    {
        hit = pickOnPlane(
            mouseX, mouseY, winW, winH, view, projection,
            chain.initialRootPosition, glm::vec3(0.0f, 1.0f, 0.0f), ok);
    }

    if (ok)
    {
        scriptedMode = false;
        target = hit;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

glm::mat4 makeBoneTransform(const glm::vec3& start, const glm::vec3& end, float modelLength)
{
    glm::vec3 dir = end - start;
    float boneLen = glm::length(dir);
    if (boneLen < 1e-6f)
        return glm::translate(glm::mat4(1.0f), start);

    dir = glm::normalize(dir);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    float d = glm::clamp(glm::dot(up, dir), -1.0f, 1.0f);
    glm::mat4 R(1.0f);

    if (d < 0.9999f)
    {
        if (d > -0.9999f)
        {
            glm::vec3 axis = glm::normalize(glm::cross(up, dir));
            float angle = std::acos(d);
            R = glm::rotate(glm::mat4(1.0f), angle, axis);
        }
        else
        {
            R = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }

    glm::mat4 T = glm::translate(glm::mat4(1.0f), start);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, boneLen / modelLength, 1.0f));

    return T * R * S;
}

glm::vec3 pickOnPlane(double mouseX, double mouseY, int winWidth, int winHeight, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& planePoint, const glm::vec3& planeNormal, bool& ok)
{
    float x = static_cast<float>((2.0 * mouseX) / static_cast<double>(winWidth) - 1.0);
    float y = static_cast<float>(1.0 - (2.0 * mouseY) / static_cast<double>(winHeight));

    glm::mat4 invVP = glm::inverse(projection * view);
    glm::vec4 nearNDC(x, y, -1.0f, 1.0f);
    glm::vec4 farNDC(x, y, 1.0f, 1.0f);

    glm::vec4 nearWorld = invVP * nearNDC;
    glm::vec4 farWorld = invVP * farNDC;
    nearWorld /= nearWorld.w;
    farWorld /= farWorld.w;

    glm::vec3 rayOrigin = glm::vec3(nearWorld);
    glm::vec3 rayDir = glm::normalize(glm::vec3(farWorld - nearWorld));

    glm::vec3 n = glm::normalize(planeNormal);
    float denom = glm::dot(rayDir, n);
    if (std::abs(denom) < 1e-6f)
    {
        ok = false;
        return glm::vec3(0.0f);
    }

    float t = glm::dot(planePoint - rayOrigin, n) / denom;
    if (t <= 0.0f)
    {
        ok = false;
        return glm::vec3(0.0f);
    }

    ok = true;
    return rayOrigin + t * rayDir;
}

float easeInOutCubic(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    if (t < 0.5f)
        return 4.0f * t * t * t;
    float f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f * 0.5f);
}

glm::vec3 sampleSplineLoop(const std::vector<glm::vec3>& points, float t01)
{
    if (points.empty())
        return glm::vec3(0.0f);
    if (points.size() < 4)
        return points.front();

    int n = static_cast<int>(points.size());
    float wrapped = t01 - std::floor(t01);
    float u = wrapped * static_cast<float>(n);
    int i1 = static_cast<int>(std::floor(u)) % n;
    float t = u - std::floor(u);

    int i0 = (i1 - 1 + n) % n;
    int i2 = (i1 + 1) % n;
    int i3 = (i1 + 2) % n;

    const glm::vec3& P0 = points[i0];
    const glm::vec3& P1 = points[i1];
    const glm::vec3& P2 = points[i2];
    const glm::vec3& P3 = points[i3];

    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * ((2.0f * P1) +
                   (-P0 + P2) * t +
                   (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * t2 +
                   (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * t3);
}

void updateScriptedTarget()
{
    if (!scriptedMode || scriptedPaused || scriptedDuration <= 0.0f)
        return;

    scriptedTime += deltaTime;
    float cycle = std::fmod(scriptedTime, scriptedDuration) / scriptedDuration;

    // Ease is applied on time parameter to create smooth accelerations/decelerations.
    float eased = easeInOutCubic(cycle);
    target = sampleSplineLoop(splineControlPoints, eased);
}
