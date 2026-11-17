/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "ecs/item/potion.h"
#include "ecs/common/damage_utils.h"
#include "ecs/player/player_components.h"
#include "ecs/player/player_utils.h"

void PotionCommand::Execute(entt::registry& registry, entt::entity entity)
{
    Create(registry, entity, m_scene, m_position);
}

void PotionCommand::Create(
    entt::registry& registry, entt::entity entity, Scene* scene, b2Vec2 position)
{
    SpriteGroup* spriteGroup = nullptr;
    SpriteAnimManager* animManager = scene->GetAnimManager();

    uint32_t animId = AnimID_Make(AnimCategory::ITEM, AnimType::POTION);
    spriteGroup = animManager->GetSpriteGroup(animId);
    AssertNew(spriteGroup);

    Sprite spriteComponent;
    spriteComponent.texture = spriteGroup->GetTexture();
    spriteComponent.srcRect = *spriteGroup->GetSourceRect(0);
    spriteComponent.dimensions = {
        spriteComponent.srcRect.w / 22.f,
        spriteComponent.srcRect.h / 22.f
    };
    spriteComponent.offset = b2Vec2_zero;

    RenderSortingLayer renderLayer;
    renderLayer.enabled = true;
    renderLayer.layer = LAYER_PLAYER;
    renderLayer.orderInLayer = 1;

    b2BodyId bodyId = CreateBody(entity, scene, position);

    registry.emplace<NameComponent>(entity, "Potion");
    registry.emplace<PotionTag>(entity);
    registry.emplace<Transform>(entity, b2Body_GetTransform(bodyId));
    registry.emplace<FixedUpdateTransform>(entity, b2Body_GetTransform(bodyId));
    registry.emplace<Rigidbody>(entity, bodyId);
    registry.emplace<Sprite>(entity, spriteComponent);
    registry.emplace<SpriteAnimState>(entity, animId);
    registry.emplace<RenderSortingLayer>(entity, renderLayer);
    registry.emplace<Damageable>(entity);
}

b2BodyId PotionCommand::CreateBody(entt::entity entity, Scene* scene, b2Vec2 position)
{
    b2WorldId worldId = scene->GetWorld();
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position;
    bodyDef.fixedRotation = false;
    bodyDef.enableSleep = true;
    bodyDef.angularDamping = 0.05f;
    bodyDef.linearDamping = 0.05f;

    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    assert(b2Body_IsValid(bodyId));

    // Box shape
    b2Polygon box = b2MakeBox(0.30f, 0.35f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 0.1f;
    shapeDef.filter.categoryBits = CATEGORY_ITEM;
    shapeDef.filter.maskBits = CATEGORY_TERRAIN | CATEGORY_ATTACK;
    shapeDef.material.friction = 0.2f;
    shapeDef.material.restitution = 0.f;
    shapeDef.enableContactEvents = true;


    // --- Shape 1 : 
    b2Polygon boxCollider = b2MakeBox(0.30f, 0.35f);

    b2ShapeDef colliderShape = b2DefaultShapeDef();
    colliderShape.density = 0.1f;
    colliderShape.filter.categoryBits = CATEGORY_ITEM;
    colliderShape.filter.maskBits = CATEGORY_TERRAIN;
    colliderShape.material.friction = 0.2f;
    colliderShape.material.restitution = 0.f;
    colliderShape.isSensor = false;
    b2CreatePolygonShape(bodyId, &colliderShape, &boxCollider);

    // --- Shape 2 : 
    b2Polygon boxSensor = b2MakeBox(0.30f, 0.35f);
    b2ShapeDef sensorShape = b2DefaultShapeDef();
    sensorShape.density = 0.1f;
    sensorShape.filter.categoryBits = CATEGORY_PROJECTILE;
    sensorShape.filter.maskBits = CATEGORY_ALL_TEAMS;
    sensorShape.isSensor = true;
    sensorShape.enableSensorEvents = true;
    b2CreatePolygonShape(bodyId, &sensorShape, &boxSensor);


    b2ShapeId polygonShape = b2CreatePolygonShape(bodyId, &shapeDef, &box);

    return bodyId;
}

void PotionSystem::OnFixedUpdate(EntityCommandBuffer& ecb)
{
    b2WorldId worldId = m_scene->GetWorld();
    b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);

    for (int i = 0; i < sensorEvents.beginCount; i++)
    {
        b2SensorBeginTouchEvent* beginEvent = sensorEvents.beginEvents + i;

        entt::entity sensorEntity = RigidbodyUtils::GetEntity(beginEvent->sensorShapeId);
        entt::entity visitorEntity = RigidbodyUtils::GetEntity(beginEvent->visitorShapeId);

        if (!m_registry.valid(sensorEntity) || !m_registry.valid(visitorEntity))
            continue;

        bool isPotion = m_registry.all_of<PotionTag>(sensorEntity);
        bool isPlayer = m_registry.all_of<PlayerAffiliation>(visitorEntity);

        if (isPotion && isPlayer)
        {

            bool healed = PlayerUtils::HealIfPlayer(m_registry, visitorEntity, 30.f);

            if (healed)
            {
                ecb.DestroyEntity(sensorEntity);
                EmitParticles(b2Body_GetTransform(m_registry.get<Rigidbody>(sensorEntity).bodyId).p);
            }
        }
    }
}



void PotionSystem::EmitParticles(b2Vec2 position)
{
    // Bonus : Ajoutez un effet lors de l'utilisation de la potion




}
