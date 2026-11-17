/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "common/game_settings.h"
#include "common/game_common.h"

#include "ecs/common/damage_utils.h"
#include "ecs/player/player_components.h"

class LeafRangerSystem : public System
{
public:
    LeafRangerSystem(Scene* scene) : System(scene, "Leaf ranger system") {}

    virtual void OnFixedUpdate(EntityCommandBuffer &ecb) override;
    virtual void OnUpdate(EntityCommandBuffer &ecb) override;

protected:
    void OnAnimFrameChanged(
        entt::entity entity,
        EntityCommandBuffer& ecb,
        const SpriteAnimEvent& animEvent,
        const Transform& transform,
        const PlayerAffiliation& affiliation,
        const PlayerController& controller,
        PlayerAnimInfo& animInfo
    );

    void OnAnimCycleEnd(
        entt::entity entity,
        const SpriteAnimEvent& animEvent,
        SpriteAnimState& anim,
        const PlayerAffiliation& affiliation,
        const PlayerControllerInput& input,
        PlayerAnimInfo& animInfo);
};