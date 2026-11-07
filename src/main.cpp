#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"
#include "PerlinNoise.hpp"
#include "Shader.hpp"
#include "TerrainMesh.hpp"

#include <algorithm>
#include <iostream>
#include <random>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Procedural Terrain Generator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    Shader shader(SHADER_DIR "/terrain.vs", SHADER_DIR "/terrain.fs");

    // Terrain parameters
    int gridResolution = 180;
    float worldSize = 120.0f;
    float amplitude = 16.0f;
    float frequency = 0.024f;
    int octaves = 5;
    unsigned int seed = 42;

    PerlinNoise noise(seed);
    TerrainMesh terrain;
    terrain.init();
    terrain.generate(noise, gridResolution, worldSize, amplitude, frequency, octaves);

    Camera camera(glm::vec3(0.0f, 28.0f, 48.0f));

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool firstMouse = true;

    bool wireframe = false;
    double lastTime = glfwGetTime();

    auto printStats = [&]() {
        std::cout << "[Terrain] Resolution: " << gridResolution << "x" << gridResolution
                  << " | Amplitude: " << amplitude
                  << " | Frequency: " << frequency
                  << " | Octaves: " << octaves
                  << " | Seed: " << seed << '\n';
    };

    std::cout << "========================================\n"
              << "     Procedural Terrain Generator\n"
              << "========================================\n"
              << " Left/Right Mouse Drag : Look around\n"
              << " W / A / S / D         : Fly camera\n"
              << " Space / Left Ctrl     : Ascend / Descend\n"
              << " Left Shift (Hold)     : Speed boost\n"
              << " Up / Down Arrows      : Adjust Amplitude (Height)\n"
              << " Right / Left Arrows   : Adjust Frequency (Roughness)\n"
              << " O / P                 : Decrease / Increase Octaves\n"
              << " M                     : Toggle Wireframe / Shaded\n"
              << " R                     : Generate New Terrain Seed\n"
              << " = / -                 : Adjust Grid Resolution\n"
              << " ESC                   : Exit\n"
              << "========================================\n";
    printStats();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        deltaTime = std::min(deltaTime, 0.05f);

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        bool needsRegen = false;

        // Amplitude tweak
        static bool upPressed = false;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            if (!upPressed) { amplitude += 2.0f; needsRegen = true; upPressed = true; }
        } else { upPressed = false; }

        static bool downPressed = false;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            if (!downPressed) { amplitude = std::max(2.0f, amplitude - 2.0f); needsRegen = true; downPressed = true; }
        } else { downPressed = false; }

        // Frequency tweak
        static bool rightPressed = false;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            if (!rightPressed) { frequency += 0.004f; needsRegen = true; rightPressed = true; }
        } else { rightPressed = false; }

        static bool leftPressed = false;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            if (!leftPressed) { frequency = std::max(0.005f, frequency - 0.004f); needsRegen = true; leftPressed = true; }
        } else { leftPressed = false; }

        // Octaves tweak
        static bool pPressed = false;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            if (!pPressed) { octaves = std::min(8, octaves + 1); needsRegen = true; pPressed = true; }
        } else { pPressed = false; }

        static bool oPressed = false;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            if (!oPressed) { octaves = std::max(1, octaves - 1); needsRegen = true; oPressed = true; }
        } else { oPressed = false; }

        // Random Seed
        static bool rPressed = false;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!rPressed) {
                seed = std::random_device{}();
                noise.reseed(seed);
                needsRegen = true;
                rPressed = true;
            }
        } else { rPressed = false; }

        // Resolution tweak
        static bool plusPressed = false;
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            if (!plusPressed) {
                gridResolution = std::min(300, gridResolution + 30);
                needsRegen = true;
                plusPressed = true;
            }
        } else { plusPressed = false; }

        static bool minusPressed = false;
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            if (!minusPressed) {
                gridResolution = std::max(40, gridResolution - 30);
                needsRegen = true;
                minusPressed = true;
            }
        } else { minusPressed = false; }

        // Wireframe toggle
        static bool mPressed = false;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!mPressed) {
                wireframe = !wireframe;
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
                mPressed = true;
            }
        } else { mPressed = false; }

        if (needsRegen) {
            terrain.generate(noise, gridResolution, worldSize, amplitude, frequency, octaves);
            printStats();
        }

        // Mouse look
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (firstMouse) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }

        float xOffset = static_cast<float>(mouseX - lastMouseX);
        float yOffset = static_cast<float>(lastMouseY - mouseY);
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        bool mouseHeld = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
                          glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

        if (mouseHeld) {
            camera.processMouseMovement(xOffset, yOffset);
        }

        // Camera flight movement
        bool boost = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(CameraMovement::FORWARD, deltaTime, boost);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(CameraMovement::BACKWARD, deltaTime, boost);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(CameraMovement::LEFT, deltaTime, boost);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(CameraMovement::RIGHT, deltaTime, boost);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard(CameraMovement::UP, deltaTime, boost);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) camera.processKeyboard(CameraMovement::DOWN, deltaTime, boost);

        // Rendering
        int winWidth, winHeight;
        glfwGetFramebufferSize(window, &winWidth, &winHeight);

        glClearColor(0.65f, 0.75f, 0.88f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (winHeight > 0) ? static_cast<float>(winWidth) / static_cast<float>(winHeight) : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 400.0f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        shader.use();
        shader.setMat4("u_Projection", projection);
        shader.setMat4("u_View", view);
        shader.setMat4("u_Model", model);
        shader.setVec3("u_CamPos", camera.position);
        shader.setVec3("u_SunDir", glm::normalize(glm::vec3(0.5f, 0.7f, 0.4f)));
        shader.setFloat("u_Amplitude", amplitude);
        shader.setInt("u_WireframeMode", wireframe ? 1 : 0);

        terrain.draw();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

