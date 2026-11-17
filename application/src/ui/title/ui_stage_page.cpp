/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "ui/title/ui_stage_page.h"
#include "ui/custom_default/ui_default_cursor.h"
#include "scene_manager/title_manager.h"

UIStagePage::UIStagePage(Scene* scene)
    : UIObject(scene)
    , m_ratioHeader(0.2f)
    , m_header(nullptr)
    , m_content(nullptr)
    , m_group(nullptr)
{
    SetName("Stage page");
    SetParent(m_scene->GetCanvas());

    InputManager* inputManager = m_scene->GetInputManager();
    ApplicationInput* applicationInput = ApplicationInput::GetFromManager(inputManager);

    AssetManager* assets = scene->GetAssetManager();
    SpriteSheet* spriteSheet = nullptr;
    SpriteGroup* spriteGroup = nullptr;
    TTF_Font* font = nullptr;

    m_group = new UISelectableGroup(m_scene);
    m_group->SetParent(this);
    m_group->AddInput(&(applicationInput->uiInputs[0]));
    m_group->SetInteractable(true);

    //--------------------------------------------------------------------------
    // Header

    m_header = new UIObject(scene);
    m_header->SetName("UI Stage Header panel");
    m_header->SetParent(this);
    m_header->GetLocalRect().anchorMin = { 0.f, 1.f - m_ratioHeader };
    m_header->GetLocalRect().anchorMax = { 1.f, 1.f };

    UIFillRect* headerFill = new UIFillRect(scene, Colors::DarkBlue);
    headerFill->SetParent(m_header);
    headerFill->SetOpacity(0.5f);
    headerFill->SetLayer(LAYER_UI_BACKGROUND);

    //--------------------------------------------------------------------------
    // Content

    m_content = new UIObject(scene);
    m_content->SetName("UI Stage Content panel");
    m_content->SetParent(this);
    m_content->GetLocalRect().anchorMin = { 0.f, 0.f };
    m_content->GetLocalRect().anchorMax = { 1.f, 1.f - m_ratioHeader };

    //--------------------------------------------------------------------------
    // Title text

    font = assets->GetFont(FONT_LARGE);
    m_titleText = new UIText(scene, "Stage selection", font, Colors::Gold);
    m_titleText->SetParent(m_header);
    m_titleText->SetAnchor(Anchor::CENTER);

    //--------------------------------------------------------------------------
    // Player text

    font = assets->GetFont(FONT_LARGE);
    m_playerText = new UIText(scene, "Fighter selection", font, Colors::Black);
    m_playerText->SetAnchor(Anchor::WEST);

    //--------------------------------------------------------------------------
    // Options text

    font = assets->GetFont(FONT_LARGE);
    m_optionText = new UIText(scene, "Options", font, Colors::Black);
    m_optionText->SetAnchor(Anchor::WEST);

    //--------------------------------------------------------------------------
    // Cursor

    UIDefaultCursor* cursor = new UIDefaultCursor(scene);
    m_group->SetCursor(cursor);

    //--------------------------------------------------------------------------
    // Player list

    std::string labelString;
    std::vector<std::string> textStrings;
    const float textWidth = 95.f;
    const float playerTextWidth = 135.f;

    std::vector<std::string> basePlayerStrings = {
        "Fire knight",
        "Water priestess",
        "Leaf ranger",
        "Fire knight (CPU)",
        "Water priestess (CPU)",
        "Leaf ranger (CPU)"
    };

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        char labelBuf[32];
        snprintf(labelBuf, sizeof(labelBuf), "Player %d", i + 1);
        std::vector<std::string> options = basePlayerStrings;
        if (i >= 1)
        {
            options.push_back("Off");
        }
        m_playerLists[i] = new UITextList(
            m_scene, std::string(labelBuf), options, Colors::White, playerTextWidth
        );
        m_playerLists[i]->SetIsCycle(true);
        m_playerLists[i]->AddSelectableListener(this);
    }

    //--------------------------------------------------------------------------
    // Stage List
    
    labelString.assign("Select stage");
    textStrings.clear();
    textStrings.push_back("Star Fields");
    textStrings.push_back("New World");
    m_stageList = new UITextList(
        m_scene, labelString, textStrings, Colors::White, textWidth
    );
    // Stage duration list

    labelString.assign("Stage duration");
    textStrings.clear();
    textStrings.push_back("1 min");
    textStrings.push_back("2 min");
    textStrings.push_back("3 min");
    m_timeList = new UITextList(
        m_scene, labelString, textStrings, Colors::White, textWidth
    );
    m_timeList->SetIsCycle(false);
    m_timeList->AddSelectableListener(this);

    ////--------------------------------------------------------------------------
    //// TODO : Ajout d'un texte et d'une liste pour la frï¿½quence d'apparition des potions

    labelString.assign("potion");
    textStrings.clear();
    textStrings.push_back("NEVER");
    textStrings.push_back("RARELY");
    textStrings.push_back("SOMETIMES");
    textStrings.push_back("OFTEN");
    m_potionList = new UITextList(
        m_scene, labelString, textStrings, Colors::White, textWidth
    );
    m_timeList->SetIsCycle(false);
    m_timeList->AddSelectableListener(this);

    /////////////////////////////////////////////
    labelString.assign("bombe");
    textStrings.clear();
    textStrings.push_back("NEVER");
    textStrings.push_back("RARELY");
    textStrings.push_back("SOMETIMES");
    textStrings.push_back("OFTEN");
    m_bombeList = new UITextList(
        m_scene, labelString, textStrings, Colors::White, textWidth
    );
    m_timeList->SetIsCycle(false);
    m_timeList->AddSelectableListener(this);
    //--------------------------------------------------------------------------
    // Buttons

    m_startButton = new UIDefaultButton(scene, "Start");
    m_startButton->AddSelectableListener(this);

    m_backButton = new UIDefaultButton(scene, "Back");
    m_backButton->AddSelectableListener(this);

    //--------------------------------------------------------------------------
    // Grid layouts

    UIGridLayout* vLayout = new UIGridLayout(m_scene, 11, 1);
    vLayout->SetParent(m_content);
    vLayout->SetAnchor(Anchor::CENTER);
    vLayout->SetSpacing(2.f);
    vLayout->SetRowSpacing(4, 10.f);
    vLayout->SetRowSpacing(9, 10.f);
    vLayout->SetRowSize(19.f);
    vLayout->SetColumnSize(300.f);

    vLayout->AddObject(m_playerText, 0, 0);
    vLayout->AddObject(m_playerLists[0], 1, 0);
    vLayout->AddObject(m_playerLists[1], 2, 0);
    vLayout->AddObject(m_playerLists[2], 3, 0);
    vLayout->AddObject(m_playerLists[3], 4, 0);
    vLayout->AddObject(m_optionText, 5, 0);
    vLayout->AddObject(m_timeList, 6, 0);
    vLayout->AddObject(m_potionList, 7, 0);
    vLayout->AddObject(m_bombeList, 8, 0);
    vLayout->AddObject(m_stageList, 9, 0);

    UIGridLayout* hLayout = new UIGridLayout(m_scene, 1, 2);
    hLayout->SetSpacing(20.f);
    hLayout->SetAnchor(Anchor::CENTER);
    hLayout->SetColumnSize(100.f);

    hLayout->AddObject(m_backButton, 0, 0);
    hLayout->AddObject(m_startButton, 0, 1);

    vLayout->AddObject(hLayout, 10, 0);

    vLayout->Update();
    hLayout->Update();

    //--------------------------------------------------------------------------
    // Fade In/Out et Initialisation

    InitFadeAnim();
    InitPageWithConfig();

    //--------------------------------------------------------------------------
    // Navigation

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        m_group->AddSelectable(m_playerLists[i]);
    }
    m_group->AddSelectable(m_timeList);
    m_group->AddSelectable(m_potionList);
    m_group->AddSelectable(m_bombeList);
    m_group->AddSelectable(m_stageList);
    m_group->AddSelectable(m_backButton);
    m_group->AddSelectable(m_startButton);

    m_group->ComputeAutoNavigation();

    m_group->SetSelected(m_playerLists[0]);
    m_group->SetCursorOnSelected();
}

