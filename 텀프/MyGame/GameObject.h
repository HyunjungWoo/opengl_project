#pragma once

#include "library.h"
#include <iostream>

class GameObject
{
protected:		// 부모 클래스가 될 것이다..
	glm::mat4 worldTransform;

	GLuint shader;
	GLuint VAO;
	GLsizei vertexCount;

public:
	GameObject();
	virtual ~GameObject();

	virtual void initilize();
	virtual void update(float elapsedTime);
	virtual void draw() const;
	virtual void release();

	void setShader(GLuint shader);
	void setVAO(GLuint vao, GLsizei count);

	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 position);
	void setPosition(float x, float y, float z);	// 오버로딩

	glm::vec3 getLook() const;
	glm::vec3 getRight() const;

	void rotateX(float degrees);
	void rotateY(float degrees);

	void move(glm::vec3 dir, float value);
	void moveForward(float value);

	//// 충돌 관련 메서드
	virtual bool checkCollision(const GameObject& other) const = 0; // 충돌 여부 판단
	virtual void visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const = 0;                // 충돌 박스 시각화

	virtual void onCollision(GameObject* other) {
		/*std::cout << "충돌 발생: " << this << " 와 " << other << std::endl;
		std::cout << "삭제" << std::endl;*/
		setCollision(true);
	}

	// 삭제 관련
	bool getDie() { 
		if(this!=nullptr) return isdie;
	}
	bool getCollision() {
		if (this != nullptr) return isCollision;
	}
	void setDie(bool value) { isdie = value; }
	void setCollision(bool value) { isCollision = value; }

private:
	bool isdie;
	bool isCollision;
};

