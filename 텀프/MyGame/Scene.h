#pragma once

#include "library.h"

#include <string>
#include <vector>
#include <memory>
#include <chrono>

class GameObject;
class PlayerObject;

class Scene
{
private:
	GLuint shader;
	GLuint plainShader;
	GLuint texShader;
	GLuint fbxShader;

	GLuint sphereVAO;
	int sphereVertexCount;

	GLuint crownVAO;
	int crownVertexCount;

	GLuint hexagonVAO;
	int hexagonVertexCount;
	GLuint hexagonTexture[6];
	GLuint playerTexture;

	GLuint bgTexture[3];
	GLuint bgVAO, bgVBO, bgEBO;



	std::vector<std::unique_ptr<GameObject>> objects;
	PlayerObject* player;
	
	int objectCount;
	int width;
	int height;

	bool isdebug;
	bool isPaused;
	bool isStartScreen = true; 
	bool isGameOver;
	bool isClear;

	float crownSpawnTimer = 0.0f; // ��� �ð� ����
	bool isCrownPlaced = false;   // �հ��� ��ġ�Ǿ����� ����

public:
	// ���� ��������, update, draw..., Ű �Է�
	Scene(int winWidth, int winHeight);
	~Scene();

	void initialize();
	void release();

	void update(float elapsedTime);

	void draw() const;

	void keyboard(unsigned char key, bool isPressed);
	void specialKeyboard(int key, bool isPressed);
	void mouse(int button, int state, int x, int y);
	void mouseMove(int x, int y);

	void setWindowSize(int winWidth, int winHeight);
	
	void initbg();
	void Drawbg() const;


private:
	static void initBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename);
	static void initCubeBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename);

	static void initBufferWithUV(GLuint* VAO, GLsizei* vertexCount, std::string objFilename);
	static void initTexture(GLuint* texture, int size, std::string* texFilename);
public:
	static GLuint makeShader(std::string vertexFilename, std::string fragmentFilename);
	static std::string readFile(std::string filename);
	static std::vector<glm::vec3> readOBJ(std::string filename);
	static std::vector<glm::vec3> readOBJWithUV(std::string filename);

};

