/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "scene_manager/stage_manager.h"
#include "imgui/imgui_manager.h"

#include "ecs/common/camera.h"
#include "ecs/common/damage_utils.h"
#include "ecs/common/kinematic_utils.h"
#include "ecs/common/terrain_utils.h"
#include "ecs/common/tracked_target.h"
#include "ecs/common/visual_id.h"

#include "ecs/player/player_components.h"
#include "ecs/player/player_command.h"
#include "ecs/player/player_input_system.h"
#include "ecs/player/player_controller_system.h"
#include "ecs/player/shield.h"
#include "ecs/player/fire_knight_system.h"
#include "ecs/player/leaf_ranger_system.h"
#include "ecs/player/water_priestess_system.h"
#include "ecs/player/player_ai.h"

#include "ecs/item/potion.h"
#include "ecs/item/bomb.h"
//#include "ecs/item/crystal.h"
#include "ecs/item/arrow.h"
#include "ecs/item/arrow_rain.h"

#include "ecs/stage/star_fields_terrain.h"
#include "ecs/stage/star_fields_platform.h"

StageManager::StageManager(InputManager *inputManager)
    : SceneManager(inputManager)
    , m_oneWayCallback(GetScene())
    , m_state(State::FIGHT)
    , m_paused(false)
    , m_end(false)
    , ilespown(false)
    /*, crystalspown(false)*/
    , m_pauseMenu(nullptr)
    , m_endMenu(nullptr)
    , m_delayStage(0.f)
    , m_delayBomb(-1.f)
    //, m_delayCrystal(-1.f)
    , m_delayPotion(-1.f)
{
    Scene *scene = GetScene();
    entt::registry &registry = scene->GetRegistry();
    SpriteAnimManager *animManager = scene->GetAnimManager();
    animManager->SetAnimIDToString(assets::AnimIDToString);

    AssetManager *assets = scene->GetAssetManager();
    assets::InitAssets(assets);
    assets::InitTextures(assets);
    assets::InitFonts(assets);
    assets::InitMusic(assets);
    assets::InitSFX(assets);
    assets::InitSpriteAnimations(scene);

    std::shared_ptr<ImGuiManager> imGuiManager = std::make_shared<ImGuiManager>(scene);
    scene->SetImGuiManager(imGuiManager);
    imGuiManager->SetFlags(
        IM_GUI_PHYSICS |
        IM_GUI_QUERY |
        IM_GUI_ENTITY_INSPECTOR
    );

    //--------------------------------------------------------------------------
    // Simulation Systems

    // Transform & terrain
    scene->AddSimulationSystem(std::make_shared<TransformSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<KinematicTargetSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<AnimatorSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<OneWaySystem>(scene));
    scene->AddSimulationSystem(std::make_shared<FloatingPlatformSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<GroundSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<TrackedTargetSystem>(scene));

    // Player
    scene->AddSimulationSystem(std::make_shared<PlayerInputSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<PlayerAISystem>(scene));
    scene->AddSimulationSystem(std::make_shared<PlayerControllerSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<FireKnightSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<LeafRangerSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<WaterPriestessSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<PlayerShieldSystem>(scene));

    // Item
    scene->AddSimulationSystem(std::make_shared<BombSystem>(scene));
  /*  scene->AddSimulationSystem(std::make_shared<CrystalSystem>(scene));*/
    scene->AddSimulationSystem(std::make_shared<PotionSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<ArrowSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<ArrowRainSystem>(scene));

    // Other
    scene->AddSimulationSystem(std::make_shared<VisualIDSimulationSystem>(scene));
    scene->AddSimulationSystem(std::make_shared<WorldBoundsSystem>(scene));

    //--------------------------------------------------------------------------
    // Presentation Systems

    scene->AddPresentationSystem(std::make_shared<CameraSystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderGameSystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderUISystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderQuerySystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderPhysicsSystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderGridSystem>(scene));
    scene->AddPresentationSystem(std::make_shared<RenderImGuiSystem>(scene, imGuiManager));

    //--------------------------------------------------------------------------
    // Composants ImGui

    // Transform & physics
    imGuiManager->AddComponent(std::make_unique<ImGuiTransform>());
    imGuiManager->AddComponent(std::make_unique<ImGuiFixedUpdateTransform>());
    imGuiManager->AddComponent(std::make_unique<ImGuiLocalTransform>());
    imGuiManager->AddComponent(std::make_unique<ImGuiRigidbody>());
    imGuiManager->AddComponent(std::make_unique<ImGuiReferencePosition>());
    imGuiManager->AddComponent(std::make_unique<ImGuiKinematicTargetPosition>());
    imGuiManager->AddComponent(std::make_unique<ImGuiKinematicTargetRotation>());

    // Render
    imGuiManager->AddComponent(std::make_unique<ImGuiRenderSortingLayer>());
    imGuiManager->AddComponent(std::make_unique<ImGuiRenderBlendMod>());
    imGuiManager->AddComponent(std::make_unique<ImGuiRenderColorMod>());
    imGuiManager->AddComponent(std::make_unique<ImGuiSprite>());
    imGuiManager->AddComponent(std::make_unique<ImGuiTiledSprite>());
    imGuiManager->AddComponent(std::make_unique<ImGuiTilemapRenderer>());
    imGuiManager->AddComponent(std::make_unique<ImGuiBackgroundLayer>());
    imGuiManager->AddComponent(std::make_unique<ImGuiSpriteAnimState>(scene));

    // Camera
    imGuiManager->AddComponent(std::make_unique<ImGuiCamera>());
    imGuiManager->AddComponent(std::make_unique<ImGuiCameraFollow>());

    // Ground
    imGuiManager->AddComponent(std::make_unique<ImGuiGroundContact>());
    imGuiManager->AddComponent(std::make_unique<ImGuiOneWayPass>());

    // Player
    imGuiManager->AddComponent(std::make_unique<ImGuiDamageable>());
    imGuiManager->AddComponent(std::make_unique<ImGuiPlayerAffiliation>());
    imGuiManager->AddComponent(std::make_unique<ImGuiPlayerAI>());
    imGuiManager->AddComponent(std::make_unique<ImGuiPlayerControllerInput>());
    imGuiManager->AddComponent(std::make_unique<ImGuiPlayerController>());
    imGuiManager->AddComponent(std::make_unique<ImGuiPlayerAnimInfo>());
    imGuiManager->AddComponent(std::make_unique<ImGuiTrackedTarget>());

    // Other
    imGuiManager->AddComponent(std::make_unique<ImGuiVisualID>());

    // Tag
    imGuiManager->AddTag(std::make_unique<ImGuiTag<ShieldTag>>("Shield tag"));
    imGuiManager->AddTag(std::make_unique<ImGuiTag<FireKnightTag>>("Fire knight tag"));
    imGuiManager->AddTag(std::make_unique<ImGuiTag<WaterPriestessTag>>("Water priestess tag"));
    imGuiManager->AddTag(std::make_unique<ImGuiTag<LeafRangerTag>>("Leaf Ranger tag"));
    imGuiManager->AddTag(std::make_unique<ImGuiTag<PotionTag>>("Potion tag"));
    imGuiManager->AddTag(std::make_unique<ImGuiTag<BombTag>>("Bomb tag"));
    /*imGuiManager->AddTag(std::make_unique<ImGuiTag<CrystalTag>>("Crystal tag"));*/


    //--------------------------------------------------------------------------
    // Création du niveau

    // Stats
    g_gameCommon.ResetPlayerStats();
    const StageConfig& stageConfig = g_gameCommon.stageConfig;
    // Stage
    switch (stageConfig.type)
    {
    case StageConfig::Type::STAR_FIELDS:
        InitStarFields();
        ilespown = false;
        break;
    case StageConfig::Type::NEW_WORLD:
        InitNewWorld();
        ilespown = true;
        
        break;
    }

    // Limite de temps
    if (g_gameCommon.stageConfig.mode == StageConfig::Mode::LIMITED_TIME)
    {
        m_delayStage = (float)g_gameCommon.stageConfig.duration * 60.f;
    }

    // Caméra
    CameraCommand::Create(registry, registry.create(), scene, b2Vec2{ 0.f, 1.f });

    // Crée l'interface utilisateur
    m_stageHUD = new UIStageHUD(scene);

    // Délais pour les potions et les bombes
 /*   m_delayBomb = rng::RangeF(5.f, 15.f);
    m_delayPotion = rng::RangeF(5.f, 15.f);*/

    // BONUS - Modifiez les délais en fonctions des paramètres du jeu
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    switch (stageConfig.bombsFrequency)
    {
    case StageConfig::Frequency::NEVER:
        m_delayBomb = 0;
       
        break;
    case StageConfig::Frequency::RARELY:
        m_delayBomb = rng::RangeF(20.f, 40.f);
    
        break;
    case StageConfig::Frequency::SOMETIMES:
        m_delayBomb = rng::RangeF(10.f, 15.f);
       
        break;
    case StageConfig::Frequency::OFTEN:
        m_delayBomb = rng::RangeF(1.f, 2.f);
        break;
    default:
        break;
    }
    
    switch (stageConfig.potionFrequency)
    {
    case StageConfig::Frequency::NEVER:
        m_delayPotion = 0.f;
        break;
    case StageConfig::Frequency::RARELY:
        m_delayPotion = rng::RangeF(20.f, 40.f);
        break;
    case StageConfig::Frequency::SOMETIMES:
        m_delayPotion = rng::RangeF(10.f, 15.f);
        break;
    case StageConfig::Frequency::OFTEN:
        m_delayPotion = rng::RangeF(1.f, 2.f);

        break;
    default:
        break;
    }
    /////////////////////////////////////////////////////////////////////////////
}

