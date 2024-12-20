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

#define STB_IMAGE_IMPLEMENTATION		// 단 하나의 .cpp 에만 define 해줘야 한다.. 중복 include 주의!
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

	std::vector<int> rowCounts = { 5, 6, 7, 8, 9, 8, 7, 6, 5 }; // 각 줄의 육각형 개수
	float radius = 1.0f; // 육각형 간의 중심 거리 (반지름)
	float startZ = -5.f;  // z 방향 시작점
	int index = 0;       // 오브젝트 인덱스 초기화
	float heightOffset = 5.f * radius; // 층 간의 y 좌표 간격

	int layers = 3; // 층의 개수 (기존 + 위로 2층 추가)


	for (int layer = 0; layer < layers; ++layer) { // 층 반복
		for (size_t row = 0; row < rowCounts.size(); ++row) {
			int hexCount = rowCounts[row];                         // 현재 줄의 육각형 개수
			float startX = -(hexCount - 1) * 1.75f * radius / 2.0f; // 줄의 중앙 시작 x 좌표

			for (int col = 0; col < hexCount; ++col) {
				float x = startX + col * 1.75f * radius; // x 좌표 계산
				float z = startZ;                      // z 좌표는 줄 단위로 고정
				float y = layer * heightOffset;        // 층의 y 좌표 계산

				// 층별 텍스처 배열 정의
				int textureIndex = 0; // 기본값
				if (layer % 3 == 0) { // 1층
					textureIndex = (row + col) % 2; // 0, 1 반복
				}
				else if (layer % 3 == 1) { // 2층
					textureIndex = 2 + (row + col) % 2; // 2, 3 반복
				}
				else if (layer % 3 == 2) { // 3층
					textureIndex = (row + col) % 2 == 0 ? 3 : 1; // 3, 1 반복
				}

				// 육각형 생성 및 설정
				auto obj = std::make_unique<CrushObject>();
				obj->setShader(texShader);
				obj->setVAO(hexagonVAO, hexagonVertexCount);
				obj->setPosition(x, y, z); // y 좌표 포함하여 설정

				obj->initilize();
				// 텍스처 설정
				obj->setTexture(textureIndex); // 선택된 텍스처를 객체에 설정

				// 객체를 벡터에 추가
				objects.push_back(std::move(obj));
			}

			startZ += sqrt(2.3) * radius; // 다음 줄의 z 좌표로 이동
		}
		startZ = -5.f; // 각 층마다 z 좌표를 초기화
	}

	objectCount = objects.size(); // 전체 오브젝트 개수
}

void Scene::release()
{
	delete player;
}

