/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "ecs/item/arrow_rain.h"
#include "ecs/common/damage_utils.h"
#include "ecs/player/player_components.h"
#include "ecs/player/player_utils.h"

std::unordered_map<entt::entity, float> arrowRainLifetimes;
void ArrowRainCommand::Execute(entt::registry& registry, entt::entity entity)
{
    Create(registry, entity, m_scene, m_affiliation, m_position, m_velocity, m_s);
}

void ArrowRainCommand::Create(
    entt::registry& registry, entt::entity entity,
    Scene* scene, const PlayerAffiliation& affiliation,
    b2Vec2 position, b2Vec2 velocity, float s)
{
    SpriteGroup* spriteGroup = nullptr;
    SpriteAnimManager* animManager = scene->GetAnimManager();

    uint32_t animId = AnimID_Make(AnimCategory::ITEM, AnimType::ARROW_RAIN);
    spriteGroup = animManager->GetSpriteGroup(animId);
    AssertNew(spriteGroup);

    Sprite spriteComponent;
    spriteComponent.texture = spriteGroup->GetTexture();
    spriteComponent.srcRect = *spriteGroup->GetSourceRect(0);
    spriteComponent.dimensions = {
        -s * spriteComponent.srcRect.w / 22.f,
        spriteComponent.srcRect.h / 22.f
    };
    spriteComponent.offset = b2Vec2_zero;

    RenderSortingLayer renderLayer;
    renderLayer.enabled = true;
    renderLayer.layer = LAYER_PLAYER;
    renderLayer.orderInLayer = 1;
    registry.emplace<PlayerAffiliation>(entity, affiliation);
    b2BodyId bodyId = CreateBody(entity, scene, affiliation, position, velocity, s);
    b2Body_SetLinearVelocity(bodyId, velocity);
    registry.emplace<NameComponent>(entity, "Arrow Rain");
    registry.emplace<ArrowRainTag>(entity);
    registry.emplace<Transform>(entity, b2Body_GetTransform(bodyId));
    registry.emplace<FixedUpdateTransform>(entity, b2Body_GetTransform(bodyId));
    registry.emplace<Rigidbody>(entity, bodyId);
    b2Body_SetGravityScale(bodyId, 0);/////////////////////////////////////////////////////
    registry.emplace<Sprite>(entity, spriteComponent);
    registry.emplace<SpriteAnimState>(entity, animId);
    registry.emplace<RenderSortingLayer>(entity, renderLayer);
    registry.emplace<Damageable>(entity);



    arrowRainLifetimes[entity] = 1.0f;

  
}

b2BodyId ArrowRainCommand::CreateBody(entt::entity entity, Scene* scene,
    const PlayerAffiliation& affiliation, b2Vec2 position, b2Vec2 velocity, float s)
{
    b2WorldId worldId = scene->GetWorld();
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;

    bodyDef.position = position;
    bodyDef.fixedRotation = true;
    bodyDef.enableSleep = true;
    bodyDef.angularDamping = 0.05f;
    bodyDef.linearDamping = 0.05f;


    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
    assert(b2Body_IsValid(bodyId));

    //// Box shape
    //b2Polygon box = b2MakeBox(0.30f, 0.35f);

    //b2ShapeDef shapeDef = b2DefaultShapeDef();
    //shapeDef.density = 0.1f;
    //shapeDef.filter.categoryBits = CATEGORY_ITEM;
    //shapeDef.filter.maskBits = CATEGORY_TERRAIN | CATEGORY_ATTACK;
    //shapeDef.material.friction = 0.2f;
    //shapeDef.material.restitution = 0.f;
    //shapeDef.enableContactEvents = true;




    //// --- Shape 1 : 
    //b2Polygon boxCollider = b2MakeBox(0.30f, 0.35f);

    //b2ShapeDef colliderShape = b2DefaultShapeDef();
    //colliderShape.density = 0.1f;
    //colliderShape.filter.categoryBits = CATEGORY_ITEM;
    //colliderShape.filter.maskBits = CATEGORY_TERRAIN;
    //colliderShape.material.friction = 0.2f;
    //colliderShape.material.restitution = 0.f;
    //colliderShape.isSensor = false;
    //b2CreatePolygonShape(bodyId, &colliderShape, &boxCollider);
    const uint64_t otherTeamMask = g_gameCommon.GetOtherTeamMask(affiliation.playerID);
    // --- Shape 2 : 
    b2Polygon boxSensor = b2MakeBox(2.5f, 2.5f);
    b2ShapeDef sensorShape = b2DefaultShapeDef();
    sensorShape.density = 0.1f;
    sensorShape.filter.categoryBits = CATEGORY_PROJECTILE;
    sensorShape.filter.maskBits = otherTeamMask;
    sensorShape.isSensor = true;
    sensorShape.enableSensorEvents = true;
    b2CreatePolygonShape(bodyId, &sensorShape, &boxSensor);


    //b2ShapeId polygonShape = b2CreatePolygonShape(bodyId, &shapeDef, &box);

    return bodyId;
}

void ArrowRainSystem::OnFixedUpdate(EntityCommandBuffer& ecb)
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

        bool isArrowRain = m_registry.all_of<ArrowRainTag>(sensorEntity);
        bool isPlayer = m_registry.all_of<PlayerAffiliation>(visitorEntity);

        if (isArrowRain && isPlayer)
        {

            bool damaged = PlayerUtils::DomageIfPlayer(m_registry, visitorEntity, -20.f);


          /*  if (delta > 3)
            {
                ecb.DestroyEntity(sensorEntity);
                EmitParticles(b2Body_GetTransform(m_registry.get<Rigidbody>(sensorEntity).bodyId).p);
            }*/
        }
    }
    float delta = m_scene->GetDelta();

    for (auto truc = arrowRainLifetimes.begin(); truc != arrowRainLifetimes.end(); )
    {
        truc->second -= delta;

        if (truc->second <= 0.f)
        {
            entt::entity e = truc->first;
            if (m_registry.valid(e))
            {
                EmitParticles(b2Body_GetTransform(m_registry.get<Rigidbody>(e).bodyId).p);
                ecb.DestroyEntity(e);
            }
            truc = arrowRainLifetimes.erase(truc);
        }
        else
        {
            ++truc;
        }
    }
}




void ArrowRainSystem::EmitParticles(b2Vec2 position)
{
    // Bonus : Ajoutez un effet lors de l'utilisation de la potion




}
