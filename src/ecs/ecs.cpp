#include "ecs.hpp"
#include "math/math.hpp"

ECS::~ECS()
{
    for (Map<uint32, Array<uint8>>::iterator it = components.begin(); it != components.end(); ++it)
    {
        size_t typeSize = BaseECSComponent::getTypeSize(it->first);
        ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);

        for (uint32 i = 0; i < it->second.size(); i += typeSize)
        {
            freefn((BaseECSComponent *)&it->second[i]);
        }
    }

    for (uint32 i = 0; i < entities.size(); i++)
    {
        delete entities[i];
    }
}

// entity methods
EntityHandle ECS::makeEntity(BaseECSComponent *entityComponents, const uint32 *componentIDs, size_t numComponents)
{
    std::pair<uint32, Array<std::pair<uint32, uint32>>> *newEntity = new std::pair<uint32, Array<std::pair<uint32, uint32>>>();
    EntityHandle handle = (EntityHandle)newEntity;

    for (uint32 i = 0; i < numComponents; i++)
    {
        if (BaseECSComponent::isTypeValid(componentIDs[i]))
        {
            DEBUG_LOG("ECS", LOG_ERROR, "'%u' is not a valid component type", componentIDs[i]);
            delete newEntity;
            return NULL_ENTITY_HANDLE;
        }

        addComponentInternal(handle, newEntity->second, componentIDs[i], &entityComponents[i]);
    }

    newEntity->first = entities.size();
    entities.push_back(newEntity);

    return handle;
}
void ECS::removeEntity(EntityHandle handle)
{
    auto entity = handleToEntity(handle);
    for (uint32 i = 0; i < entity.size(); i++)
    {
        deleteComponent(entity[i].first, entity[i].second);
    }

    uint32 destIndex = handleToEntityIndex(handle);
    uint32 srcIndex = entities.size() - 1;

    delete entities[destIndex];

    entities[destIndex] = entities[srcIndex];
    entities.pop_back();
}

void ECS::addComponentInternal(EntityHandle handle, Array<std::pair<uint32, uint32>> &entity, uint32 componentID, BaseECSComponent *component)
{
    ECSComponentCreateFunction createfn = BaseECSComponent::getTypeCreateFunction(componentID);
    std::pair<uint32, uint32> newPair;
    newPair.first = componentID;
    newPair.second = createfn(components[componentID], handle, component);
    entity.push_back(newPair);
}

void ECS::deleteComponent(uint32 componentID, uint32 index)
{
    Array<uint8> &array = components[componentID];
    ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(componentID);
    size_t typeSize = BaseECSComponent::getTypeSize(componentID);

    uint32 srcIndex = array.size() - typeSize;
    BaseECSComponent *srcComponent = (BaseECSComponent *)&array[srcIndex];
    BaseECSComponent *destComponent = (BaseECSComponent *)&array[index];

    freefn(destComponent);

    if (index == srcIndex)
    {
        array.resize(srcIndex);
        return;
    }

    Memory::memcpy(destComponent, srcComponent, typeSize);

    auto srcComponents = handleToEntity(srcComponent->entity);
    for (uint32 i = 0; i < srcComponents.size(); i++)
    {
        if (componentID == srcComponents[i].first && srcIndex == srcComponents[i].second)
        {
            srcComponents[i].second = index;
            break;
        }
    }

    array.resize(srcIndex);
}

bool ECS::removeComponentInternal(EntityHandle handle, uint32 componentID)
{
    auto entityComponents = handleToEntity(handle);
    for (uint32 i = 0; i < entityComponents.size(); i++)
    {
        if (componentID == entityComponents[i].first)
        {
            deleteComponent(entityComponents[i].first, entityComponents[i].second);
            uint32 srcIndex = entityComponents.size() - 1;
            uint32 destIndex = i;

            entityComponents[destIndex] = entityComponents[srcIndex];
            entityComponents.pop_back();
            return true;
        }
    }
    return false;
}

BaseECSComponent *ECS::getComponentInternal(Array<std::pair<uint32, uint32>> &entityComponents, Array<uint8> &array, uint32 componentID)
{
    for (uint32 i = 0; i < entityComponents.size(); i++)
    {
        if (componentID == entityComponents[i].first)
        {
            return (BaseECSComponent *)&array[entityComponents[i].second];
        }
    }

    return nullptr;
}

bool ECS::removeSystem(BaseECSSystem &system)
{
    for (uint32 i = 0; i < systems.size(); i++)
    {
        if (&system == systems[i])
        {
            systems.erase(systems.begin() + i);
            return true;
        }
    }
    return false;
}

void ECS::updateSystems(float delta)
{
    Array<BaseECSComponent *> componentParam;
    Array<Array<uint8> *> componentArrays;
    for (uint32 i = 0; i < systems.size(); i++)
    {
        const Array<uint32> &componentTypes = systems[i]->getComponentTypes();
        if (componentTypes.size() == 1)
        {
            size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
            Array<uint8> &array = components[componentTypes[0]];
            for (uint32 j = 0; j < array.size(); j += typeSize)
            {
                BaseECSComponent *component = (BaseECSComponent *)&array[j];
                systems[j]->updateComponents(delta, &component);
            }
        }
        else
        {
            updateSystemWithMultipleComponents(i, delta, componentTypes, componentParam, componentArrays);
        }
    }
}

uint32 ECS::findLeastCommonComponent(const Array<uint32> &componentTypes)
{
    uint32 minSize = components[componentTypes[0]].size() / BaseECSComponent::getTypeSize(componentTypes[0]);
    uint32 minIndex = 0;
    for (uint32 i = 1; i < componentTypes.size(); i++)
    {
        size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
        uint32 size = components[componentTypes[i]].size() / typeSize;
        if (size < minSize)
        {
            minSize = size;
            minIndex = i;
        }
    }

    return minIndex;
}

void ECS::updateSystemWithMultipleComponents(uint32 index, float delta, const Array<uint32> &componentTypes, Array<BaseECSComponent *> &componentParam, Array<Array<uint8> *> &componentArrays)
{
    componentParam.resize(Math::max(componentParam.size(), componentTypes.size()));
    componentArrays.resize(Math::max(componentArrays.size(), componentTypes.size()));

    for (uint32 i = 0; i < componentTypes.size(); i++)
    {
        componentArrays[i] = &components[componentTypes[i]];
    }

    uint32 minSizeIndex = findLeastCommonComponent(componentTypes);

    size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
    Array<uint8> &array = *componentArrays[minSizeIndex];
    for (uint32 i = 0; i < array.size(); i += typeSize)
    {
        componentParam[minSizeIndex] = (BaseECSComponent *)&array[i];
        Array<std::pair<uint32, uint32>> &entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);
        bool isValid = true;

        for (uint32 j = 0; j < componentTypes.size(); j++)
        {
            if (j == minSizeIndex)
            {
                continue;
            }

            componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
            if (componentParam[j] == nullptr)
            {
                isValid = false;
                break;
            }
        }

        if (isValid)
        {
            systems[index]->updateComponents(delta, &componentParam[0]);
        }
    }
}
