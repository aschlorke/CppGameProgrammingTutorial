#pragma once

#include "core/common.hpp"
#include "ecsComponent.hpp"
#include "ecsSystem.hpp"
#include "dataStructures/map.hpp"
#include "core/memory.hpp"

class ECS
{
public:
    ECS() {}
    ~ECS();

    // entity methods
    EntityHandle makeEntity(BaseECSComponent *components, const uint32 *componentIDs, size_t numComponents);
    void removeEntity(EntityHandle handle);

    // component methods
    template <class Component>
    inline void addComponent(EntityHandle handle, Component *component)
    {
        addComponentInternal(handle, handleToEntity(handle), Component::ID, component);
    }

    template <class Component>
    inline bool removeComponent(EntityHandle entity)
    {
        return removeComponentInternal(entity, Component::ID);
    }

    template <class Component>
    inline Component *getComponent(EntityHandle entity)
    {
        return getComponentInternal(handleToEntity(entity), components[Component::ID], Component::ID);
    }

    // system methods
    inline void addSystem(BaseECSSystem &system)
    {
        systems.push_back(&system);
    }
    void updateSystems(float delta);

    bool removeSystem(BaseECSSystem &system);

private:
    Array<BaseECSSystem *> systems;
    Map<uint32, Array<uint8>> components;
    Array<std::pair<uint32, Array<std::pair<uint32, uint32>>> *> entities;

    inline std::pair<uint32, Array<std::pair<uint32, uint32>>> *handleToRawType(EntityHandle handle)
    {
        return (std::pair<uint32, Array<std::pair<uint32, uint32>>> *)handle;
    }

    inline uint32 handleToEntityIndex(EntityHandle handle)
    {
        return handleToRawType(handle)->first;
    }

    inline Array<std::pair<uint32, uint32>> &handleToEntity(EntityHandle handle)
    {
        return handleToRawType(handle)->second;
    }

    void deleteComponent(uint32 componentID, uint32 index);
    bool removeComponentInternal(EntityHandle handle, uint32 componentID);
    void addComponentInternal(EntityHandle handle, Array<std::pair<uint32, uint32>> &entity, uint32 componentID, BaseECSComponent *component);
    BaseECSComponent *getComponentInternal(Array<std::pair<uint32, uint32>> &entityComponents, Array<uint8> &array, uint32 componentID);
    uint32 findLeastCommonComponent(const Array<uint32> &componentTypes);

    void updateSystemWithMultipleComponents(uint32 index, float delta, const Array<uint32> &componentTypes, Array<BaseECSComponent *> &componentParam, Array<Array<uint8> *> &componentArrays);

    NULL_COPY_AND_ASSIGN(ECS);
};