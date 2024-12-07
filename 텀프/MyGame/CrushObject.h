#pragma once

#include "GameObject.h"
#include "AABB.h"


class CrushObject : public GameObject
{
	bool	isCrush;
	AABB	boundingBox;       // AABB 충돌 경계
	float	radius;  // 육각기둥 반지름
	float	height;
public:

	CrushObject();
	virtual ~CrushObject();

	virtual void initilize() override;
	virtual void update(float elapsedTime) override;
	virtual void draw() const override;
	virtual void release() override;

	void setTexture(GLuint textureID) {
		this->textureID = textureID;
	}

	// 텍스처 ID 반환
	GLuint getTextureID() const {
		return textureID;
	}

	void onColiision() {
		isCrush = true;
	}

	const AABB& getAABB() const {
		return boundingBox;
	}

	// AABB 업데이트
	void updateAABB();

	// 충돌 처리
	virtual bool checkCollision(const GameObject& other) const override;

	// 충돌 박스 시각화
	virtual void visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const override;

private:
	GLuint textureID;   // 객체별 텍스처 ID
};

