#include "Scene.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <algorithm>
#include <numeric>

#include "RotateObject.h"
#include "PlayerObject.h"
#include "CrushObject.h"

#define STB_IMAGE_IMPLEMENTATION		// �� �ϳ��� .cpp ���� define ����� �Ѵ�.. �ߺ� include ����!
#include <stb_image.h>


Scene::Scene(int winWidth, int winHeight)
	: width{ winWidth }, height{ winHeight }
{
	isdebug = false;
	isPaused = false;
	isStartScreen = true;
	isClear = false;
	isGameOver = false;

	bgVAO, bgVBO, bgEBO = {};
}

Scene::~Scene()
{
	release();
}

void Scene::initialize()
{
	shader = makeShader("./Shader/vertex.glsl", "./Shader/fragment.glsl");
	plainShader = makeShader("./Shader/plainVert.glsl", "./Shader/plainFrag.glsl");
	texShader = makeShader("./Shader/texVertex.glsl", "./Shader/texFrag.glsl");
	fbxShader = makeShader("./Shader/vertex_shader.glsl", "./Shader/fragment_shader.glsl");


	initBuffer(&sphereVAO, &sphereVertexCount, "./OBJ/sphere.obj");
	initBuffer(&crownVAO, &crownVertexCount, "./OBJ/crown.obj");
	initBufferWithUV(&hexagonVAO, &hexagonVertexCount, "./OBJ/hexagon2.obj");

	std::string Filename[6] = { "./Image/test2.png","./Image/test5.png","./Image/test.png","./Image/test3.png" ,"./Image/test4.png", "./Image/crown.png" };
	initTexture(hexagonTexture, 6, Filename);

	std::string playerFilename = "./Image/1.png";
	initTexture(&playerTexture, 1, &playerFilename);

	std::string bgFilename[3] = { "./Image/mainScreen.png", "./Image/Gameover.png", "./Image/GameClear.png"};
	initTexture(bgTexture, 3, bgFilename);


	player = new PlayerObject;

	player->setVAO(sphereVAO, sphereVertexCount);
	player->setShader(fbxShader);
	player->rotateY(180.f);

	player->setPosition(0.f, 12.0f, 5.f);
	//std::cout << "playerPosition : " << player->getPosition().x << ", " << player->getPosition().y << ", " << player->getPosition().z;

	initbg();

	std::vector<int> rowCounts = { 5, 6, 7, 8, 9, 8, 7, 6, 5 }; // �� ���� ������ ����
	float radius = 1.0f; // ������ ���� �߽� �Ÿ� (������)
	float startZ = -5.f;  // z ���� ������
	int index = 0;       // ������Ʈ �ε��� �ʱ�ȭ
	float heightOffset = 5.f * radius; // �� ���� y ��ǥ ����

	int layers = 3; // ���� ���� (���� + ���� 2�� �߰�)


	for (int layer = 0; layer < layers; ++layer) { // �� �ݺ�
		for (size_t row = 0; row < rowCounts.size(); ++row) {
			int hexCount = rowCounts[row];                         // ���� ���� ������ ����
			float startX = -(hexCount - 1) * 1.75f * radius / 2.0f; // ���� �߾� ���� x ��ǥ

			for (int col = 0; col < hexCount; ++col) {
				float x = startX + col * 1.75f * radius; // x ��ǥ ���
				float z = startZ;                      // z ��ǥ�� �� ������ ����
				float y = layer * heightOffset;        // ���� y ��ǥ ���

				// ���� �ؽ�ó �迭 ����
				int textureIndex = 0; // �⺻��
				if (layer % 3 == 0) { // 1��
					textureIndex = (row + col) % 2; // 0, 1 �ݺ�
				}
				else if (layer % 3 == 1) { // 2��
					textureIndex = 2 + (row + col) % 2; // 2, 3 �ݺ�
				}
				else if (layer % 3 == 2) { // 3��
					textureIndex = (row + col) % 2 == 0 ? 3 : 1; // 3, 1 �ݺ�
				}

				// ������ ���� �� ����
				auto obj = std::make_unique<CrushObject>();
				obj->setShader(texShader);
				obj->setVAO(hexagonVAO, hexagonVertexCount);
				obj->setPosition(x, y, z); // y ��ǥ �����Ͽ� ����

				obj->initilize();
				// �ؽ�ó ����
				obj->setTexture(textureIndex); // ���õ� �ؽ�ó�� ��ü�� ����

				// ��ü�� ���Ϳ� �߰�
				objects.push_back(std::move(obj));
			}

			startZ += sqrt(2.3) * radius; // ���� ���� z ��ǥ�� �̵�
		}
		startZ = -5.f; // �� ������ z ��ǥ�� �ʱ�ȭ
	}

	objectCount = objects.size(); // ��ü ������Ʈ ����
}

