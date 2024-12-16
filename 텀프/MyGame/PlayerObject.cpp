
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

    // VAO ����
    glGenVertexArrays(1, &FbxVAO);
    glBindVertexArray(FbxVAO);

    // VBO ����
    glGenBuffers(1, &FbxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, FbxVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // ���� ��ġ �Ӽ�
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // ���� ��� �Ӽ�
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // �ؽ�ó ��ǥ �Ӽ�
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0); // VAO ���ε� ����

    loadTexture("Image/1.png"); // �ؽ�ó ���� ���
}

void PlayerObject::update(float elapsedTime)
{
    glm::vec3 dir(0.f);
    if (isWPressed)
        dir += glm::vec3(getLook().x, 0.f, getLook().z);  // Y�� �̵� ����
    if (isAPressed) {
        dir += glm::vec3(getRight().x, 0.f, getRight().z);
    }
    if (isSPressed)
        dir -= glm::vec3(getLook().x, 0.f, getLook().z);
    if (isDPressed)
        dir -= glm::vec3(getRight().x, 0.f, getRight().z);


    if (glm::length(dir) >= glm::epsilon<float>())    // 0���� Ŀ���Ѵ�
        move(dir, moveSpeed * elapsedTime);


    // �߷� ����
    if (!isOnGround) {
        velocity.y += gravity * elapsedTime;
    }

    // ��ġ ���
    glm::vec3 newPosition = getPosition() + velocity * elapsedTime;
    setPosition(newPosition);

    // �ٴ� �浹 �˻�
    if (getCollision()) {
        if (velocity.y <= 0.0f) { // �Ʒ��� �������� ���� ����
            isOnGround = true;   // ���鿡 ����
            setCollision(false);
            isJumping = false;   // ���� ����
            velocity.y = 0.0f;   // ���� �ӵ� �ʱ�ȭ
        }
    } else {
        isOnGround = false;      // ���� ����
    }

   

}

void PlayerObject::draw() const
{
    glm::mat4 rotatedTransform = worldTransform; // �⺻ ���� ���� worldTransform

    if (isAPressed) {
        // �������� 90�� ȸ��
        rotatedTransform = glm::rotate(worldTransform, glm::radians(+90.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isDPressed) {
        // ���������� -90�� ȸ��
        rotatedTransform = glm::rotate(worldTransform, glm::radians(-90.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isWPressed) {
        // ���������� -90�� ȸ��
        rotatedTransform = glm::rotate(worldTransform, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    }
    else if (isSPressed) {
        // ���������� -90�� ȸ��
        rotatedTransform = glm::rotate(worldTransform, glm::radians(180.0f), glm::vec3(0.f, 1.f, 0.f));
    }
    // ������ ��ȯ ����
    glm::mat4 finalTransform = glm::scale(rotatedTransform, glm::vec3(1.f, 1.f, 1.f)); // ũ�� ����


    // �𵨺�ȯ���
    GLint modelLoc = glGetUniformLocation(shader, "model");
    if (modelLoc < 0)
        std::cout << "modelLoc ã�� ����\n";
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalTransform));

    glBindVertexArray(FbxVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glm::vec3 playerPos = getPosition();

    // ī�޶��� ��ġ: �÷��̾� ��ġ ����(Z �� -3)���� �̵��ϰ� Y ������ ����
    glm::vec3 cameraOffset(0.0f, 3.0f, 4.0f); // Y��: ����, Z��: �Ÿ�
    glm::vec3 cameraPos = playerPos - getLook() * cameraOffset.z; // �������� ī�޶� ��ġ
    cameraPos.y = playerPos.y + cameraOffset.y; // �������� ī�޶� ���� ����

}

void PlayerObject::release()
{
}

void PlayerObject::keyboard(unsigned char key, bool isPressed)
{
    if (isPressed) {            // ������ ��
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
    else {                        // ������ ��
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
    int moveXValue = x - befMousePosX; // x�� �̵���
    int moveYValue = y - befMousePosY; // y�� �̵���

    // �¿� ȸ�� (Yaw)
    if (moveXValue > 0) {
        // ����� �ð� ���� ȸ��
        rotateY(float(-moveXValue / 3.f));
    }
    else if (moveXValue < 0) {
        // ������ �ݽð� ���� ȸ��
        rotateY(float(-moveXValue / 3.f));
    }

    befMousePosX = x;
    befMousePosY = y;
}

bool PlayerObject::checkCollision(const GameObject& other) const
{
    const CrushObject* crushObj = dynamic_cast<const CrushObject*>(&other);
    if (crushObj) {
        // CrushObject���� �浹 �˻�
        const AABB& aabb = crushObj->getAABB();
        glm::vec3 sphereCenter = glm::vec3(getPosition().x, getPosition().y, getPosition().z);  // getPosition()���� ����

        float sphereRadius = getRadius();

        // AABB-��ü �浹 �˻�
        glm::vec3 closestPoint = glm::clamp(sphereCenter, aabb.getMinBounds(), aabb.getMaxBounds());
        float distanceSquared = glm::distance2(sphereCenter, closestPoint);

        return distanceSquared <= (sphereRadius * sphereRadius);
    }
}

void PlayerObject::visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const
{
    // OpenGL ���� ����
     // ���� �����
    glUseProgram(shader); // ���� ���̴� Ȱ��ȭ

    GLint viewLoc = glGetUniformLocation(shader, "viewTransform");
    if (viewLoc < 0)
        std::cout << "viewLoc ã�� ����\n";

    GLint projLoc = glGetUniformLocation(shader, "projTransform");
    if (projLoc < 0)
        std::cout << "projLoc ã�� ����\n";

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));

    // �� ��ȯ ����� ���̴��� ���� (World Transform ���)
    glm::mat4 scaledTransform = glm::scale(worldTransform, glm::vec3(2.0f, 2.0f, 2.0f)); // ũ�� 2��
    glm::mat4 finalTransform = glm::translate(scaledTransform, glm::vec3(0.0f, 0.45f, 0.0f)); // Y������ 1.0 �̵�

    GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
    if (modelLoc >= 0) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalTransform)); // ������ �� �̵��� ��ȯ ��� ����
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertexCount); // ���� �׸���
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
        velocity.y = jumpSpeed; // ���� �ʱ� �ӵ� ����
        isOnGround = false;     // ���� ���·� ��ȯ
        isJumping = true;       // ���� ���·� ��ȯ
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
            vertex.texCoords[1] = 1.0f - mesh->mTextureCoords[0][i].y; // UV Y�� ������
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
        format = GL_RED; // �׷��̽����� �ؽ�ó
    else if (nrChannels == 3)
        format = GL_RGB; // RGB �ؽ�ó
    else if (nrChannels == 4)
        format = GL_RGBA; // RGBA �ؽ�ó
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