void Scene::update(float elapsedTime)
{
	if (isStartScreen || isPaused || isClear || isGameOver) { return; }
	// 경과 시간 누적
	crownSpawnTimer += elapsedTime;
	//std::cout << crownSpawnTimer << "\n";

	// 30초가 지나고 왕관이 아직 배치되지 않았을 때
	if (crownSpawnTimer >= 30.0f && !isCrownPlaced) {
		isCrownPlaced = true;

		// 왕관 오브젝트 생성
		auto crown = std::make_unique<CrushObject>();
		crown->setShader(texShader); // 왕관에 텍스처 셰이더 사용
		crown->setVAO(crownVAO, crownVertexCount); // 왕관 VAO와 Vertex 설정
		crown->setPosition(0.875, 2.2, 0.f); // 왕관 위치 설정
		crown->initilize();
		crown->setDie(false);
		// 왕관 텍스처 설정 (필요하면 변경)
		crown->setTexture(3); // 왕관에 적절한 텍스처 ID 설정


		// 왕관을 오브젝트 목록에 추가
		objects.push_back(std::move(crown));


		// 마지막 오브젝트가 왕관인지 확인
		auto* lastObj = dynamic_cast<CrushObject*>(objects.back().get());
		if (lastObj) {
			//std::cout << "왕관";
		}
	}
	
	if (isCrownPlaced) {
		// 마지막 오브젝트가 삭제되었는지 확인
		if (!objects.empty()) {
			auto* lastObj = dynamic_cast<CrushObject*>(objects.back().get());
			if (lastObj && lastObj->getDie()) {
				std::cout << "왕관이 삭제되었습니다." << std::endl;
				isClear = true;  // 왕관이 삭제된 상태면 게임 클리어
			}
		}
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

	// 업데이트 및 삭제 처리
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] && objects[i]->getDie()) {
			objects[i].reset();  // 객체 삭제
			--objectCount;
		}
		else if (objects[i]) {
			objects[i]->update(elapsedTime);
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

	//glm::vec3 cameraPos = player->getPosition() + glm::vec3(0, 0, +3);		// 플레이어 위치에서
	//cameraPos.y = 5.f;
	//glm::vec3 targetPos = cameraPos + player->getLook();// 플레이어 앞을 바라본다

	 // 플레이어의 현재 위치
	glm::vec3 playerPos = player->getPosition();

	// 카메라의 위치: 플레이어 위치 뒤쪽(Z 축 -3)으로 이동하고 Y 축으로 높임
	glm::vec3 cameraOffset(0.0f, 3.0f, 4.0f); // Y축: 높이, Z축: 거리
	glm::vec3 cameraPos = playerPos - player->getLook() * cameraOffset.z; // 뒤쪽으로 카메라 위치
	cameraPos.y = playerPos.y + cameraOffset.y; // 위쪽으로 카메라 높이 설정

	// 카메라가 바라볼 목표 지점: 플레이어 위치
	glm::vec3 targetPos = playerPos;

	// 월드 좌표계의 "위쪽" 벡터
	glm::vec3 up(0.0f, 1.0f, 0.0f);


	glm::mat4 viewMatrix = glm::lookAt(cameraPos, targetPos, up);
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 100.f);

	

	{	// 바닥을 깔아준다
		// 카메라, 투영은 씬 전체에 적용..
		glUseProgram(plainShader);

		GLint viewLoc = glGetUniformLocation(plainShader, "viewTransform");
		if (viewLoc < 0)
			std::cout << "viewLoc 찾지 못함\n";
		GLint projLoc = glGetUniformLocation(plainShader, "projTransform");
		if (projLoc < 0)
			std::cout << "projLoc 찾지 못함\n";

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 6);

	}

	{
		glUseProgram(fbxShader);

		// 카메라, 투영은 씬 전체에 적용..
		GLint viewLoc = glGetUniformLocation(fbxShader, "view");
		if (viewLoc < 0)
			std::cout << "viewLoc 찾지 못함\n";
		GLint projLoc = glGetUniformLocation(fbxShader, "projection");
		if (projLoc < 0)
			std::cout << "projLoc 찾지 못함\n";
		GLint cameraPosLoc = glGetUniformLocation(texShader, "cameraPos");
		if (cameraPosLoc < 0)
			std::cout << "cameraPosLoc 찾지 못함\n";

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
		glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

		glUseProgram(fbxShader);

		glBindTexture(GL_TEXTURE_2D, playerTexture); // playerTexture 바인딩

		player->draw();		// 안그리긴 해도... 나중에 그릴 수 있으니 호출해준다
	}

	glUseProgram(shader);

	if (isdebug) player->visualizeCollisionBox(viewMatrix, projMatrix);
	// 오브젝트 그리기

	glUseProgram(texShader);

	// 카메라, 투영은 씬 전체에 적용..
	GLint viewLoc = glGetUniformLocation(texShader, "viewTransform");
	if (viewLoc < 0)
		std::cout << "viewLoc 찾지 못함\n";
	GLint projLoc = glGetUniformLocation(texShader, "projTransform");
	if (projLoc < 0)
		std::cout << "projLoc 찾지 못함\n";
	GLint cameraPosLoc = glGetUniformLocation(texShader, "cameraPos");
	if (cameraPosLoc < 0)
		std::cout << "cameraPosLoc 찾지 못함\n";

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniform3f(cameraPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	// OBJ With Texture 그려주기
	glUseProgram(texShader);
	
	// 알파 값 설정 (0.0 ~ 1.0 사이 값)
	float alphaValue = 1.f; // 50% 투명


	for (const auto& obj : objects) {
		if (CrushObject* crushObj = dynamic_cast<CrushObject*>(obj.get())) {
			// CrushObject로 캐스팅 성공
			GLuint textureID = crushObj->getTextureID();

			// 텍스처 바인딩
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
			// Draw 호출
			if (obj->getDie()) { /*std::cout << "그리지마\n";*/ }
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
				if(isdebug) crushObj->visualizeCollisionBox(viewMatrix, projMatrix); // AABB를 시각화
			}
		}
		else {
			// 다른 타입의 GameObject일 경우 처리 (필요 시)
			//std::cerr << "Non-CrushObject encountered in texObjects.\n";
		}
	}
}

