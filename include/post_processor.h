#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include "glm/glm.hpp"

#include "texture2D.h"
#include "sprite_renderer.h"
#include "shader.h"

class PostProcessor
{
public:
    Shader PostProcessingShader;
    Texture2D Texture;
    unsigned int Width, Height;
    bool Confuse, Chaos, Shake;

    PostProcessor(Shader shader, unsigned int width, unsigned int height);

    void BeginRender();
    void EndRender();
    void Render(float time);

private:
    unsigned int MSFBO, FBO; // MSFBO = Multisampled FBO
    unsigned int RBO; // RBO is used for multisampled color buffer
    unsigned int VAO;
    void initRenderData();
};

#endif
