#include "BusSimulation.h"
#include "Util.h"
#include "stb_image.h"
#include "TextRenderer.h"
#include <cstdlib>

TextureData loadTextureData(const char* filepath) {
    TextureData tex;
    tex.id = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    if (stbi_info(filepath, &width, &height, &channels)) {
        tex.ar = (float)width / (float)height;
    }
    else {
        tex.ar = 1.0f;
    }
    return tex;
}

void createUniversalQuad(unsigned int& VAO, unsigned int& VBO) {
    float vertices[] = {
         -0.1f,  0.1f,    0.0f, 1.0f,
         -0.1f, -0.1f,    0.0f, 0.0f,
          0.1f, -0.1f,    1.0f, 0.0f,
          0.1f,  0.1f,    1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

std::vector<float> generateBusRoute(const float stopCoords[]) {
    std::vector<float> pathCoords;

    for (int i = 0; i < NUM_STOPS; i++) {
        float p0x = stopCoords[i * 2];
        float p0y = stopCoords[i * 2 + 1];

        int next = (i + 1) % NUM_STOPS;
        float p2x = stopCoords[next * 2];
        float p2y = stopCoords[next * 2 + 1];

        float midX = (p0x + p2x) / 2.0f;
        float midY = (p0y + p2y) / 2.0f;

        float nx = -(p2y - p0y);
        float ny = (p2x - p0x);

        float offsetAmount = ((rand() % 100) / 100.0f) - 0.5f;
        float p1x = midX + nx * offsetAmount;
        float p1y = midY + ny * offsetAmount;

        for (int j = 0; j < NUM_SEGMENTS; j++) {
            float t = (float)j / (float)NUM_SEGMENTS;
            float x = (1 - t) * (1 - t) * p0x + 2 * (1 - t) * t * p1x + t * t * p2x;
            float y = (1 - t) * (1 - t) * p0y + 2 * (1 - t) * t * p1y + t * t * p2y;
            pathCoords.push_back(x);
            pathCoords.push_back(y);
        }
    }

    return pathCoords;
}

void createPath(const std::vector<float>& pathCoords, unsigned int& VAO, unsigned int& VBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, pathCoords.size() * sizeof(float), pathCoords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void updateBusLogic(double deltaTime, BusState& bus, const std::vector<float>& pathCoords) {
    if (bus.isAtStop) {
        bus.stopTimer += deltaTime;
        if (bus.stopTimer >= STOP_WAIT_TIME) {
            bus.isAtStop = false;
            bus.stopTimer = 0.0;
        }
    }
    else {
        float prevProgress = bus.pathProgress;
        bus.pathProgress += BUS_SPEED * (float)deltaTime;

        int prevStopIdx = (int)(prevProgress / NUM_SEGMENTS);
        int currStopIdx = (int)(bus.pathProgress / NUM_SEGMENTS);

        if (currStopIdx > prevStopIdx) {
            bus.pathProgress = (float)((currStopIdx % NUM_STOPS) * NUM_SEGMENTS);
            bus.isAtStop = true;

            if (bus.isInspectorPresent) {
                bus.isInspectorPresent = false;
                bus.passengerCount--;

                if (bus.passengerCount > 0) {
                    int finesThisStop = rand() % (bus.passengerCount + 1);
                    bus.finesIssued += finesThisStop;
                }
            }
        }
        else if (bus.pathProgress >= TOTAL_POINTS) {
            bus.pathProgress -= TOTAL_POINTS;
        }

        int idx0 = ((int)bus.pathProgress) % TOTAL_POINTS;
        int idx1 = (idx0 + 1) % TOTAL_POINTS;
        float t = bus.pathProgress - (int)bus.pathProgress;

        bus.x = pathCoords[idx0 * 2] + t * (pathCoords[idx1 * 2] - pathCoords[idx0 * 2]);
        bus.y = pathCoords[idx0 * 2 + 1] + t * (pathCoords[idx1 * 2 + 1] - pathCoords[idx0 * 2 + 1]);
    }
}

void drawRoute(const RenderContext& ctx, size_t pointCount) {
    glUseProgram(ctx.lineShader);
    glBindVertexArray(ctx.lineVAO);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_LOOP, 0, pointCount);
    glLineWidth(1.0f);
}

void drawQuad(const RenderContext& ctx, unsigned int textureId, float posX, float posY, float scaleX, float scaleY) {
    glUseProgram(ctx.basicShader);
    glBindVertexArray(ctx.quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform2f(ctx.uSLoc, scaleX, scaleY);
    glUniform2f(ctx.uPosLoc, posX, posY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawStops(const RenderContext& ctx, const TextureData& stopTex, const float stopCoords[], float scaleHeight) {
    float scaleX = (scaleHeight * stopTex.ar) / ctx.screenAR;
    float scaleY = scaleHeight;

    glUseProgram(ctx.basicShader);
    glBindVertexArray(ctx.quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stopTex.id);
    glUniform2f(ctx.uSLoc, scaleX, scaleY);

    for (int i = 0; i < NUM_STOPS; i++) {
        glUniform2f(ctx.uPosLoc, stopCoords[i * 2], stopCoords[i * 2 + 1]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    for (int i = 0; i < NUM_STOPS; i++) {
        RenderTextCentered(ctx.textShader, std::to_string(i), stopCoords[i * 2], stopCoords[i * 2 + 1], 0.4f, 1.0f, 1.0f, 1.0f, ctx.screenW, ctx.screenH);
    }
}

void drawHUD(const RenderContext& ctx, const HUDTextures& hud, const BusState& bus) {
    const TextureData& currentDoor = bus.isAtStop ? hud.doorOpen : hud.doorClosed;
    float doorSY = 1.5f;
    float doorSX = (doorSY * currentDoor.ar) / ctx.screenAR;
    float padX = 0.06f;
    float padY = 0.10f;
    drawQuad(ctx, currentDoor.id, -1.0f + 0.1f * doorSX + padX, 1.0f - 0.1f * doorSY - padY, doorSX, doorSY);

    if (bus.isInspectorPresent) {
        float inspSY = 2.0f;
        float inspSX = (inspSY * hud.inspector.ar) / ctx.screenAR;
        drawQuad(ctx, hud.inspector.id, 1.0f - 0.1f * inspSX, -1.0f + 0.1f * inspSY, inspSX, inspSY);
    }

    float nameSY = 1.0f;
    float nameSX = (nameSY * hud.nameSurname.ar) / ctx.screenAR;
    drawQuad(ctx, hud.nameSurname.id, -1.0f + 0.1f * nameSX, -1.0f + 0.1f * nameSY, nameSX, nameSY);

    std::string textPassengers = "Passengers: " + std::to_string(bus.passengerCount);
    std::string textFines = "Fines issued: " + std::to_string(bus.finesIssued);
    RenderText(ctx.textShader, textPassengers, 0.65f, 0.85f, 0.4f, 0.0f, 0.0f, 0.0f, ctx.screenW, ctx.screenH);
    RenderText(ctx.textShader, textFines, 0.65f, 0.75f, 0.4f, 0.0f, 0.0f, 0.0f, ctx.screenW, ctx.screenH);
}

void cleanup(RenderContext& ctx) {
    glDeleteVertexArrays(1, &ctx.quadVAO);
    glDeleteBuffers(1, &ctx.quadVBO);
    glDeleteVertexArrays(1, &ctx.lineVAO);
    glDeleteBuffers(1, &ctx.lineVBO);
    glDeleteProgram(ctx.basicShader);
    glDeleteProgram(ctx.lineShader);
    glDeleteProgram(ctx.textShader);
    glfwTerminate();
}