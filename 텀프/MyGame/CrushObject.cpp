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
    //// AABB 초기화
    //height = std::sqrt(3.0f) * radius; // 육각형 높이

    //glm::vec3 minBounds(-radius + 0.1, 0 , -height / 2.0f); // 최소 경계
    //glm::vec3 maxBounds(radius -0.2, 0.4 , height / 2.0f);  // 최대 경계

    //boundingBox.setBounds(minBounds, maxBounds); // AABB 설정
   /* std::cout << "Min Bounds: " << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << std::endl;
    std::cout << "Max Bounds: " << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << std::endl;*/

    // AABB 초기화
    height = std::sqrt(3.0f) * radius; // 육각형 높이

    // 객체의 현재 위치를 반영하여 AABB 경계를 설정
    glm::vec3 position = getPosition(); // 현재 위치

    glm::vec3 minBounds(position.x - radius + 0.1f, position.y, position.z - height / 2.0f); // 최소 경계
    glm::vec3 maxBounds(position.x + radius - 0.2f, position.y + 0.4f, position.z + height / 2.0f); // 최대 경계

    boundingBox.setBounds(minBounds, maxBounds); // AABB 설정
}

void CrushObject::update(float elapsedTime)
{
}

void CrushObject::draw() const
{
	// 모델변환행렬
	GLint modelLoc = glGetUniformLocation(shader, "modelTransform");
	if (modelLoc < 0)
		std::cout << "modelLoc 찾지 못함\n";
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

    // AABB의 반쪽 크기 (Half Extents)
    float halfHeight = sqrt(3.0f) * radius / 2.0f; // 높이의 절반
    float halfRadius = radius;                    // 반경
    float halfDepth = radius * sqrt(3.0f) / 2.0f; // 깊이의 절반

    glm::vec3 halfExtent = glm::vec3(halfRadius, halfHeight, halfDepth);
    std::cout << "halfExtent: " << halfExtent.x << ", " << halfExtent.y << ", " << halfExtent.z << std::endl;
    
    // 최소 및 최대 경계 계산
    glm::vec3 minBounds = center - glm::vec3(halfExtent.x, 0, halfExtent.z);
    glm::vec3 maxBounds = center + glm::vec3(halfExtent.x, 0.4, halfExtent.z);

    boundingBox.setBounds(minBounds, maxBounds);

    // 디버깅 출력
    std::cout << "Center: " << center.x << ", " << center.y << ", " << center.z << std::endl;
    std::cout << "Min Bounds: " << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << std::endl;
    std::cout << "Max Bounds: " << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << std::endl;
}

bool CrushObject::checkCollision(const GameObject& other) const
{
    // PlayerObject와의 충돌 검사
    const PlayerObject* playerObj = dynamic_cast<const PlayerObject*>(&other);
    if (playerObj) {
        // PlayerObject는 구체로 가정하고 충돌 검사
        glm::vec3 sphereCenter = playerObj->getCenter();  // 플레이어의 위치
        float sphereRadius = playerObj->getRadius();      // 플레이어의 반지름

        // AABB와 구체 간의 충돌 검사
        glm::vec3 closestPoint = glm::clamp(sphereCenter, boundingBox.getMinBounds(), boundingBox.getMaxBounds());
        float distanceSquared = glm::distance2(sphereCenter, closestPoint);

        return distanceSquared <= (sphereRadius * sphereRadius);
    }

    // 다른 충돌 타입에 대한 처리 (현재는 PlayerObject만 처리)
    std::cerr << "CrushObject: 지원하지 않는 충돌 타입\n";
    return false;
}

void CrushObject::visualizeCollisionBox(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) const
{
    glm::vec3 min = boundingBox.getMinBounds();
    glm::vec3 max = boundingBox.getMaxBounds();

    // 충돌 박스의 꼭짓점
    glm::vec3 vertices[] = {
         {-0.8f, 0.0f, -1.0f}, {-0.8f, 0.0f, 1.0f},
        {-0.8f, 0.4f, -1.0f}, {-0.8f, 0.4f, 1.0f},
        {0.8f, 0.0f, -1.0f}, {0.8f, 0.0f, 1.0f},
        {0.8f, 0.4f, -1.0f}, {0.8f, 0.4f, 1.0f}
    };

    // 충돌 박스의 선 인덱스
    GLuint indices[] = {
        0, 1, 1, 3, 3, 2, 2, 0, // 아래 면
        4, 5, 5, 7, 7, 6, 6, 4, // 위 면
        0, 4, 1, 5, 2, 6, 3, 7  // 수직 연결
    };

    // OpenGL 버퍼 설정
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

    
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // 리소스 해제
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}
