#pragma once

#include "GameObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

struct Vertex {
	float position[3];
	float normal[3];
	float texCoords[2];
};


class PlayerObject : public GameObject
{
	bool isWPressed;
	bool isAPressed;
	bool isSPressed;
	bool isDPressed;
	bool isEPressed;
	bool isQPressed;
	bool isSpacePressed;

	float moveSpeed;

	bool isLeftMousePressed;
	int befMousePosX;
	int befMousePosY;

	float radius; // Sphere �浹 �ݰ�

	float jumpSpeed = 5.0f;         // ���� �ʱ� �ӵ�
	float gravity = -9.8f;          // �߷� ���ӵ�
	glm::vec3 velocity;             // �÷��̾��� �ӵ�
	bool isJumping;                 // ���� ������ Ȯ��
	bool isOnGround;              // ���� ���� �ִ��� Ȯ��


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
	virtual void visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const override;                // �浹 �ڽ� �ð�ȭ


	float getRadius() const; // ������ ��ȯ
	glm::vec3 getCenter() const; // ���� �߽� ��ȯ

	void jump();

private:

	std::vector<Vertex> vertices;
	GLuint textureID;
	GLuint FbxVAO, FbxVBO;
	void processMesh(aiMesh* mesh);
	void loadTexture(const std::string& path);
	void loadModel(const std::string& path);



};

