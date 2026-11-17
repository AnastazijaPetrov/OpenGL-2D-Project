#version 330 core

in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex0;
uniform vec3 textColor;

void main()
{
    outCol = vec4(textColor, texture(uTex0, chTex).r);
}