StageManager::~StageManager()
{
}

void StageManager::OnSceneUpdate()
{
    Scene *scene = GetScene();
    InputManager *inputManager = scene->GetInputManager();
    ApplicationInput *applicationInput = ApplicationInput::GetFromManager(inputManager);

    ////////////////////////////////////
  
   
    int player1Score = g_gameCommon.playerStats[0].fallCount;
    int player2Score = g_gameCommon.playerStats[1].fallCount;
    if (!ilespown && (player1Score == 2 || player2Score == 2)) {
        entt::registry& registry = scene->GetRegistry();
        StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ 7.0f, 4.5f }, 1);
        ilespown = true;
    }
    ////////////////////////////////////////////////////
    m_delayStage -= scene->GetDelta();
    if (m_delayStage < 0.f)
    {
        QuitScene();
    }
////////////////////////////////////////////////////////////////////////////
    const int playerCount = g_gameCommon.playerCount;
    for (int i = 0; i < playerCount; i++)
    {
        int score = static_cast<int>(g_gameCommon.playerStats[i].ejectionScore);
        score = static_cast<int>(g_gameCommon.playerStats[i].fallCount);
        score = 3 - score;
        if (score == 0) {
            
            if (m_endMenu == nullptr)
            {
                assert(m_endMenu == nullptr);
                
                m_endMenu = new UIEndMenu(scene);
                m_endMenu->SetParent(scene->GetCanvas());
                m_endMenu->PlayFadeIn();

                scene->GetTime().SetTimeScale(0.0f);

                scene->GetAssetManager()->PlaySoundFX(SFX_UI_PAUSE);
            }
        }
         game::UpdateFontSize(scene);
       

    }
  
    //////////////////////////////////////////////////////////////////////////////////////////


    if (applicationInput->pausePressed && m_endMenu == nullptr)
    {
        if (m_paused)
        {
            assert(m_pauseMenu != nullptr);

            m_pauseMenu->PlayFadeOut();
            scene->GetAssetManager()->PlaySoundFX(SFX_UI_UNPAUSE);
        }
        else
        {
            assert(m_pauseMenu == nullptr);

            m_pauseMenu = new UIPauseMenu(scene);
            m_pauseMenu->SetParent(scene->GetCanvas());
            m_pauseMenu->PlayFadeIn();

            scene->GetTime().SetTimeScale(0.0f);
            m_paused = true;

            scene->GetAssetManager()->PlaySoundFX(SFX_UI_PAUSE);
        }
    }

    game::UpdateFontSize(scene);
}