UIStagePage::~UIStagePage()
{
}

void UIStagePage::Update()
{
    UIObject::Update();

    if (IsFadingIn() || IsFadingOut()) return;

    TitleManager* titleManager = TitleManager::GetFromScene(m_scene);
    if (titleManager == nullptr) return;

    AssetManager* assets = m_scene->GetAssetManager();
    InputManager* inputManager = m_scene->GetInputManager();
    ApplicationInput* applicationInput = ApplicationInput::GetFromManager(inputManager);

    for (const UIInput& input : applicationInput->uiInputs)
    {
        if (input.start)
        {
            m_scene->GetSceneManager()->QuitScene();

            assets->PlaySoundFX(SFX_UI_VALIDATE);

            UpdateConfigs();
            m_group->SetSelected(nullptr);
            m_group->SetInteractable(false);

            titleManager->QuitScene();
        }
    }
}

void UIStagePage::OnClick(UISelectable* which)
{
    TitleManager* titleManager = TitleManager::GetFromScene(m_scene);
    if (titleManager == nullptr) return;

    if (which == m_startButton)
    {
        UpdateConfigs();
        m_group->SetSelected(nullptr);
        m_group->SetInteractable(false);

        titleManager->QuitScene();
    }
    else if (which == m_backButton)
    {
        UpdateConfigs();
        PlayFadeOut();
        m_group->SetSelected(nullptr);
        m_group->SetInteractable(false);

        titleManager->OpenPage(TitleManager::Page::TITLE);
    }
}