void Scene::release()
{
	delete player;
}

void Scene::update(float elapsedTime)
{
	if (isStartScreen || isPaused || isClear || isGameOver) { return; }
	// ��� �ð� ����
    crownSpawnTimer += elapsedTime;
	//std::cout << crownSpawnTimer << "\n";

    // 30�ʰ� ������ �հ��� ���� ��ġ���� �ʾ��� ��
    if (crownSpawnTimer >= 30.0f && !isCrownPlaced) {
        isCrownPlaced = true;

        // �հ� ������Ʈ ����
        auto crown = std::make_unique<CrushObject>();
        crown->setShader(texShader); // �հ��� �ؽ�ó ���̴� ���
        crown->setVAO(crownVAO, crownVertexCount); // �հ� VAO�� Vertex ����
        crown->setPosition(0.875, 2.2, 0.f); // �հ� ��ġ ����
        crown->initilize();

        // �հ� �ؽ�ó ���� (�ʿ��ϸ� ����)
        crown->setTexture(3); // �հ��� ������ �ؽ�ó ID ����

        // �հ��� ������Ʈ ��Ͽ� �߰�
        objects.push_back(std::move(crown));
    }

	player->update(elapsedTime);
	if (player->getPosition().y <= -10.f)
	{
		std::cout << "y: " << player->getPosition().y << "\n";
		isGameOver = true;
	}
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] && !objects[i]->getDie() && objects[i]->checkCollision(*player)) {
			player->onCollision(objects[i].get());
			objects[i]->onCollision(player);
		}
	}

	// ������Ʈ �� ���� ó��
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] && objects[i]->getDie()) {
			objects[i].reset();  // ��ü ����
			--objectCount;
		}
		else if (objects[i]) {
			objects[i]->update(elapsedTime);
		}
	}

	if (!objects.empty()) {
		// ������ ������ ��ü�� Ȯ��
		auto& lastObj = objects.back();

		if (auto* crownObj = dynamic_cast<CrushObject*>(lastObj.get())) {
			// �հ� ��ü�� ���, getDie()�� true�� ��� ������ ������ �Ǵ�
			if (crownObj->getDie()) {
				std::cout << "�հ��� �����Ǿ����ϴ�." << std::endl;
				isClear = true;  // ���� Ŭ���� ���·� ����
			}
		}
	}

}