void Scene::keyboard(unsigned char key, bool isPressed)
{
	player->keyboard(key, isPressed);

	if (isPressed) {			// 눌러졌을 때
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
				isStartScreen = false;  // 스페이스바로 게임 시작
			}
			break;
		case 'r':
			isGameOver = false;
			isClear = false;
			release();
			// 게임 초기화
			initialize();
			break;
		default:
			break;
		}
	}
	else {						// 떼졌을 때
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

	// 화면 업데이트가 된다....

	constexpr int WHEEL_UP = 3;
	constexpr int WHEEL_DOWN = 4;

	switch (state) {
	case GLUT_DOWN:
		switch (button) {
		case GLUT_LEFT_BUTTON: {
			float xPos = (static_cast<float>(x) / width * 2.f - 1.f);
			float yPos = -(static_cast<float>(y) / height * 2.f - 1.f);

			//std::cout << "좌클릭 : " << x << ", " << y << std::endl;
			//std::cout << "OpenGL x 좌표는 " << xPos << std::endl;
			//std::cout << "OpenGL y 좌표는 " << yPos << std::endl;

			break;
		}
		case GLUT_MIDDLE_BUTTON:
			std::cout << "휠클릭 : " << x << ", " << y << std::endl;
			break;
		case GLUT_RIGHT_BUTTON:
			std::cout << "우클릭 : " << x << ", " << y << std::endl;
			break;
		case WHEEL_UP:
			std::cout << "휠  업 : " << x << ", " << y << std::endl;
			break;
		case WHEEL_DOWN:
			std::cout << "휠다운 : " << x << ", " << y << std::endl;
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
	// 정점 데이터: 위치 (x, y, z), 텍스처 좌표 (u, v, w), 법선 벡터 (nx, ny, nz)
	float vertices[] = {
		//--- 위치 (x, y, z) ---   //--- 텍스처 좌표 (u, v, w) ---   //--- 법선 벡터 (nx, ny, nz) ---
		-1.0f, -1.0f, 0.0f,        0.0f, 0.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Bottom-left
	 1.0f, -1.0f, 0.0f,        1.0f, 0.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Bottom-right
	 1.0f,  1.0f, 0.0f,        1.0f, 1.0f, 0.0f,                0.0f, 0.0f, 1.0f, // Top-right
	-1.0f,  1.0f, 0.0f,        0.0f, 1.0f, 0.0f,                0.0f, 0.0f, 1.0f  // Top-left
	};

	// 인덱스 데이터: 사각형을 두 개의 삼각형으로 정의
	unsigned int indices[] = {
		0, 1, 2, // 삼각형 1
		0, 2, 3  // 삼각형 2
	};

	// VAO, VBO, EBO 생성

	glGenVertexArrays(1, &bgVAO);
	glGenBuffers(1, &bgVBO);
	glGenBuffers(1, &bgEBO);

	// VAO 바인딩
	glBindVertexArray(bgVAO);

	// VBO 설정
	glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// EBO 설정
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 위치 속성 (layout = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 텍스처 좌표 속성 (layout = 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 법선 벡터 속성 (layout = 2)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// VAO 바인딩 해제
	glBindVertexArray(0);

}

void Scene::Drawbg() const
{
	// 텍스처 활성화 및 바인딩
	glActiveTexture(GL_TEXTURE0);

	if(isStartScreen)
		glBindTexture(GL_TEXTURE_2D, bgTexture[0]); // yourTextureID는 생성된 텍스처 ID
	else if (isGameOver) 
		glBindTexture(GL_TEXTURE_2D, bgTexture[1]); // yourTextureID는 생성된 텍스처 ID
	else if (isClear)
		glBindTexture(GL_TEXTURE_2D, bgTexture[2]); // yourTextureID는 생성된 텍스처 ID


	// 셰이더 활성화
	glUseProgram(texShader); // yourShaderProgram은 셰이더 프로그램 ID

	// 유니폼 설정
	glm::mat4 modelMatrix = glm::mat4(1.0f); // 기본 변환 없음
	glm::mat4 viewMatrix = glm::mat4(1.0f);  // 뷰 변환 없음
	glm::mat4 projMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f); // 직교 투영

	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "viewTransform"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projTransform"), 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniform1f(glGetUniformLocation(texShader, "brightness"), 10.0f);

	// VAO 바인딩
	if (!bgVAO || !bgVBO || !bgEBO) {
		std::cerr << "bgVAO, bgVBO, 또는 bgEBO가 생성되지 않았습니다!" << std::endl;
		return;
	}
	glBindVertexArray(bgVAO);

	// 그리기
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// VAO 바인딩 해제
	glBindVertexArray(0);

}



void Scene::initBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// 동적 할당....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	std::vector<glm::vec3> verticies = readOBJ(objFilename);

	*vertexCount = verticies.size() / 3;

	// GPU는 하나의 컴퓨터와 같다... 내부에 gpu전용 cpu, 램, 등등 존재...

	// CPU 메모리에 있는 데이터를, GPU 메모리에 복사
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.size(), verticies.data(), GL_STATIC_DRAW);

	// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(0);

	//// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(1);

	//// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(2);

}

void Scene::initCubeBuffer(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// 동적 할당....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// 정육면체...
	// 정육면체는 면이 6개이고, 점이 8개이고, 선이 12개..
	// 점당 float가 최소 세개(x, y, z)
	// 삼각형 하나당 점 3개
	// 면당 삼각형이 2개
	// 육면체 하나에 면이 6개
	// 6 * 2 * 3 * 3 --> 108개

	*vertexCount = 36;

	float size = 0.5f;
	float vertexData[6 * 2 * 3 * 9]{// 6면, 면당 2삼각형, 삼각형당 점3개, 점당 9개값 --> 324
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

	// CPU 메모리에 있는 데이터를, GPU 메모리에 복사
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 324, vertexData, GL_STATIC_DRAW);

	// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(0);

	// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(1);

	// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(2);
}

void Scene::initBufferWithUV(GLuint* VAO, GLsizei* vertexCount, std::string objFilename)
{
	glGenVertexArrays(1, VAO);		// 동적 할당....
	glBindVertexArray(*VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	std::vector<glm::vec3> verticies = readOBJWithUV(objFilename);

	*vertexCount = verticies.size() / 3;

	// CPU 메모리에 있는 데이터를, GPU 메모리에 복사
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.size(), verticies.data(), GL_STATIC_DRAW);

	// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(0);

	//// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 3));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(1);

	//// 이 데이터가 어떤 데이터인지, 우리가 정의를 했기 때문에, openGL에 알려줘야 한다!
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, reinterpret_cast<void*>(sizeof(float) * 6));
	// location, 갯수, 타입, nomalized?, 간격(바이트), 시작오프셋
	glEnableVertexAttribArray(2);
}

