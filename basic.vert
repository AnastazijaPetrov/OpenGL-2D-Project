#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;

out vec2 chTex;

uniform vec2 uPos;
uniform vec2 uS;

void main()
{
    gl_Position = vec4(inPos.x * uS.x + uPos.x, inPos.y * uS.y + uPos.y, 0.0, 1.0);
    chTex = inTex;
}