void Scene::draw() const
{
	if (isPaused) {
		
	}
	else if (isStartScreen || isClear || isGameOver)
	{
		Drawbg();
	}

	//glm::vec3 cameraPos = player->getPosition() + glm::vec3(0, 0, +3);		// �÷��̾� ��ġ����
	//cameraPos.y = 5.f;
	//glm::vec3 targetPos = cameraPos + player->getLook();// �÷��̾� ���� �ٶ󺻴�

	 // �÷��̾��� ���� ��ġ
	glm::vec3 playerPos = player->getPosition();

	// ī�޶��� ��ġ: �÷��̾� ��ġ ����(Z �� -3)���� �̵��ϰ� Y ������ ����
	glm::vec3 cameraOffset(0.0f, 3.0f, 4.0f); // Y��: ����, Z��: �Ÿ�
	glm::vec3 cameraPos = playerPos - player->getLook() * cameraOffset.z; // �������� ī�޶� ��ġ
	cameraPos.y = playerPos.y + cameraOffset.y; // �������� ī�޶� ���� ����

	// ī�޶� �ٶ� ��ǥ ����: �÷��̾� ��ġ
	glm::vec3 targetPos = playerPos;

	// ���� ��ǥ���� "����" ����
	glm::vec3 up(0.0f, 1.0f, 0.0f);


	glm::mat4 viewMatrix = glm::lookAt(cameraPos, targetPos, up);
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 100.f);

	

	{	// �ٴ��� ����ش�
		// ī�޶�, ������ �� ��ü�� ����..
		glUseProgram(plainShader);

		GLint viewLoc = glGetUniformLocation(plainShader, "viewTransform");
		if (viewLoc < 0)
			std::cout << "viewLoc ã�� ����\n";
		GLint projLoc = glGetUniformLocation(plainShader, "projTransform");
		if (projLoc < 0)
			std::cout << "projLoc ã�� ����\n";

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 6);

	}

	{
		glUseProgram(fbxShader);

		// ī�޶�, ������ �� ��ü�� ����..
		GLint viewLoc = glGetUniformLocation(fbxShader, "view");
		if (viewLoc < 0)
			std::cout << "viewLoc ã�� ����\n";
		GLint projLoc = glGetUniformLocation(fbxShader, "projection");
		if (projLoc < 0)
			std::cout << "projLoc ã�� ����\n";
		GLint cameraPosLoc = glGetUniformLocation(texShader, "cameraPos");
		if (cameraPosLoc < 0)
			std::cout << "cameraPosLoc ã�� ����\n";

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
		glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

		glUseProgram(fbxShader);

		glBindTexture(GL_TEXTURE_2D, playerTexture); // playerTexture ���ε�

		player->draw();		// �ȱ׸��� �ص�... ���߿� �׸� �� ������ ȣ�����ش�
	}

	glUseProgram(shader);

	if (isdebug) player->visualizeCollisionBox(viewMatrix, projMatrix);
	// ������Ʈ �׸���

	glUseProgram(texShader);

	// ī�޶�, ������ �� ��ü�� ����..
	GLint viewLoc = glGetUniformLocation(texShader, "viewTransform");
	if (viewLoc < 0)
		std::cout << "viewLoc ã�� ����\n";
	GLint projLoc = glGetUniformLocation(texShader, "projTransform");
	if (projLoc < 0)
		std::cout << "projLoc ã�� ����\n";
	GLint cameraPosLoc = glGetUniformLocation(texShader, "cameraPos");
	if (cameraPosLoc < 0)
		std::cout << "cameraPosLoc ã�� ����\n";

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	// OBJ With Texture �׷��ֱ�
	glUseProgram(texShader);
	
	// ���� �� ���� (0.0 ~ 1.0 ���� ��)
	float alphaValue = 1.f; // 50% ����


	for (const auto& obj : objects) {
		if (CrushObject* crushObj = dynamic_cast<CrushObject*>(obj.get())) {
			// CrushObject�� ĳ���� ����
			GLuint textureID = crushObj->getTextureID();

			// �ؽ�ó ���ε�
			if (textureID == 0) {
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[0]);
			}
			else if(textureID == 1){
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[1]);
			}
			else if (textureID == 2) {
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[2]);
			}
			else if (textureID == 3) {
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[3]);
			}
			else if (textureID == 4) {
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[4]);
			}
			else if (textureID == 5) {
				glBindTexture(GL_TEXTURE_2D, hexagonTexture[5]);
			}
			// Draw ȣ��
			if (obj->getDie()) { /*std::cout << "�׸�����\n";*/ }
			else{
				if (obj->getCollision()) {
					alphaValue = 4.f;
					glUniform1f(glGetUniformLocation(texShader, "brightness"), alphaValue);
				}
				else
				{
					alphaValue = 1.5f;
					glUniform1f(glGetUniformLocation(texShader, "brightness"), alphaValue);

				}
				crushObj->draw();
				if(isdebug) crushObj->visualizeCollisionBox(viewMatrix, projMatrix); // AABB�� �ð�ȭ
			}
		}
		else {
			// �ٸ� Ÿ���� GameObject�� ��� ó�� (�ʿ� ��)
			//std::cerr << "Non-CrushObject encountered in texObjects.\n";
		}
	}
}

