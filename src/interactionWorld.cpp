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
    std::sort(entities.begin(), entities.end(), compareAABB);

    Vector3f centerSum(0.0f);
    Vector3f centerSqSum(0.0f);
    for (size_t i = 0; i < entities.size(); i++)
    {
        AABB aabb = ecs.getComponent<ColliderComponent>(entities[i])->aabb;

        Vector3f center = aabb.getCenter();
        centerSum += center;
        centerSqSum += (center * center);

        for (size_t j = i - 1; j < entities.size(); j++)
        {
            AABB otherAABB = ecs.getComponent<ColliderComponent>(entities[j])->aabb;

            if (otherAABB.getMinExtents()[compareAABB.axis] > aabb.getMaxExtents()[compareAABB.axis])
            {
                break;
            }

            if (aabb.intersects(otherAABB))
            {
            }
        }
    }

    // Set max variance axis
    centerSum /= entities.size();
    centerSqSum /= entities.size();
    Vector3f variance = centerSqSum - (centerSum * centerSum);
    float maxVar = variance[0];
    uint32 maxVarAxis = 0;
    if (variance[1] > maxVar)
    {
        maxVar = variance[1];
        maxVarAxis = 1;
    }
    if (variance[2] > maxVar)
    {
        maxVar = variance[2];
        maxVarAxis = 1;
    }

    compareAABB.axis = maxVarAxis;
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