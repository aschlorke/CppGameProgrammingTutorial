#include "interactionWorld.hpp"

void InteractionWorld::onMakeEntity(EntityHandle handle)
{
    entities.push_back(handle);
}

void InteractionWorld::onRemoveEntity(EntityHandle handle)
{
    entitiesToRemove.push_back(handle);
}
void InteractionWorld::onAddComponent(EntityHandle handle, uint32 id)
{
    if (id == ColliderComponent::ID)
    {
        if (ecs.getComponent<TransformComponent>(handle) != nullptr)
        {
            entities.push_back(handle);
        }
    }
    else if (id == TransformComponent::ID)
    {
        if (ecs.getComponent<ColliderComponent>(handle) != nullptr)
        {
            entities.push_back(handle);
        }
    }
}
void InteractionWorld::onRemoveComponent(EntityHandle handle, uint32 id)
{
    if (id == ColliderComponent::ID || id == TransformComponent::ID)
    {
        entitiesToRemove.push_back(handle);
    }
}

void InteractionWorld::processInteractions(float delta)
{
    removeEntities();

    int axis = findHighestVarianceAxis();

    // Sort AABBs by min on that axis
    // Go through the list, test intersections in range
}

void InteractionWorld::removeEntities()
{
    if (entitiesToRemove.size() == 0)
    {
        return;
    }

    for (size_t i = 0; i < entitiesToRemove.size(); i++)
    {
        bool didRemove = false;
        do
        {
            didRemove = false;
            for (size_t j = 0; j < entities.size(); j++)
            {
                if (entitiesToRemove[i] == entities[j])
                {
                    entities.swap_remove(i);
                    entitiesToRemove.swap_remove(j);
                    didRemove = true;
                    break;
                }
            }
            if (didRemove && entitiesToRemove.size() == 0)
            {
                return;
            }
        } while (didRemove);
    }

    entitiesToRemove.clear();
}

int InteractionWorld::findHighestVarianceAxis()
{
}