void Scene::keyboard(unsigned char key, bool isPressed)
{
	player->keyboard(key, isPressed);

	if (isPressed) {			// �������� ��
		switch (key) {
		case 'M':
		case 'm':
			isdebug = !isdebug;
			break;
		case 'p':
		case 'P':
			isPaused = !isPaused;
			break;
		case ' ':
			if (isStartScreen) {
				isStartScreen = false;  // �����̽��ٷ� ���� ����
			}
			break;
		case 'r':
			isGameOver = false;
			isClear = false;
			release();
			// ���� �ʱ�ȭ
			initialize();
			break;
		default:
			break;
		}
	}
	else {						// ������ ��
		switch (key) {
		default:
			break;
		}
	}
}

void Scene::specialKeyboard(int key, bool isPressed)
{
}

void Scene::mouse(int button, int state, int x, int y)
{
	player->mouse(button, state, x, y);

	// ȭ�� ������Ʈ�� �ȴ�....

	constexpr int WHEEL_UP = 3;
	constexpr int WHEEL_DOWN = 4;

	switch (state) {
	case GLUT_DOWN:
		switch (button) {
		case GLUT_LEFT_BUTTON: {
			float xPos = (static_cast<float>(x) / width * 2.f - 1.f);
			float yPos = -(static_cast<float>(y) / height * 2.f - 1.f);

			//std::cout << "��Ŭ�� : " << x << ", " << y << std::endl;
			//std::cout << "OpenGL x ��ǥ�� " << xPos << std::endl;
			//std::cout << "OpenGL y ��ǥ�� " << yPos << std::endl;

			break;
		}
		case GLUT_MIDDLE_BUTTON:
			std::cout << "��Ŭ�� : " << x << ", " << y << std::endl;
			break;
		case GLUT_RIGHT_BUTTON:
			std::cout << "��Ŭ�� : " << x << ", " << y << std::endl;
			break;
		case WHEEL_UP:
			std::cout << "��  �� : " << x << ", " << y << std::endl;
			break;
		case WHEEL_DOWN:
			std::cout << "�ٴٿ� : " << x << ", " << y << std::endl;
			break;
		}
		break;
	case GLUT_UP:
		switch (button) {
		case GLUT_LEFT_BUTTON:
			break;
		case GLUT_RIGHT_BUTTON:
			break;
		case GLUT_MIDDLE_BUTTON:
			break;
		}
		break;
	}
}

void Scene::mouseMove(int x, int y)
{
	player->mouseMove(x, y);
}

void Scene::setWindowSize(int winWidth, int winHeight)
{
	width = winWidth;
	height = winHeight;
}

