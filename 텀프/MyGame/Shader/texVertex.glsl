#version 330 core

layout (location = 0) in vec3 inPos;		// 응용 프로그램에서 받아온 도형 좌표값
layout (location = 1) in vec3 inTexcoord;	// 점의 텍스처 좌표
layout (location = 2) in vec3 inNormal;		// 점의 법선벡터

out vec3 outTexcoord;
out vec3 outFragPos;
out vec3 outNormal;

uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projTransform;

void main()
{
	outTexcoord = inTexcoord;
	outFragPos = vec3(modelTransform * vec4(inPos, 1.f));
	outNormal = mat3(modelTransform) * inNormal;	// x, y, z축에 대하여 같은 크기의 신축
	// outNormal = transpose(inverse(mat3(modelTransform))) * inNormal;		// x, y, z축에 대하여 다른 크기의 신축
	// normal은 이동을 하면 안된다!		// x, y, z축에 대하여 균등 축소/확대라면 직교행렬..-> inverse == transpose

	gl_Position = projTransform * viewTransform * vec4(outFragPos, 1.f);
}
