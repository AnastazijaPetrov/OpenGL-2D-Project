#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

const int NUM_STOPS = 10;
const int NUM_SEGMENTS = 20;
const int TOTAL_POINTS = NUM_STOPS * NUM_SEGMENTS;
const double FRAME_DELAY = 1.0 / 75.0;
const double STOP_WAIT_TIME = 10.0;
const float BUS_SPEED = 8.0f;

struct TextureData {
    unsigned int id;
    float ar;
};

struct RenderContext {
    unsigned int basicShader;
    unsigned int lineShader;
    unsigned int textShader;
    unsigned int quadVAO, quadVBO;
    unsigned int lineVAO, lineVBO;
    int uPosLoc;
    int uSLoc;
    int screenW;
    int screenH;
    float screenAR;
};

struct HUDTextures {
    TextureData doorOpen;
    TextureData doorClosed;
    TextureData inspector;
    TextureData nameSurname;
};

struct BusState {
    float x;
    float y;
    float pathProgress = 0.0f;
    double stopTimer = 0.0;
    bool isAtStop = true;
    int passengerCount = 0;
    int finesIssued = 0;
    bool isInspectorPresent = false;
};

TextureData loadTextureData(const char* filepath);
void createUniversalQuad(unsigned int& VAO, unsigned int& VBO);
std::vector<float> generateBusRoute(const float stopCoords[]);
void createPath(const std::vector<float>& pathCoords, unsigned int& VAO, unsigned int& VBO);

void updateBusLogic(double deltaTime, BusState& bus, const std::vector<float>& pathCoords);
void drawRoute(const RenderContext& ctx, size_t pointCount);
void drawQuad(const RenderContext& ctx, unsigned int textureId, float posX, float posY, float scaleX, float scaleY);
void drawStops(const RenderContext& ctx, const TextureData& stopTex, const float stopCoords[], float scaleHeight);
void drawHUD(const RenderContext& ctx, const HUDTextures& hud, const BusState& bus);
void cleanup(RenderContext& ctx);