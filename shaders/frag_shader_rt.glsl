#version 430 core

in vec2 fragTexCoord;
out vec4 color;

uniform sampler2D renderedImage;

void main() {
    color = texture(renderedImage, fragTexCoord);
}