void StageManager::OnSceneFixedUpdate()
{
    Scene *scene = GetScene();
    const StageConfig &stageConfig = g_gameCommon.stageConfig;
    StageManager* stageManager = StageManager::GetFromScene(scene);

    int temps = (int)stageManager->GetRemainingTime();
    
  /*  printf("%d ", temps);*/
   /* if (!crystalspown && (temps<=170.f)) {
        AddCrystal();
        crystalspown = true;
    }*/
    if (m_state == State::FIGHT)
    {
       
       
        if (stageConfig.bombsFrequency != StageConfig::Frequency::NEVER)
        {
            m_delayBomb -= scene->GetDelta();
            if (m_delayBomb < 0.f)
            {
                AddBomb();
            }
        }
        if (stageConfig.potionFrequency != StageConfig::Frequency::NEVER)
        {
            m_delayPotion -= scene->GetDelta();
            if (m_delayPotion < 0.f)
            {
                AddPotion();
            }
        }
    }


}

void StageManager::QuitPause()
{
    m_pauseMenu->Delete();
    m_pauseMenu = nullptr;
    m_paused = false;
    GetScene()->GetTime().SetTimeScale(1.f);
}

StageManager *StageManager::GetFromScene(Scene *scene)
{
    if (scene == nullptr)
    {
        assert(false);
        return nullptr;
    }
    return dynamic_cast<StageManager *>(scene->GetSceneManager());
}

