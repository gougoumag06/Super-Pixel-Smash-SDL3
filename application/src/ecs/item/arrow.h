/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "common/game_settings.h"
#include "common/game_common.h"
#include "ecs/common/damage_utils.h"


struct ArrowTag {};

struct ArrowCommand : public EntityCommand
{
public:
    ArrowCommand(EntityCommandBuffer& ecb, Scene* scene,
        const PlayerAffiliation& affiliation, b2Vec2 position, b2Vec2 velocity, float s)
        : EntityCommand(ecb, ecb.CreateEntity())
        , m_scene(scene)
        , m_affiliation(affiliation)
        , m_position(position)
        , m_velocity(velocity)
        , m_s(s)

    {
    }

    virtual ~ArrowCommand() = default;

    virtual void Execute(entt::registry& registry, entt::entity entity) override;

    static void Create(
        entt::registry& registry, entt::entity entity,
        Scene* scene, const PlayerAffiliation& affiliation,
        b2Vec2 position, b2Vec2 velocity, float s);



protected:
    static b2BodyId CreateBody(
        entt::entity entity, Scene* scene,
        const PlayerAffiliation& affiliation, b2Vec2 position, b2Vec2 velocity, float s);

    Scene* m_scene;
    PlayerAffiliation m_affiliation;//////
    b2Vec2 m_position;
    b2Vec2 m_velocity;
    float m_s;

};

class ArrowSystem : public System
{
public:
    ArrowSystem(Scene* scene) : System(scene, "Arrow system") {}
    virtual void OnFixedUpdate(EntityCommandBuffer& ecb) override;

protected:
    void EmitParticles(b2Vec2 position);
};
