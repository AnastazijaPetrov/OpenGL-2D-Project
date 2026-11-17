#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Util.h"
#include "TextRenderer.h"
#include "BusSimulation.h"

GLFWcursor* cursorDefault;
GLFWcursor* cursorPress;
BusState bus;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        if (bus.isAtStop && !bus.isInspectorPresent && bus.passengerCount < 50) {
            bus.isInspectorPresent = true;
            bus.passengerCount++;
        }
    }
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwSetCursor(window, cursorPress);
        if (bus.isAtStop && bus.passengerCount < 50) {
            bus.passengerCount++;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        glfwSetCursor(window, cursorDefault);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        int minPassengers = bus.isInspectorPresent ? 1 : 0;
        if (bus.isAtStop && bus.passengerCount > minPassengers) {
            bus.passengerCount--;
        }
    }
}

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Project SV26-2023", primaryMonitor, NULL);
    if (!window) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyboardCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);

    cursorDefault = loadImageToCursor("res/cursor.png");
    cursorPress = loadImageToCursor("res/cursorPressed.png");
    glfwSetCursor(window, cursorDefault);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    RenderContext renderContext;
    renderContext.screenW = mode->width;
    renderContext.screenH = mode->height;
    renderContext.screenAR = (float)mode->width / (float)mode->height;

    renderContext.basicShader = createShader("basic.vert", "basic.frag");
    renderContext.lineShader = createShader("line.vert", "line.frag");
    renderContext.textShader = createShader("text.vert", "text.frag");

    glUseProgram(renderContext.basicShader);
    glUniform1i(glGetUniformLocation(renderContext.basicShader, "uTex0"), 0);
    renderContext.uPosLoc = glGetUniformLocation(renderContext.basicShader, "uPos");
    renderContext.uSLoc = glGetUniformLocation(renderContext.basicShader, "uS");

    TextureData busStopTex = loadTextureData("res/busStop.png");
    float stopScaleHeight = 0.6f;

    TextureData busTex = loadTextureData("res/bus.png");
    float busScaleHeight = 0.6f;

    HUDTextures hud = {
        loadTextureData("res/doorOpen.png"),
        loadTextureData("res/doorClosed.png"),
        loadTextureData("res/ticketInspector.png"),
        loadTextureData("res/namesurname.png")
    };

    float stopCoords[NUM_STOPS * 2] = {
        -0.6f, -0.45f, -0.35f, -0.6f,  0.0f, -0.7f,  0.35f, -0.5f,  0.6f, -0.15f,
        0.5f,  0.3f,   0.15f,  0.6f, -0.2f,  0.55f, -0.5f,  0.3f,  -0.65f, -0.05f
    };

    createUniversalQuad(renderContext.quadVAO, renderContext.quadVBO);

    std::vector<float> pathCoords = generateBusRoute(stopCoords);
    createPath(pathCoords, renderContext.lineVAO, renderContext.lineVBO);

    initText("res/arial.ttf", renderContext.textShader);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bus.x = stopCoords[0];
    bus.y = stopCoords[1];
    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double startTime = glfwGetTime();
        double deltaTime = startTime - lastFrameTime;
        lastFrameTime = startTime;

        glClear(GL_COLOR_BUFFER_BIT);

        updateBusLogic(deltaTime, bus, pathCoords);

        drawRoute(renderContext, pathCoords.size() / 2);
        drawStops(renderContext, busStopTex, stopCoords, stopScaleHeight);
        drawQuad(renderContext, busTex.id, bus.x, bus.y, (busScaleHeight * busTex.ar) / renderContext.screenAR, busScaleHeight);
        drawHUD(renderContext, hud, bus);

        glfwSwapBuffers(window);
        glfwPollEvents();
        while (glfwGetTime() - startTime < FRAME_DELAY) {}
    }

    cleanup(renderContext);
    return 0;
}