void StageManager::AddPotion()
{
    Scene *scene = GetScene();
    entt::registry &registry = scene->GetRegistry();

    b2Vec2 position = { rng::RangeF(-6.f, 6.f), 10.f };
    PotionCommand::Create(registry, registry.create(), scene, position);

 /*   m_delayPotion = rng::RangeF(15.f, 30.f);*/
    const StageConfig& stageConfig = g_gameCommon.stageConfig;
    switch (stageConfig.potionFrequency)
    {
    case StageConfig::Frequency::NEVER:
        m_delayPotion = 0.f;
        break;
    case StageConfig::Frequency::RARELY:
        m_delayPotion = rng::RangeF(20.f, 40.f);;
        break;
    case StageConfig::Frequency::SOMETIMES:
        m_delayPotion = rng::RangeF(10.f, 15.f);;
        break;
    case StageConfig::Frequency::OFTEN:
        m_delayPotion = rng::RangeF(1.f, 2.f);;

        break;
    default:
        break;
    }
}

void StageManager::AddBomb()
{
    Scene *scene = GetScene();
    entt::registry &registry = scene->GetRegistry();

    b2Vec2 position = { rng::RangeF(-7.f, 7.f), 10.f };
    b2Vec2 velocity = { rng::RangeF(-3.f, 3.f), 0.f };
    BombCommand::Create(registry, registry.create(), scene, position, velocity);

   /* m_delayBomb = rng::RangeF(7.f, 20.f);*/
    const StageConfig& stageConfig = g_gameCommon.stageConfig;
    switch (stageConfig.bombsFrequency)
    {
    case StageConfig::Frequency::NEVER:
        m_delayBomb = 0;

        break;
    case StageConfig::Frequency::RARELY:
        m_delayBomb = rng::RangeF(20.f, 40.f);

        break;
    case StageConfig::Frequency::SOMETIMES:
        m_delayBomb = rng::RangeF(10.f, 15.f);

        break;
    case StageConfig::Frequency::OFTEN:
        m_delayBomb = rng::RangeF(1.f, 2.f);
        break;
    default:
        break;
    }

}

