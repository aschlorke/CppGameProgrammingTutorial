#include "gameRenderContext.hpp"

// flush the render buffer and draw everything
void GameRenderContext::flush()
{
    Texture *currentTexture = nullptr;

    for (auto it = meshRenderBuffer.begin(); it != meshRenderBuffer.end(); it++)
    {
        VertexArray *vertexArray = it->first.first;
        Texture *texture = it->first.second;
        Matrix *transforms = &it->second[0];
        size_t numTransforms = it->second.size();

        if (numTransforms == 0)
        {
            continue;
        }

        if (texture != currentTexture)
        {
            shader.setSampler("diffuse", *texture, sampler, 0);
        }
        vertexArray->updateBuffer(4, transforms, numTransforms * sizeof(Matrix));
        this->draw(shader, *vertexArray, drawParams, numTransforms);

        it->second.clear();
    }
}