void Scene::initbg()
{
	// ���� ������: ��ġ (x, y, z), �ؽ�ó ��ǥ (u, v, w), ���� ���� (nx, ny, nz)
	float vertices[] = {
		//--- ��ġ (x, y, z) ---   //--- �ؽ�ó ��ǥ (u, v, w) ---   //--- ���� ���� (nx, ny, nz) ---
		-1.0f, -1.0f, 0.0f,        0.0f, 0.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Bottom-left
	 1.0f, -1.0f, 0.0f,        1.0f, 0.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Bottom-right
	 1.0f,  1.0f, 0.0f,        1.0f, 1.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Top-right
	-1.0f,  1.0f, 0.0f,        0.0f, 1.0f, 0.0f,                0.0f, 0.0f, 1.0f  // Top-left
	};

	// �ε��� ������: �簢���� �� ���� �ﰢ������ ����
	unsigned int indices[] = {
		0, 1, 2, // �ﰢ�� 1
		0, 2, 3  // �ﰢ�� 2
	};

	// VAO, VBO, EBO ����

	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);
	glGenBuffers(1, &bgEBO);

	// VAO ���ε�
	glBindVertexArray(bgVAO);

	// VBO ����
	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// EBO ����
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// ��ġ �Ӽ� (layout = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// �ؽ�ó ��ǥ �Ӽ� (layout = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// ���� ���� �Ӽ� (layout = 2)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// VAO ���ε� ����
	glBindVertexArray(0);

}

void Scene::Drawbg() const
{
	// �ؽ�ó Ȱ��ȭ �� ���ε�
	glActiveTexture(GL_TEXTURE0);

	if(isStartScreen)
		glBindTexture(GL_TEXTURE_2D, bgTexture[0]); // yourTextureID�� ������ �ؽ�ó ID
	else if (isGameOver) 
		glBindTexture(GL_TEXTURE_2D, bgTexture[1]); // yourTextureID�� ������ �ؽ�ó ID
	else if (isClear)
		glBindTexture(GL_TEXTURE_2D, bgTexture[2]); // yourTextureID�� ������ �ؽ�ó ID


	// ���̴� Ȱ��ȭ
	glUseProgram(texShader); // yourShaderProgram�� ���̴� ���α׷� ID

	// ������ ����
	glm::mat4 modelMatrix = glm::mat4(1.0f); // �⺻ ��ȯ ����
	glm::mat4 viewMatrix = glm::mat4(1.0f);  // �� ��ȯ ����
	glm::mat4 projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f); // ���� ����

	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "viewTransform"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projTransform"), 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniform1f(glGetUniformLocation(texShader, "brightness"), 10.0f);

	// VAO ���ε�
	if (!bgVAO || !bgVBO || !bgEBO) {
		std::cerr << "bgVAO, bgVBO, �Ǵ� bgEBO�� �������� �ʾҽ��ϴ�!" << std::endl;
		return;
	}
	glBindVertexArray(bgVAO);

	// �׸���
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// VAO ���ε� ����
	glBindVertexArray(0);

}



void Scene::initBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// ���� �Ҵ�....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	std::vector<glm::vec3> verticies = readOBJ(objFilename);

	*vertexCount = verticies.size() / 3;

	// GPU�� �ϳ��� ��ǻ�Ϳ� ����... ���ο� gpu���� cpu, ��, ��� ����...

	// CPU �޸𸮿� �ִ� �����͸�, GPU �޸𸮿� ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.size(), verticies.data(), GL_STATIC_DRAW);

	// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(0);

	//// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(1);

	//// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(2);

}

