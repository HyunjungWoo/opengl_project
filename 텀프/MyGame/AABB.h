#ifndef AABB_H
#define AABB_H

#include "library.h"

class AABB {
private:
    glm::vec3 minBounds; // AABB ÃÖ¼Ò ÁÂÇ¥
    glm::vec3 maxBounds; // AABB ÃÖ´ë ÁÂÇ¥

public:
    AABB();
    AABB(const glm::vec3& minBounds, const glm::vec3& maxBounds);
    ~AABB();

    void setBounds(const glm::vec3& min, const glm::vec3& max)
    {
        minBounds = min;
        maxBounds = max;
    }

    const glm::vec3& getMinBounds() const { return minBounds; }
    const glm::vec3& getMaxBounds() const { return maxBounds; }

    bool checkCollision(const AABB& other) const;
};

#endif // AABB_