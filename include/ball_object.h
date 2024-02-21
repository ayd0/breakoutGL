#ifndef BALL_OBJECT_H
#define BALL_OBJECT_H

#include <glad/glad.h>
#include "glm/glm.hpp"

#include "../include/game_object.h"
#include "../include/texture2D.h"


class BallObject : public GameObject {
public:
    float   Radius;
    bool    Stuck;
    bool    Sticky, PassThrough;

    BallObject();
    BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);
    virtual ~BallObject() = default;

    glm::vec2 Move(float dt, unsigned int window_width);
    void      Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif // BALL_OBJECT_H