void UIStagePage::OnItemChanged(UISelectable* which, int itemIdx, int prevItemIdx, bool increase)
{
    UpdateConfigs();
}

void UIStagePage::OnFadeOutEnd(UIObject* which)
{
    if (which != this) return;

    TitleManager* titleManager = TitleManager::GetFromScene(m_scene);
    if (titleManager == nullptr) return;

    titleManager->QuitPage(TitleManager::Page::STAGE);
}

void UIStagePage::InitFadeAnim()
{
    UIFadeDef fadeDef;
    fadeDef.Reset(1.f);
    fadeDef.fadeOpacity = true;

    SetFadeInAnimation(fadeDef);
    SetFadeOutAnimation(fadeDef);
    SetFadeChildren(false);
}

void UIStagePage::InitPageWithConfig()
{
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        if (i >= g_gameCommon.playerCount && i >= 1)
        {
            m_playerLists[i]->SetFirstSelectedItem(6);
            continue;
        }

        int idx = 0;
        switch (g_gameCommon.playerConfigs[i].type)
        {
        default:
        case PlayerType::FIRE_KNIGHT:     idx = 0; break;
        case PlayerType::WATER_PRIESTESS: idx = 1; break;
        case PlayerType::LEAF_RANGER:     idx = 2; break;
        }
        if (g_gameCommon.playerConfigs[i].isCPU)
        {
            idx += 3;
        }
        m_playerLists[i]->SetFirstSelectedItem(idx);
    }

    switch (g_gameCommon.stageConfig.duration)
    {
    default:
    case  1: m_timeList->SetFirstSelectedItem(0); break;
    case  2: m_timeList->SetFirstSelectedItem(1); break;
    case  3: m_timeList->SetFirstSelectedItem(2); break;
    }
    /////////////////////////////////////////////////////////////////////////
    switch (g_gameCommon.stageConfig.type)
    {
    default:
    case StageConfig::Type::STAR_FIELDS:
        m_stageList->SetFirstSelectedItem(0); break;
    case StageConfig::Type::NEW_WORLD:
        m_stageList->SetFirstSelectedItem(1); break;
    }
    switch (g_gameCommon.stageConfig.potionFrequency)
    {
    default:
    case  StageConfig::Frequency::NEVER:
        m_potionList->SetFirstSelectedItem(0); break;
    case StageConfig::Frequency::RARELY:
        m_potionList->SetFirstSelectedItem(1); break;
    case  StageConfig::Frequency::SOMETIMES:
        m_potionList->SetFirstSelectedItem(2); break;
    case StageConfig::Frequency::OFTEN:
        m_potionList->SetFirstSelectedItem(3); break;
    }

    ///////////////////////////////////////////////////////////////////////
    // TODO : Gï¿½rer la frï¿½quence de la potion
    switch (g_gameCommon.stageConfig.bombsFrequency)
    {
    default:
    case  StageConfig::Frequency::NEVER:
        m_bombeList->SetFirstSelectedItem(0); break;
    case StageConfig::Frequency::RARELY:
        m_bombeList->SetFirstSelectedItem(1); break;
    case  StageConfig::Frequency::SOMETIMES:
        m_bombeList->SetFirstSelectedItem(2); break;
    case StageConfig::Frequency::OFTEN:
        m_bombeList->SetFirstSelectedItem(3); break;
    }
}