//void StageManager::AddCrystal()
//{
//    Scene* scene = GetScene();
//    entt::registry& registry = scene->GetRegistry();
//
//    b2Vec2 position = {0.f, 10.f };
//    b2Vec2 velocity = { rng::RangeF(-3.f, 3.f), 0.f };
//    CrystalCommand::Create(registry, registry.create(), scene, position, velocity);
//
//   
//
//}
void StageManager::InitStarFields()
{
   
    Scene *scene = GetScene();
    AssetManager *assets = scene->GetAssetManager();
    entt::registry &registry = scene->GetRegistry();

    // Stage
    StarFieldsTerrainCommand::Create(registry, registry.create(), scene, b2Vec2_zero);
    StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ -7.0f, 4.5f }, 0);
    /*StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ 7.0f, 4.5f }, 1);*/
    
  





    // Crée les joueurs
    b2Vec2 startPositions[4] = {
        b2Vec2(-6.f, 0.05f), b2Vec2(+6.f, 0.05f),
        b2Vec2(-2.f, 0.05f), b2Vec2(+2.f, 0.05f),
    };
    for (int playerID = 0; playerID < g_gameCommon.playerCount; playerID++)
    {
        PlayerCommand::Create(
            registry, registry.create(), scene,
            playerID, startPositions[playerID % 4]
        );
    }

    // Background
    float factors[] = { 0.05f, 0.3f, 0.6f };
    LayerMode modes[] = {
        LayerMode::FILL_VERTICAL,
        LayerMode::FILL_BELOW,
        LayerMode::FILL_BELOW
    };
    TextureID textureIDs[] = {
        TEXTURE_STAR_FIELDS_0,
        TEXTURE_STAR_FIELDS_1,
        TEXTURE_STAR_FIELDS_2
    };
    for (int i = 0; i < 3; i++){
        entt::entity layerEntity = registry.create();
        RenderSortingLayer renderLayer{};
        renderLayer.enabled = true;
        renderLayer.layer = LAYER_BACKGROUND;
        renderLayer.orderInLayer = i;

        BackgroundLayer backgroundLayer(assets->GetTexture(textureIDs[i]));
        backgroundLayer.SetDimensions(14.f);
        backgroundLayer.mode = modes[i];
        backgroundLayer.shiftFactor = b2Vec2{ factors[i], 0.9f * factors[i] };

        b2Vec2 position = b2Vec2{ 0.f, -5.f } + 0.5f * backgroundLayer.dimensions;

        registry.emplace<NameComponent>(layerEntity, "Background layer " + std::to_string(i));
        registry.emplace<BackgroundLayer>(layerEntity, backgroundLayer);
        registry.emplace<Transform>(layerEntity, position);
        registry.emplace<RenderSortingLayer>(layerEntity, renderLayer);
    }

    // Music
    scene->GetAssetManager()->FadeInMusic(MUSIC_STAGE_1);
}
void StageManager::InitNewWorld()
{
    Scene* scene = GetScene();
    AssetManager* assets = scene->GetAssetManager();
    entt::registry& registry = scene->GetRegistry();

    // Stage
    StarFieldsTerrainCommand::Create(registry, registry.create(), scene, b2Vec2_zero);
    StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ -7.0f, 4.5f }, 0);
    StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ 7.0f, 4.5f }, 1);
    StarFieldsPlatformCommand::Create(registry, registry.create(), scene, b2Vec2{ 0.f, 7.5f }, 2);



    // Crée les joueurs
    b2Vec2 startPositions[4] = {
        b2Vec2(-6.f, 0.05f), b2Vec2(+6.f, 0.05f),
        b2Vec2(-2.f, 0.05f), b2Vec2(+2.f, 0.05f),
    };
    for (int playerID = 0; playerID < g_gameCommon.playerCount; playerID++)
    {
        PlayerCommand::Create(
            registry, registry.create(), scene,
            playerID, startPositions[playerID % 4]
        );
    }

    // Background
    float factors[] = { 0.05f, 0.3f, 0.6f };
    LayerMode modes[] = {
        LayerMode::FILL_VERTICAL,
        LayerMode::FILL_BELOW,
        LayerMode::FILL_BELOW
    };
    TextureID textureIDs[] = {
        TEXTURE_OAK_WOODS_0,
        TEXTURE_STAR_FIELDS_1,
        TEXTURE_STAR_FIELDS_2
    };
    for (int i = 0; i < 3; i++) {
        entt::entity layerEntity = registry.create();
        RenderSortingLayer renderLayer{};
        renderLayer.enabled = true;
        renderLayer.layer = LAYER_BACKGROUND;
        renderLayer.orderInLayer = i;

        BackgroundLayer backgroundLayer(assets->GetTexture(textureIDs[i]));
        backgroundLayer.SetDimensions(14.f);
        backgroundLayer.mode = modes[i];
        backgroundLayer.shiftFactor = b2Vec2{ factors[i], 0.9f * factors[i] };

        b2Vec2 position = b2Vec2{ 0.f, -5.f } + 0.5f * backgroundLayer.dimensions;

        registry.emplace<NameComponent>(layerEntity, "Background layer " + std::to_string(i));
        registry.emplace<BackgroundLayer>(layerEntity, backgroundLayer);
        registry.emplace<Transform>(layerEntity, position);
        registry.emplace<RenderSortingLayer>(layerEntity, renderLayer);
    }

    // Music
    scene->GetAssetManager()->FadeInMusic(MUSIC_STAGE_CUSTOM);
}
