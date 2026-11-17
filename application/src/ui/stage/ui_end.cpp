/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "ui/stage/ui_end.h"
#include "scene_manager/stage_manager.h"
#include "ui/custom_default/ui_default_cursor.h"

UIEndMenu::UIEndMenu(Scene* scene)
    : UIObject(scene)
{

    SetName("END");
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
    // Title text

    font = assets->GetFont(FONT_LARGE);
    m_titleText = new UIText(scene, "END", font, Colors::Gold);
    m_titleText->SetName("UI end title");
    m_titleText->SetAnchor(Anchor::CENTER);

    //char buffer[32] = { 0 };
    //const int playerCount = g_gameCommon.playerCount;
    //for (int i = 0; i < playerCount; i++)
    //{
    //    if (g_gameCommon.playerStats[i].ejectionScore > 3)
    //    {
    //        switch (g_gameCommon.playerConfigs[i].playerID)
    //        {
    //        case 1:
    //            //sprintf_s(buffer, "Player 1 Wins !"); break;
    //            m_winText = new UIText(scene, "Player 1 Wins !", font, Colors::Gold); break;
    //        case 2:
    //            //sprintf_s(buffer, "Player 2 Wins !"); break;
    //            m_winText = new UIText(scene, "Player 2 Wins !", font, Colors::Gold); break;
    //        }
    //    }
    //}


    //

    font = assets->GetFont(FONT_LARGE);
    int player1Score = g_gameCommon.playerStats[0].fallCount;
    int player2Score = g_gameCommon.playerStats[1].fallCount;

    std::string winnerText;
    if (player1Score < player2Score) {
        winnerText = "P1 Wins!";
    }
    else if (player2Score < player1Score) {
        winnerText = "P2 Wins!";
    }
    else {
        winnerText = "égalité";
    }


    m_winText = new UIText(scene, winnerText, font, Colors::Gold);
    m_winText->SetName("UI win title");
    m_winText->SetAnchor(Anchor::CENTER);

    //--------------------------------------------------------------------------
    // Fill

    m_endFill = new UIFillRect(scene, Colors::DarkBlue);
    m_endFill->SetParent(this);
    m_endFill->SetLayer(LAYER_UI_BACKGROUND);
    m_endFill->SetOpacity(0.5f);
    m_endFill->GetLocalRect().anchorMin.y = 0.33f;
    m_endFill->GetLocalRect().anchorMax.y = 0.67f;

    //--------------------------------------------------------------------------
    // Buttons
    m_mainMenuButton = new UIDefaultButton(scene, "Quit to menu");
    m_mainMenuButton->AddSelectableListener(this);

    //--------------------------------------------------------------------------
    // Cursor

    UIDefaultCursor* cursor = new UIDefaultCursor(scene);
    m_group->SetCursor(cursor);

    //--------------------------------------------------------------------------
    // Grid layout

    UIGridLayout* vLayout = new UIGridLayout(m_scene, 8, 1);
    vLayout->SetParent(this);
    vLayout->SetAnchor(Anchor::CENTER);
    vLayout->SetSpacing(5.f);
    vLayout->SetRowSize(22.f);
    vLayout->SetColumnSize(100.f);
    vLayout->AddObject(m_titleText, 2, 0);
    vLayout->AddObject(m_winText, 3, 0);
    vLayout->AddObject(m_mainMenuButton, 5, 0);

    //--------------------------------------------------------------------------
    // Fade In/Out

    InitFadeAnim();

    //--------------------------------------------------------------------------
    // Navigation


    m_group->AddSelectable(m_mainMenuButton);

    m_group->ComputeAutoNavigation();

    m_group->SetSelected(m_mainMenuButton);
    m_group->SetCursorOnSelected();


}

void UIEndMenu::OnClick(UISelectable* which)
{
    StageManager* stageManager = StageManager::GetFromScene(m_scene);
    if (stageManager == nullptr) return;


    stageManager->QuitScene();

}

void UIEndMenu::OnPlayFadeIn()
{
    PlayFadeInAlone();

    UIObject* objects[] = {
        m_titleText, m_mainMenuButton,
        m_group->GetCursor(), m_endFill
    };
    for (UIObject* object : objects)
    {
        object->PlayFadeIn();
    }
}

void UIEndMenu::OnPlayFadeOut()
{
    PlayFadeOutAlone();

    UIObject* objects[] = {
        m_titleText, m_mainMenuButton,
        m_group->GetCursor(), m_endFill
    };
    for (UIObject* object : objects)
    {
        object->PlayFadeOut();
    }
}



void UIEndMenu::InitFadeAnim()
{
    UIFadeDef fadeDef;
    fadeDef.Reset(0.5f);

    SetFadeInAnimation(fadeDef);
    SetFadeOutAnimation(fadeDef);
    SetFadeChildren(false);

    fadeDef.Reset(0.5f);
    fadeDef.fadeOpacity = true;
    fadeDef.shift = b2Vec2(0.f, 10.f);

    m_titleText->SetFadeInAnimation(fadeDef);

    m_mainMenuButton->SetFadeInAnimation(fadeDef);
    m_group->GetCursor()->SetFadeInAnimation(fadeDef);

    fadeDef.Reset(0.5f);
    fadeDef.fadeOpacity = true;
    fadeDef.shift = b2Vec2(0.f, -10.f);

    m_titleText->SetFadeOutAnimation(fadeDef);

    m_mainMenuButton->SetFadeOutAnimation(fadeDef);
    m_group->GetCursor()->SetFadeOutAnimation(fadeDef);

    fadeDef.Reset(0.5f);
    fadeDef.fadeOpacity = true;
    m_endFill->SetFadeInAnimation(fadeDef);
    m_endFill->SetFadeOutAnimation(fadeDef);
}
