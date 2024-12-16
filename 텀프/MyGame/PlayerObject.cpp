
#include "PlayerObject.h"
#include "CrushObject.h"
#include "AABB.h"

#include <iostream>
#include <cmath>
#include <vector>


PlayerObject::PlayerObject() : isJumping(false), isOnGround(true), gravity(-9.8f), jumpSpeed(5.0f)
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
    radius = 0.5f;

    loadModel("OBJ/1.fbx");

    // VAO 생성
    glGenVertexArrays(1, &FbxVAO);
    glBindVertexArray(FbxVAO);

    // VBO 생성
    glGenBuffers(1, &FbxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, FbxVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 정점 위치 속성
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // 정점 노멀 속성
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // 텍스처 좌표 속성
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0); // VAO 바인딩 해제

    loadTexture("Image/1.png"); // 텍스처 파일 경로
}

void PlayerObject::update(float elapsedTime)
{
    glm::vec3 dir(0.f);
    if (isWPressed)
        dir += glm::vec3(getLook().x, 0.f, getLook().z);  // Y축 이동 제거
    if (isAPressed) {
        dir += glm::vec3(getRight().x, 0.f, getRight().z);
    }
    if (isSPressed)
        dir -= glm::vec3(getLook().x, 0.f, getLook().z);
    if (isDPressed)
        dir -= glm::vec3(getRight().x, 0.f, getRight().z);


    if (glm::length(dir) >= glm::epsilon<float>())    // 0보다 커야한다
        move(dir, moveSpeed * elapsedTime);


    // 중력 적용
    if (!isOnGround) {
        velocity.y += gravity * elapsedTime;
    }

    // 위치 계산
    glm::vec3 newPosition = getPosition() + velocity * elapsedTime;
    setPosition(newPosition);

    // 바닥 충돌 검사
    if (getCollision()) {
        if (velocity.y <= 0.0f) { // 아래로 떨어지는 중일 때만
            isOnGround = true;   // 지면에 닿음
            setCollision(false);
            isJumping = false;   // 점프 종료
            velocity.y = 0.0f;   // 수직 속도 초기화
        }
    } else {
        isOnGround = false;      // 공중 상태
    }

   

}

void PlayerObject::draw() const
{
    glm::mat4 rotatedTransform = worldTransform; // 기본 값은 기존 worldTransform

    if (isAPressed) {
        // 왼쪽으로 90도 회전
        rotatedTransform = glm::rotate(worldTransform, glm::radians(+90.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isDPressed) {
        // 오른쪽으로 -90도 회전
        rotatedTransform = glm::rotate(worldTransform, glm::radians(-90.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isWPressed) {
        // 오른쪽으로 -90도 회전
        rotatedTransform = glm::rotate(worldTransform, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isSPressed) {
        // 오른쪽으로 -90도 회전
        rotatedTransform = glm::rotate(worldTransform, glm::radians(180.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    // 스케일 변환 적용
    glm::mat4 finalTransform = glm::scale(rotatedTransform, glm::vec3(1.f, 1.f, 1.f)); // 크기 조정


    // 모델변환행렬
    GLint modelLoc = glGetUniformLocation(shader, "model");
    if (modelLoc < 0)
        std::cout << "modelLoc 찾지 못함\n";
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalTransform));

    glBindVertexArray(FbxVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glm::vec3 playerPos = getPosition();

    // 카메라의 위치: 플레이어 위치 뒤쪽(Z 축 -3)으로 이동하고 Y 축으로 높임
    glm::vec3 cameraOffset(0.0f, 3.0f, 4.0f); // Y축: 높이, Z축: 거리
    glm::vec3 cameraPos = playerPos - getLook() * cameraOffset.z; // 뒤쪽으로 카메라 위치
    cameraPos.y = playerPos.y + cameraOffset.y; // 위쪽으로 카메라 높이 설정

}

void PlayerObject::release()
{
}

void PlayerObject::keyboard(unsigned char key, bool isPressed)
{
    if (isPressed) {            // 눌렸을 때
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
        case ' ':
            isSpacePressed = true;
            jump();
            break;
        }
    }
    else {                        // 떼졌을 때
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
        case ' ':
            isSpacePressed = false;
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
    int moveXValue = x - befMousePosX; // x축 이동량
    int moveYValue = y - befMousePosY; // y축 이동량

    // 좌우 회전 (Yaw)
    if (moveXValue > 0) {
        // 양수면 시계 방향 회전
        rotateY(float(-moveXValue / 3.f));
    }
    else if (moveXValue < 0) {
        // 음수면 반시계 방향 회전
        rotateY(float(-moveXValue / 3.f));
    }

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

    // 모델 변환 행렬을 쉐이더에 전달 (World Transform 사용)
    glm::mat4 scaledTransform = glm::scale(worldTransform, glm::vec3(2.0f, 2.0f, 2.0f)); // 크기 2배
    glm::mat4 finalTransform = glm::translate(scaledTransform, glm::vec3(0.0f, 0.45f, 0.0f)); // Y축으로 1.0 이동

    GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
    if (modelLoc >= 0) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalTransform)); // 스케일 및 이동된 변환 행렬 전달
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

void PlayerObject::jump()
{
    if (isOnGround && !isJumping) {
        velocity.y = jumpSpeed; // 점프 초기 속도 설정
        isOnGround = false;     // 공중 상태로 전환
        isJumping = true;       // 점프 상태로 전환
    }
}

void PlayerObject::processMesh(aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position[0] = mesh->mVertices[i].x;
        vertex.position[1] = mesh->mVertices[i].y;
        vertex.position[2] = mesh->mVertices[i].z;

        if (mesh->mNormals) {
            vertex.normal[0] = mesh->mNormals[i].x;
            vertex.normal[1] = mesh->mNormals[i].y;
            vertex.normal[2] = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoords[0] = mesh->mTextureCoords[0][i].x;
            vertex.texCoords[1] = 1.0f - mesh->mTextureCoords[0][i].y; // UV Y축 뒤집기
        }
        else {
            vertex.texCoords[0] = 0.0f;
            vertex.texCoords[1] = 0.0f;
        }

        vertices.push_back(vertex);
    }
}

void PlayerObject::loadTexture(const std::string& path)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        exit(1);
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED; // 그레이스케일 텍스처
    else if (nrChannels == 3)
        format = GL_RGB; // RGB 텍스처
    else if (nrChannels == 4)
        format = GL_RGBA; // RGBA 텍스처
    else {
        std::cerr << "Unsupported texture format: " << nrChannels << " channels" << std::endl;
        stbi_image_free(data);
        exit(1);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void PlayerObject::loadModel(const std::string& path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: " << importer.GetErrorString() << std::endl;
        exit(1);
    }

    aiMesh* mesh = scene->mMeshes[0];
    processMesh(mesh);
}