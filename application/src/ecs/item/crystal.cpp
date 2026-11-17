///*
//    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
//    Licensed under the MIT License.
//    See LICENSE.md in the project root for license information.
//*/
//
//#include "ecs/item/crystal.h"
//#include "ecs/common/damage_utils.h"
//#include "ecs/player/player_components.h"
//
//void CrystalCommand::Execute(entt::registry& registry, entt::entity entity)
//{
//    Create(registry, entity, m_scene, m_position, m_velocity);
//}
//
//void CrystalCommand::Create(
//    entt::registry& registry, entt::entity entity, Scene* scene, b2Vec2 position, b2Vec2 velocity)
//{
//    SpriteGroup* spriteGroup = nullptr;
//    SpriteAnimManager* animManager = scene->GetAnimManager();
//
//    uint32_t animId = AnimID_Make(AnimCategory::ITEM, AnimType::CRYSTAL);
//
//    spriteGroup = animManager->GetSpriteGroup(animId);
//    AssertNew(spriteGroup);
//
//    Sprite spriteComponent;
//    spriteComponent.texture = spriteGroup->GetTexture();
//    spriteComponent.srcRect = *spriteGroup->GetSourceRect(0);
//    spriteComponent.dimensions = {
//        spriteComponent.srcRect.w / 22.f,
//        spriteComponent.srcRect.h / 22.f
//    };
//    spriteComponent.offset = b2Vec2_zero;
//
//    RenderSortingLayer renderLayer;
//    renderLayer.enabled = true;
//    renderLayer.layer = LAYER_PLAYER;
//    renderLayer.orderInLayer = 1;
//
//    b2BodyId bodyId = CreateBody(entity, scene, position, velocity);
//
//    registry.emplace<NameComponent>(entity, "Crystal");
//    registry.emplace<Transform>(entity, b2Body_GetTransform(bodyId));
//    registry.emplace<FixedUpdateTransform>(entity, b2Body_GetTransform(bodyId));
//    registry.emplace<Rigidbody>(entity, bodyId);
//    registry.emplace<Sprite>(entity, spriteComponent);
//    registry.emplace<SpriteAnimState>(entity, animId, true);
//    registry.emplace<RenderSortingLayer>(entity, renderLayer);
//    registry.emplace<Damageable>(entity);
//    registry.emplace<PlayerAffiliation>(entity);
//    registry.emplace<CrystalTag>(entity);
//}
//
//b2BodyId CrystalCommand::CreateBody(entt::entity entity, Scene* scene, b2Vec2 position, b2Vec2 velocity)
//{
//    b2WorldId worldId = scene->GetWorld();
//    b2BodyDef bodyDef = b2DefaultBodyDef();
//    bodyDef.type = b2_dynamicBody;
//    bodyDef.position = position;
//    bodyDef.linearVelocity = velocity;
//    bodyDef.fixedRotation = false;
//    bodyDef.enableSleep = true;
//    bodyDef.angularDamping = 0.05f;
//    bodyDef.linearDamping = 0.05f;
//
//    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
//    assert(b2Body_IsValid(bodyId));
//
//    // Shape
//    b2Circle circle{};
//    circle.center = b2Vec2{ 0.f, 0.0f };
//    circle.radius = 0.35f;
//    // Box shape
//
//    // Box shape
//    b2Polygon box = b2MakeBox(0.30f, 0.35f);
//
//    b2ShapeDef shapeDef = b2DefaultShapeDef();
//    shapeDef.density = 0.1f;
//    shapeDef.filter.categoryBits = CATEGORY_ITEM;
//    shapeDef.filter.maskBits = CATEGORY_TERRAIN | CATEGORY_ATTACK;
//    shapeDef.material.friction = 0.2f;
//    shapeDef.material.restitution = 0.f;
//    shapeDef.enableContactEvents = true;
//
//
//    // --- Shape 1 : 
//    b2Polygon boxCollider = b2MakeBox(0.30f, 0.35f);
//
//    b2ShapeDef colliderShape = b2DefaultShapeDef();
//    colliderShape.density = 0.1f;
//    colliderShape.filter.categoryBits = CATEGORY_ITEM;
//    colliderShape.filter.maskBits = CATEGORY_TERRAIN;
//    colliderShape.material.friction = 0.2f;
//    colliderShape.material.restitution = 0.f;
//    colliderShape.isSensor = false;
//    b2CreatePolygonShape(bodyId, &colliderShape, &boxCollider);
//
//    // --- Shape 2 : 
//    b2Polygon boxSensor = b2MakeBox(0.30f, 0.35f);
//    b2ShapeDef sensorShape = b2DefaultShapeDef();
//    sensorShape.density = 0.1f;
//    sensorShape.filter.categoryBits = CATEGORY_PROJECTILE;
//    sensorShape.filter.maskBits = CATEGORY_ALL;
//    sensorShape.isSensor = true;
//    sensorShape.enableSensorEvents = true;
//    b2CreatePolygonShape(bodyId, &sensorShape, &boxSensor);
//
//
//    b2ShapeId polygonShape = b2CreatePolygonShape(bodyId, &shapeDef, &box);
//
//    return bodyId;
//}
//
//void CrystalSystem::OnFixedUpdate(EntityCommandBuffer& ecb)
//{
//    b2WorldId worldId = m_scene->GetWorld();
//    b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);
//
//    auto view = m_registry.view<CrystalTag, Damageable, PlayerAffiliation, const Transform>();
//    for (auto [entity, damageable, affiliation, transform] : view.each())
//    {
//        printf("%d ", sensorEvents.beginCount);
//        for (int i = 0; i < sensorEvents.beginCount; i++)
//        {
//            b2SensorBeginTouchEvent* beginEvent = sensorEvents.beginEvents + i;
//
//            entt::entity sensorEntity = RigidbodyUtils::GetEntity(beginEvent->sensorShapeId);
//            entt::entity visitorEntity = RigidbodyUtils::GetEntity(beginEvent->visitorShapeId);
//
//            if (!m_registry.valid(sensorEntity) || !m_registry.valid(visitorEntity))
//                continue;
//
//            bool isCrystal = m_registry.all_of<CrystalTag>(sensorEntity);
//            bool isTerrain = false;
//
//            // Vérifie si la collision implique du terrain (masque de collision)
//            uint16_t visitorCategory = b2Shape_GetFilterCategoryBits(beginEvent->visitorShapeId);
//            if (visitorCategory & CATEGORY_TERRAIN)
//                isTerrain = true;
//
//            if (isCrystal && isTerrain)
//            {
//                // Explosion logique ici
//                const auto& transform = m_registry.get<Transform>(sensorEntity);
//                QueryFilter filter(CATEGORY_ALL_TEAMS);
//                Damage damage;
//                damage.amount = 50.f;
//                damage.ejectionType = Damage::Type::CENTER;
//                damage.attackCenter = transform.position + b2Vec2{ 0.f, -1.5f };
//                damage.ejectionSpeed = 30.f;
//                DamageUtils::AttackCircle(m_scene, sensorEntity, {}, damage, filter, transform.position, 1.5f);
//
//                ecb.DestroyEntity(sensorEntity);  // Détruire le cristal
//                continue;
//            }
//
//            // (suite du code actuel pour interaction joueur + cristal...)
//        }
//
//    }
//
// /*   auto view = m_registry.view<CrystalTag, Damageable, PlayerAffiliation, const Transform>();
//    for (auto [entity, damageable, affiliation, transform] : view.each())
//    {
//        if (damageable.cumulativeDamage.amount <= 0) continue;
//
//        if ((m_registry.valid(damageable.lastDamagerEntity)) &&
//            (m_registry.all_of<PlayerController>(damageable.lastDamagerEntity)))
//        {
//            affiliation = damageable.lastDamagerAffiliation;
//            QueryFilter filter(CATEGORY_ALL_TEAMS);
//            Damage damage;
//            damage.amount = 50.f;
//            damage.ejectionType = Damage::Type::CENTER;
//            damage.attackCenter = transform.position + b2Vec2{ 0.f, -1.5f };
//            damage.ejectionSpeed = 30.f;
//            DamageUtils::AttackCircle(m_scene, entity, affiliation, damage, filter, transform.position, 1.5f);
//
//           
//            ecb.DestroyEntity(entity);
//        }
//        else
//        {
//            DamageUtils::ResetCumulativeDamage(damageable);
//        }
//    }*/
//}
//
//void CrystalSystem::OnUpdate(EntityCommandBuffer& ecb)
//{
//    
//}
//
//void CrystalSystem::EmitParticles(b2Vec2 position)
//{
//   
//}