void Scene::initCubeBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// ���� �Ҵ�....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// ������ü...
	// ������ü�� ���� 6���̰�, ���� 8���̰�, ���� 12��..
	// ���� float�� �ּ� ����(x, y, z)
	// �ﰢ�� �ϳ��� �� 3��
	// ��� �ﰢ���� 2��
	// ����ü �ϳ��� ���� 6��
	// 6 * 2 * 3 * 3 --> 108��

	*vertexCount = 36;

	float size = 0.5f;
	float vertexData[6 * 2 * 3 * 9]{// 6��, ��� 2�ﰢ��, �ﰢ���� ��3��, ���� 9���� --> 324
		// px, py, pz, tx, ty, tz=0, nx, ny, nz

		//Front
		-size, -size, size,		0, 0, 0,	1, 0, 0,
		size, -size, size,		1, 0, 0,	1, 0, 0,
		size, size, size,		1, 1, 0,	1, 0, 0,
		size, size, size,		1, 1, 0,	1, 0, 0,
		-size, size, size,		0, 1, 0,	1, 0, 0,
		-size, -size, size,		0, 0, 0,	1, 0, 0,
		//Back
		-size, -size, -size,	1, 0, 0,	0, 1, 0,
		-size, size, -size,		1, 1, 0,	0, 1, 0,
		size, size, -size,		0, 1, 0,	0, 1, 0,
		size, size, -size,		0, 1, 0,	0, 1, 0,
		size, -size, -size,		0, 0, 0,	0, 1, 0,
		-size, -size, -size,	1, 0, 0,	0, 1, 0,
		//Left
		-size, -size, size,		1, 0, 0,	0, 0, 1,
		-size, size, size,		1, 1, 0,	0, 0, 1,
		-size, size, -size,		0, 1, 0,	0, 0, 1,
		-size, size, -size,		0, 1, 0,	0, 0, 1,
		-size, -size, -size,	0, 0, 0,	0, 0, 1,
		-size, -size, size,		1, 0, 0,	0, 0, 1,
		//Right
		size, -size, size,		0, 0, 0,	1, 1, 0,
		size, -size, -size,		1, 0, 0,	1, 1, 0,
		size, size, -size,		1, 1, 0,	1, 1, 0,
		size, size, -size,		1, 1, 0,	1, 1, 0,
		size, size, size,		0, 1, 0,	1, 1, 0,
		size, -size, size,		0, 0, 0,	1, 1, 0,
		//Top
		-size, size, size,		0, 0, 0,	1, 0, 1,
		size, size, size,		1, 0, 0,	1, 0, 1,
		size, size, -size,		1, 1, 0,	1, 0, 1,
		size, size, -size,		1, 1, 0,	1, 0, 1,
		-size, size, -size,		0, 1, 0,	1, 0, 1,
		-size, size, size,		0, 0, 0,	1, 0, 1,
		//Bottom
		-size, -size, size,		0, 0, 0,	0, 1, 1,
		-size, -size, -size,	1, 0, 0,	0, 1, 1,
		size, -size, -size,		1, 1, 0,	0, 1, 1,
		size, -size, -size,		1, 1, 0,	0, 1, 1,
		size, -size, size,		0, 1, 0,	0, 1, 1,
		-size, -size, size,		0, 0, 0,	0, 1, 1
	};

	// CPU �޸𸮿� �ִ� �����͸�, GPU �޸𸮿� ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 324, vertexData, GL_STATIC_DRAW);

	// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(0);

	// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(1);

	// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(2);
}

void Scene::initBufferWithUV(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// ���� �Ҵ�....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	std::vector<glm::vec3> verticies = readOBJWithUV(objFilename);

	*vertexCount = verticies.size() / 3;

	// CPU �޸𸮿� �ִ� �����͸�, GPU �޸𸮿� ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.size(), verticies.data(), GL_STATIC_DRAW);

	// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(0);

	//// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(1);

	//// �� �����Ͱ� � ����������, �츮�� ���Ǹ� �߱� ������, openGL�� �˷���� �Ѵ�!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, ����, Ÿ��, nomalized?, ����(����Ʈ), ���ۿ�����
	glEnableVertexAttribArray(2);
}

