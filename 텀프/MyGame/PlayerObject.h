#pragma once

#include "GameObject.h"

class PlayerObject : public GameObject
{
	bool isWPressed;
	bool isAPressed;
	bool isSPressed;
	bool isDPressed;
	bool isEPressed;
	bool isQPressed;

	float moveSpeed;

	bool isLeftMousePressed;
	int befMousePosX;
	int befMousePosY;

	float radius; // Sphere 충돌 반경

public:
	PlayerObject();
	virtual ~PlayerObject();

	virtual void initilize() override;
	virtual void update(float elapsedTime) override;
	virtual void draw() const override;
	virtual void release() override;

	void keyboard(unsigned char key, bool isPressed);
	void mouse(int button, int state, int x, int y);
	void mouseMove(int x, int y);

	virtual bool checkCollision(const GameObject& other) const override;
	virtual void visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const override;                // 충돌 박스 시각화


	float getRadius() const; // 반지름 반환
	glm::vec3 getCenter() const; // 구의 중심 반환


private:



};

