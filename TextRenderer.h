#pragma once
#include <GL/glew.h>
#include <iostream>
#include <map>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID;
    int SizeX, SizeY;
    int BearingX, BearingY;
    unsigned int Advance;
};

void initText(const char* fontPath, unsigned int textShader);
void RenderText(unsigned int shader, std::string text, float x, float y, float scale, float r, float g, float b, float screenWidth, float screenHeight);
void RenderTextCentered(unsigned int shader, std::string text, float centerX, float centerY, float scale, float r, float g, float b, float screenWidth, float screenHeight);