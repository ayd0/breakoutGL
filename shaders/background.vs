#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec4 FragPos;

uniform mat4 model;
uniform mat4 projection;

void main() {
    TexCoords = vertex.zw;
    FragPos = vec4(vertex.xy, 0.0, 1.0); 
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
