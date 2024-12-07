#version 330 core

const float halfSize = 10.f;	// 한 변의 길이는 20

uniform mat4 viewTransform;
uniform mat4 projTransform;

void main()
{
	vec3 pos[6];

	pos[0] = vec3(halfSize, 0.f, -halfSize);
	pos[1] = vec3(-halfSize, 0.f, -halfSize);
	pos[2] = vec3(-halfSize, 0.f, halfSize);
	pos[3] = vec3(-halfSize, 0.f, halfSize);
	pos[4] = vec3(halfSize, 0.f, halfSize);
	pos[5] = vec3(halfSize, 0.f, -halfSize);

	gl_Position = projTransform * viewTransform * vec4(pos[gl_VertexID], 1.f);
}
