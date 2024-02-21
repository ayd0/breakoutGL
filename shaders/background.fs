#version 330 core
in vec2 TexCoords;
in vec4 FragPos;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform vec2 ballPos;
uniform float aspect;
uniform bool shadow;

void main() {
    vec2 scaledFrag = vec2(FragPos.x, FragPos.y / aspect);
    vec2 scaledBall = vec2(ballPos.x, ballPos.y / aspect);

    float dist = distance(scaledFrag, scaledBall);
    if (shadow) {
        if (dist <= 0.05) {
            color = vec4(spriteColor, 0.6) * texture(image, TexCoords);
        } else {
            color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
        }
    }
    else {
        color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
    }
}