void UIStagePage::UpdateConfigs()
{
    int activeCount = 0;
    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        int sel = m_playerLists[i]->GetSelectedItem();
        if (sel == 6) continue;

        PlayerConfig& cfg = g_gameCommon.playerConfigs[activeCount];
        cfg.teamID = activeCount;
        cfg.playerID = activeCount;
        cfg.skinID = activeCount % 2;
        cfg.isCPU = (sel >= 3);
        switch (sel % 3)
        {
        default:
        case 0: cfg.type = PlayerType::FIRE_KNIGHT; break;
        case 1: cfg.type = PlayerType::WATER_PRIESTESS; break;
        case 2: cfg.type = PlayerType::LEAF_RANGER; break;
        }
        activeCount++;
    }
    g_gameCommon.playerCount = activeCount;

    switch (m_timeList->GetSelectedItem())
    {
    default:
    case 0: g_gameCommon.stageConfig.duration = 1; break;
    case 1: g_gameCommon.stageConfig.duration = 2; break;
    case 2: g_gameCommon.stageConfig.duration = 3; break;
    }
    switch (m_stageList->GetSelectedItem())
    {
    default:
    case 0: g_gameCommon.stageConfig.type = StageConfig::Type::STAR_FIELDS; break;
    case 1: g_gameCommon.stageConfig.type = StageConfig::Type::NEW_WORLD; break;
    }
    //////////////////////////////////////////////////////////////////////
    switch (m_potionList->GetSelectedItem())
    {
    default:
    case 0: g_gameCommon.stageConfig.potionFrequency = StageConfig::Frequency::NEVER; break;
    case 1: g_gameCommon.stageConfig.potionFrequency = StageConfig::Frequency::RARELY; break;
    case 2: g_gameCommon.stageConfig.potionFrequency = StageConfig::Frequency::SOMETIMES; break;
    case 3: g_gameCommon.stageConfig.potionFrequency = StageConfig::Frequency::OFTEN; break;
    }
    ////////////////////////////////////////////////////////////////////////////
    switch (m_bombeList->GetSelectedItem())
    {
    default:
    case 0: g_gameCommon.stageConfig.bombsFrequency = StageConfig::Frequency::NEVER; break;
    case 1: g_gameCommon.stageConfig.bombsFrequency = StageConfig::Frequency::RARELY; break;
    case 2: g_gameCommon.stageConfig.bombsFrequency = StageConfig::Frequency::SOMETIMES; break;
    case 3: g_gameCommon.stageConfig.bombsFrequency = StageConfig::Frequency::OFTEN; break;
    }
    g_gameCommon.UpdatePlayerConfigs();

    // TODO : Gï¿½rer la frï¿½quence de la potion
}
