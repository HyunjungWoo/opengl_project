
#include "PlayerObject.h"
#include "CrushObject.h"
#include "AABB.h"

#include <iostream>
#include <cmath>
#include <vector>


PlayerObject::PlayerObject()
{
	isWPressed = false;
	isAPressed = false;
	isSPressed = false;
	isDPressed = false;
	isQPressed = false;
	isEPressed = false;

	moveSpeed = 5.f;

	initilize();
	
}

PlayerObject::~PlayerObject()
{

}

void PlayerObject::initilize()
{
	radius = 1.0f;


}

void PlayerObject::update(float elapsedTime)
{
	glm::vec3 dir(0.f);
	if (isWPressed)
		dir += getLook();
	if (isAPressed)
		dir += getRight();
	if (isSPressed)
		dir -= getLook();
	if (isDPressed)
		dir -= getRight();
	if (isEPressed)
		dir.y -= 0.1f;
	if (isQPressed)
		dir.y += 0.1f;


	if (glm::length(dir) >= glm::epsilon<float>())	// 0보다 커야한다
		move(dir, moveSpeed * elapsedTime);

	
}

void PlayerObject::draw() const
{
	

}

void PlayerObject::release()
{
}

void PlayerObject::keyboard(unsigned char key, bool isPressed)
{
	if (isPressed) {			// 눌러졌을 때
		switch (key) {
		case 'W':
		case 'w':
			isWPressed = true;
			break;
		case 'A':
		case 'a':
			isAPressed = true;
			break;
		case 'S':
		case 's':
			isSPressed = true;
			break;
		case 'D':
		case 'd':
			isDPressed = true;
			break;
		case 'q':
		case 'Q':
			isQPressed = true;
			
			break;
		case 'E':
		case 'e':
			isEPressed = true;
			break;
		}
	}
	else {						// 떼졌을 때
		switch (key) {
		case 'W':
		case 'w':
			isWPressed = false;
			break;
		case 'A':
		case 'a':
			isAPressed = false;
			break;
		case 'S':
		case 's':
			isSPressed = false;
			break;
		case 'D':
		case 'd':
			isDPressed = false;
			break;
		case 'q':
		case 'Q':
			isQPressed = false;
			break;
		case 'E':
		case 'e':
			isEPressed = false;
			break;
		}
	}
}

void PlayerObject::mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			isLeftMousePressed = true;
			befMousePosX = x;
			befMousePosY = y;
		}
		else if (state == GLUT_UP)
			isLeftMousePressed = false;
	}
}

void PlayerObject::mouseMove(int x, int y)
{
	int moveXValue = x - befMousePosX; // x축이동량
	int moveYValue = y - befMousePosY; // y축 이동량

	//if (moveXValue > 0) //  양수면 시계...

	// 좌우 회전 (Yaw)
	rotateY(float(-moveXValue / 3.f)); 

	// 상하 회전 (Pitch)
	static float currentPitch = 0.0f; // 현재 Pitch 각도 누적
	float pitchAngle = float(-moveYValue / 3.0f);
	currentPitch -= pitchAngle;

	// Pitch 각도 제한: -89도 ~ 89도
	if (currentPitch > 89.0f) {
		pitchAngle -= (currentPitch - 89.0f);
		currentPitch = 89.0f;
	}
	else if (currentPitch < -89.0f) {
		pitchAngle -= (currentPitch + 89.0f);
		currentPitch = -89.0f;
	}

	rotateX(pitchAngle); // 제한된 각도로 Pitch 회전

	befMousePosX = x;
	befMousePosY = y;
}

bool PlayerObject::checkCollision(const GameObject& other) const
{
	const CrushObject* crushObj = dynamic_cast<const CrushObject*>(&other);
	if (crushObj) {
		// CrushObject와의 충돌 검사
		const AABB& aabb = crushObj->getAABB();
		glm::vec3 sphereCenter = glm::vec3(getPosition().x, getPosition().y, getPosition().z);  // getPosition()으로 수정


		float sphereRadius = getRadius();

		// AABB-구체 충돌 검사
		glm::vec3 closestPoint = glm::clamp(sphereCenter, aabb.getMinBounds(), aabb.getMaxBounds());
		float distanceSquared = glm::distance2(sphereCenter, closestPoint);


		return distanceSquared <= (sphereRadius * sphereRadius);


	}

	

	std::cerr << "Unsupported collision type for PlayerObject.\n";
	return false;
}

void PlayerObject::visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const
{

	// OpenGL 버퍼 설정
	 // 라인 드로잉
	glUseProgram(shader); // 라인 쉐이더 활성화

	GLint viewLoc = glGetUniformLocation(shader, "viewTransform");
	if (viewLoc < 0)
		std::cout << "viewLoc 찾지 못함\n";

	GLint projLoc = glGetUniformLocation(shader, "projTransform");
	if (projLoc < 0)
		std::cout << "projLoc 찾지 못함\n";

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

	// 모델 변환 행렬을 셰이더에 전달 (World Transform 사용)
	GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
	if (modelLoc >= 0) {
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(worldTransform)); // 월드 변환
	}

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, vertexCount); // 선을 그리기
	
}



float PlayerObject::getRadius() const
{
	return radius;
}

glm::vec3 PlayerObject::getCenter() const
{
	return glm::vec3(worldTransform[3][0], worldTransform[3][1], worldTransform[3][2]);
}


