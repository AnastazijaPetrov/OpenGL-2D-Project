#include "TextRenderer.h"

std::map<char, Character> Characters;
unsigned int textVAO, textVBO;
int textColorLoc;

void initText(const char* fontPath, unsigned int textShader) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "Init free type failed" << std::endl;
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        std::cout << "Font load failed" << std::endl;
        return;
    }
    FT_Set_Pixel_Sizes(face, 0, 96);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            (int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows,
            face->glyph->bitmap_left, face->glyph->bitmap_top,
            (unsigned int)face->glyph->advance.x
        };
        Characters[c] = character;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glUseProgram(textShader);
    glUniform1i(glGetUniformLocation(textShader, "uTex0"), 0);
    textColorLoc = glGetUniformLocation(textShader, "textColor");

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderText(unsigned int shader, std::string text, float x, float y, float scale, float r, float g, float b, float screenWidth, float screenHeight) {
    glUseProgram(shader);
    glUniform3f(textColorLoc, r, g, b);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    float halfW = screenWidth / 2.0f;
    float halfH = screenHeight / 2.0f;

    for (char c : text) {
        Character ch = Characters[c];

        float xpos = x + (ch.BearingX * scale) / halfW;
        float ypos = y - ((ch.SizeY - ch.BearingY) * scale) / halfH;
        float w = (ch.SizeX * scale) / halfW;
        float h = (ch.SizeY * scale) / halfH;

        float vertices[4][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        x += (ch.Advance >> 6) * scale / halfW;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTextCentered(unsigned int shader, std::string text, float centerX, float centerY, float scale, float r, float g, float b, float screenWidth, float screenHeight) {
    float halfW = screenWidth / 2.0f;
    float halfH = screenHeight / 2.0f;

    float totalWidth = 0.0f;
    float maxHeight = 0.0f;

    for (char c : text) {
        Character ch = Characters[c];
        totalWidth += (ch.Advance >> 6) * scale / halfW;
        float h = (ch.BearingY * scale) / halfH;
        if (h > maxHeight) maxHeight = h;
    }

    float startX = centerX - (totalWidth / 2.0f);
    float startY = centerY - (maxHeight / 2.0f);

    RenderText(shader, text, startX, startY, scale, r, g, b, screenWidth, screenHeight);
}