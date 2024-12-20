#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPosition, 1.0));  // 월드 좌표계에서의 위치
    Normal = mat3(transpose(inverse(model))) * aNormal;  // 노멀 변환
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