void Scene::initTexture(GLuint* texture, int size, std::string* texFilename)
{
	glGenTextures(size, texture);

	// ���� �̹��� ������(binary) --> �츮�� �ƴ� ���� �̹����� ����Ʈ ������ �о �����ؾ� �Ѵ�.
	stbi_set_flip_vertically_on_load(true);

	for (int i = 0; i < size; ++i) {
		glBindTexture(GL_TEXTURE_2D, texture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		int width, height, level;
		unsigned char* data = stbi_load(texFilename[i].c_str(), &width, &height, &level, 0);		// ���� �Ҵ�...

		if (3 == level)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	// GPU�� ������ ����
		else if (4 == level)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else {
			std::cerr << texFilename[i] << " load Error! 3ä�� Ȥ�� 4ä�� �̹����� �־��ּ���. Ȥ�� �̹��� ���� ����\n";
			exit(1);
		}

		stbi_image_free(data);
	}
}

GLuint Scene::makeShader(std::string vertexFilename, std::string fragmentFilename)
{
	// ���̴� ������...

	GLint result;
	GLchar errorLog[512];

	GLuint vertexShader, fragmentShader;

	std::string vertexStr = readFile(vertexFilename);	// �� ������ ����������
	const char* vertexSource{ vertexStr.c_str() };		// �� ������ ����Ƿ��� �� ������ �Ҹ�Ǹ� �ȵȴ�..!

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << vertexFilename << " ERROR: vertex shader error\n" << errorLog << std::endl;
	}
	else {
		std::cout << vertexFilename << " ������ ����!" << std::endl;
	}

	std::string fragmentStr = readFile(fragmentFilename);
	const char* fragmentSource{ fragmentStr.c_str() };

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << fragmentFilename << " ERROR: fragment shader error\n" << errorLog << std::endl;
	}
	else {
		std::cout << fragmentFilename << " ������ ����!" << std::endl;
	}

	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);		// ���� vertex Shader�� ���̴� ���α׷��� �߰��� �ش�
	glAttachShader(shaderID, fragmentShader);	// ���� fragment Shader�� ���̴� ���α׷��� �߰��� �ش�
	glLinkProgram(shaderID);					// �߰��� ���̴����� �ϳ��� ���α׷����� �����Ѵ�!

	glDeleteShader(vertexShader);				// ���� ���̻� vertex Shaer�� �ʿ����...
	glDeleteShader(fragmentShader);				// ���� ���̻� fragment Shaer�� �ʿ����...

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << vertexFilename << ", " << fragmentFilename << " ERROR: shader program ���� ����\n" << errorLog << std::endl;
		//return;
	}
	else {
		std::cout << vertexFilename << ", " << fragmentFilename << " shader program ���� ����!" << std::endl;
	}

	return shaderID;
}

std::string Scene::readFile(std::string filename)
{
	std::ifstream in{ filename };
	if (!in) {
		std::cout << filename << "���� �������� ����!" << std::endl;
		exit(1);
	}

	std::string str;
	std::string temp;
	while (std::getline(in, temp)) {
		str += temp;
		str.append(1, '\n');
	}

	return str;
}

std::vector<glm::vec3> Scene::readOBJ(std::string filename)
{
	std::ifstream in{ filename };
	if (!in) {
		std::cout << filename << " file read failed\n";
		exit(1);
	}

	// c++ stream --> input output�� ���ִ� �帧?

	srand(static_cast<unsigned int>(time(nullptr)));

	std::vector<glm::vec3> vertex;
	std::vector<glm::vec3> color;
	std::vector<glm::vec3> normal;
	std::vector<glm::ivec3> vindex;
	std::vector<glm::ivec3> nindex;
	while (in) {
		std::string line;
		std::getline(in, line);
		std::stringstream ss{ line };
		std::string str;
		ss >> str;
		if (str == "v") {
			glm::vec3 v;
			for (int i = 0; i < 3; ++i) {
				std::string subStr;
				ss >> subStr;
				v[i] = std::stof(subStr);
			}
			vertex.push_back(v);
			color.push_back(glm::vec3(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX)));
		}
		else if (str == "vn") {
			glm::vec3 n;
			for (int i = 0; i < 3; ++i) {
				std::string subStr;
				ss >> subStr;
				n[i] = std::stof(subStr);
			}
			normal.push_back(n);
		}
		else if (str == "f") {
			glm::ivec3 fv;
			glm::ivec3 fn;
			for (int i = 0; i < 3; ++i) {
				std::string substr;
				ss >> substr;
				std::stringstream subss{ substr };
				std::string vIdx;
				std::getline(subss, vIdx, '/');
				fv[i] = std::stoi(vIdx) - 1;
				std::getline(subss, vIdx, '/');
				// �ؽ�ó �ǳʶٱ�
				std::getline(subss, vIdx, '/');
				fn[i] = std::stoi(vIdx) - 1;
			}
			vindex.push_back(fv);
			nindex.push_back(fn);
		}
	}

	std::vector<glm::vec3> data;
	for (int i = 0; i < vindex.size(); ++i) {
		data.push_back(vertex[vindex[i][0]]);
		data.push_back(color[vindex[i][0]]);
		data.push_back(normal[nindex[i][0]]);
		data.push_back(vertex[vindex[i][1]]);
		data.push_back(color[vindex[i][1]]);
		data.push_back(normal[nindex[i][1]]);
		data.push_back(vertex[vindex[i][2]]);
		data.push_back(color[vindex[i][2]]);
		data.push_back(normal[nindex[i][2]]);
	}

	std::cout << filename << " File Read, " << data.size() / 3 << " Vertices Exists." << std::endl;
	return data;
}

