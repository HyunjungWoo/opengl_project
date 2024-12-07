#include "RotateObject.h"

#include <iostream>

RotateObject::RotateObject()
{
	rotateSpeed = (rand() % 2 + 4) * 45.f;	// 45 ~ 180도
}

RotateObject::~RotateObject()
{
}

void RotateObject::initilize()
{
}

void RotateObject::update(float elapsedTime)
{
	rotateY(rotateSpeed * elapsedTime);		// 초당 rotateSpeed만큼 회전
}

void RotateObject::draw() const
{
	// 모델변환행렬
	GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
	if (modelLoc < 0)
		std::cout << "modelLoc 찾지 못함\n";
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(worldTransform));

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void RotateObject::release()
{
}
