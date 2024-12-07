#include "CrushObject.h"

#include <iostream>
#include "PlayerObject.h"

CrushObject::CrushObject() 
{   
    height = 0.f;
    radius = 1.0f;
	isCrush = false;
    
}

CrushObject::~CrushObject()
{
}

void CrushObject::initilize()
{
    //// AABB �ʱ�ȭ
    //height = std::sqrt(3.0f) * radius; // ������ ����

    //glm::vec3 minBounds(-radius + 0.1, 0 , -height / 2.0f); // �ּ� ���
    //glm::vec3 maxBounds(radius -0.2, 0.4 , height / 2.0f);  // �ִ� ���

    //boundingBox.setBounds(minBounds, maxBounds); // AABB ����
   /* std::cout << "Min Bounds: " << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << std::endl;
    std::cout << "Max Bounds: " << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << std::endl;*/

    // AABB �ʱ�ȭ
    height = std::sqrt(3.0f) * radius; // ������ ����

    // ��ü�� ���� ��ġ�� �ݿ��Ͽ� AABB ��踦 ����
    glm::vec3 position = getPosition(); // ���� ��ġ

    glm::vec3 minBounds(position.x - radius + 0.1f, position.y, position.z - height / 2.0f); // �ּ� ���
    glm::vec3 maxBounds(position.x + radius - 0.2f, position.y + 0.4f, position.z + height / 2.0f); // �ִ� ���

    boundingBox.setBounds(minBounds, maxBounds); // AABB ����
}

void CrushObject::update(float elapsedTime)
{
}

void CrushObject::draw() const
{
	// �𵨺�ȯ���
	GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
	if (modelLoc < 0)
		std::cout << "modelLoc ã�� ����\n";
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(worldTransform));

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void CrushObject::release()
{
}

void CrushObject::updateAABB()
{
    glm::vec3 center = glm::vec3(worldTransform[3].x, worldTransform[3].y, worldTransform[3].z);

    // AABB�� ���� ũ�� (Half Extents)
    float halfHeight = sqrt(3.0f) * radius / 2.0f; // ������ ����
    float halfRadius = radius;                    // �ݰ�
    float halfDepth = radius * sqrt(3.0f) / 2.0f; // ������ ����

    glm::vec3 halfExtent = glm::vec3(halfRadius, halfHeight, halfDepth);
    std::cout << "halfExtent: " << halfExtent.x << ", " << halfExtent.y << ", " << halfExtent.z << std::endl;
    
    // �ּ� �� �ִ� ��� ���
    glm::vec3 minBounds = center - glm::vec3(halfExtent.x, 0, halfExtent.z);
    glm::vec3 maxBounds = center + glm::vec3(halfExtent.x, 0.4, halfExtent.z);

    boundingBox.setBounds(minBounds, maxBounds);

    // ����� ���
    std::cout << "Center: " << center.x << ", " << center.y << ", " << center.z << std::endl;
    std::cout << "Min Bounds: " << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << std::endl;
    std::cout << "Max Bounds: " << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << std::endl;
}

bool CrushObject::checkCollision(const GameObject& other) const
{
    // PlayerObject���� �浹 �˻�
    const PlayerObject* playerObj = dynamic_cast<const PlayerObject*>(&other);
    if (playerObj) {
        // PlayerObject�� ��ü�� �����ϰ� �浹 �˻�
        glm::vec3 sphereCenter = playerObj->getCenter();  // �÷��̾��� ��ġ
        float sphereRadius = playerObj->getRadius();      // �÷��̾��� ������

        // AABB�� ��ü ���� �浹 �˻�
        glm::vec3 closestPoint = glm::clamp(sphereCenter, boundingBox.getMinBounds(), boundingBox.getMaxBounds());
        float distanceSquared = glm::distance2(sphereCenter, closestPoint);

        return distanceSquared <= (sphereRadius * sphereRadius);
    }

    // �ٸ� �浹 Ÿ�Կ� ���� ó�� (����� PlayerObject�� ó��)
    std::cerr << "CrushObject: �������� �ʴ� �浹 Ÿ��\n";
    return false;
}

void CrushObject::visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const
{
    glm::vec3 min = boundingBox.getMinBounds();
    glm::vec3 max = boundingBox.getMaxBounds();

    // �浹 �ڽ��� ������
    glm::vec3 vertices[] = {
         {-0.8f, 0.0f, -1.0f}, {-0.8f, 0.0f, 1.0f},
        {-0.8f, 0.4f, -1.0f}, {-0.8f, 0.4f, 1.0f},
        {0.8f, 0.0f, -1.0f}, {0.8f, 0.0f, 1.0f},
        {0.8f, 0.4f, -1.0f}, {0.8f, 0.4f, 1.0f}
    };

    // �浹 �ڽ��� �� �ε���
    GLuint indices[] = {
        0, 1, 1, 3, 3, 2, 2, 0, // �Ʒ� ��
        4, 5, 5, 7, 7, 6, 6, 4, // �� ��
        0, 4, 1, 5, 2, 6, 3, 7  // ���� ����
    };

    // OpenGL ���� ����
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

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

    
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // ���ҽ� ����
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