std::vector<glm::vec3> Scene::readOBJWithUV(std::string filename)
{

	std::ifstream in{ filename };
	if (!in) {
		std::cout << filename << " file read failed\n";
		exit(1);
	}

	std::vector<glm::vec3> vertex;
	std::vector<glm::vec3> tex;
	std::vector<glm::vec3> normal;
	std::vector<glm::ivec3> vindex;
	std::vector<glm::ivec3> tindex;
	std::vector<glm::ivec3> nindex;
	while (in) {
		std::string line;
		std::getline(in, line);
		std::stringstream ss{ line };
		std::string str;
		ss >> str;
		if (str == "v") {
			glm::vec3 v;
			for (int i = 0; i < 3; ++i) {
				std::string subStr;
				ss >> subStr;
				v[i] = std::stof(subStr);
			}
			vertex.push_back(v);
		}
		else if (str == "vt") {
			glm::vec3 t;
			for (int i = 0; i < 3; ++i) {
				std::string subStr;
				ss >> subStr;
				t[i] = std::stof(subStr);
			}
			tex.push_back(t);
		}
		else if (str == "vn") {
			glm::vec3 n;
			for (int i = 0; i < 3; ++i) {
				std::string subStr;
				ss >> subStr;
				n[i] = std::stof(subStr);
			}
			normal.push_back(n);
		}
		else if (str == "f") {
			glm::ivec3 fv;
			glm::ivec3 ft;
			glm::ivec3 fn;
			for (int i = 0; i < 3; ++i) {
				std::string substr;
				ss >> substr;
				std::stringstream subss{ substr };
				std::string vIdx;
				std::getline(subss, vIdx, '/');
				fv[i] = std::stoi(vIdx) - 1;
				std::getline(subss, vIdx, '/');
				ft[i] = std::stoi(vIdx) - 1;
				std::getline(subss, vIdx, '/');
				fn[i] = std::stoi(vIdx) - 1;
			}
			vindex.push_back(fv);
			tindex.push_back(ft);
			nindex.push_back(fn);
		}
	}

	std::vector<glm::vec3> data;
	for (int i = 0; i < vindex.size(); ++i) {
		data.push_back(vertex[vindex[i][0]]);
		data.push_back(tex[tindex[i][0]]);
		data.push_back(normal[nindex[i][0]]);
		data.push_back(vertex[vindex[i][1]]);
		data.push_back(tex[tindex[i][1]]);
		data.push_back(normal[nindex[i][1]]);
		data.push_back(vertex[vindex[i][2]]);
		data.push_back(tex[tindex[i][2]]);
		data.push_back(normal[nindex[i][2]]);
	}

	std::cout << filename << " File Read, " << data.size() / 3 << " Vertices Exists." << std::endl;
	return data;
}
