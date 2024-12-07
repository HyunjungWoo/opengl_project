#pragma once

#include "GameObject.h"
#include "AABB.h"


class CrushObject : public GameObject
{
	bool	isCrush;
	AABB	boundingBox;       // AABB �浹 ���
	float	radius;  // ������� ������
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

	// �ؽ�ó ID ��ȯ
	GLuint getTextureID() const {
		return textureID;
	}

	void onColiision() {
		isCrush = true;
	}

	const AABB& getAABB() const {
		return boundingBox;
	}

	// AABB ������Ʈ
	void updateAABB();

	// �浹 ó��
	virtual bool checkCollision(const GameObject& other) const override;

	// �浹 �ڽ� �ð�ȭ
	virtual void visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const override;

private:
	GLuint textureID;   // ��ü�� �ؽ�ó ID
};

