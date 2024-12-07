#include "AABB.h"

AABB::AABB()
{
}

AABB::AABB(const glm::vec3& minBounds, const glm::vec3& maxBounds)
{
}

AABB::~AABB()
{
}

bool AABB::checkCollision(const AABB& other) const
{
    return (minBounds.x <= other.maxBounds.x && maxBounds.x >= other.minBounds.x) &&
        (minBounds.y <= other.maxBounds.y && maxBounds.y >= other.minBounds.y) &&
        (minBounds.z <= other.maxBounds.z && maxBounds.z >= other.minBounds.z);
}