void Scene::initTexture(GLuint* texture, int size, std::string* texFilename)
{
	glGenTextures(size, texture);

	// 실제 이미지 데이터(binary) --> 우리가 아는 실제 이미지를 바이트 단위로 읽어서 저장해야 한다.
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
		unsigned char* data = stbi_load(texFilename[i].c_str(), &width, &height, &level, 0);		// 동적 할당...

		if (3 == level)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	// GPU로 데이터 복사
		else if (4 == level)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else {
			std::cerr << texFilename[i] << " load Error! 3채널 혹은 4채널 이미지를 넣어주세요. 혹은 이미지 파일 오류\n";
			exit(1);
		}

		stbi_image_free(data);
	}
}

GLuint Scene::makeShader(std::string vertexFilename, std::string fragmentFilename)
{
	// 쉐이더 만들자...

	GLint result;
	GLchar errorLog[512];

	GLuint vertexShader, fragmentShader;

	std::string vertexStr = readFile(vertexFilename);	// 이 변수는 지역변수라서
	const char* vertexSource{ vertexStr.c_str() };		// 이 변수에 저장되려면 위 변수가 소멸되면 안된다..!

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
		std::cout << vertexFilename << " 컴파일 성공!" << std::endl;
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
		std::cout << fragmentFilename << " 컴파일 성공!" << std::endl;
	}

	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);		// 만든 vertex Shader를 쉐이더 프로그램에 추가해 준다
	glAttachShader(shaderID, fragmentShader);	// 만든 fragment Shader를 쉐이더 프로그램에 추가해 준다
	glLinkProgram(shaderID);					// 추가한 쉐이더들을 하나의 프로그램으로 생성한다!

	glDeleteShader(vertexShader);				// 이제 더이상 vertex Shaer는 필요없다...
	glDeleteShader(fragmentShader);				// 이제 더이상 fragment Shaer는 필요없다...

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << vertexFilename << ", " << fragmentFilename << " ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		//return;
	}
	else {
		std::cout << vertexFilename << ", " << fragmentFilename << " shader program 생성 성공!" << std::endl;
	}

	return shaderID;
}

std::string Scene::readFile(std::string filename)
{
	std::ifstream in{ filename };
	if (!in) {
		std::cout << filename << "파일 존재하지 않음!" << std::endl;
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

	// c++ stream --> input output을 해주는 흐름?

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
				// 텍스처 건너뛰기
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
