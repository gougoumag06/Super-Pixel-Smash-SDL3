/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "common/game_settings.h"
#include "common/game_common.h"

#include "ui/stage/ui_stage_hud.h"
#include "ui/stage/ui_pause_menu.h"
#include "ui/stage/ui_end.h"
#include "ecs/common/terrain_utils.h"

class StageManager : public SceneManager
{
public:
    StageManager(InputManager *inputManager);
    virtual ~StageManager();

    virtual void OnSceneUpdate() override;
    virtual void OnSceneFixedUpdate() override;

    enum class State : int
    {
        INTRODUCTION = 0, FIGHT, STAGE_COMPLETE, STATS
    };

    void QuitPause();
    float GetRemainingTime() const;
    bool IsPaused() const;
    bool IsEnd() const;

    static StageManager *GetFromScene(Scene *scene);

private:
    void InitStarFields();
    void InitNewWorld();

    void AddPotion();
    void AddBomb();
  /*  void AddCrystal();*/

    State m_state;

    OneWayCallback m_oneWayCallback;

    UIPauseMenu *m_pauseMenu;
    UIEndMenu* m_endMenu;
    UIStageHUD *m_stageHUD;
    bool m_paused;
    bool m_end;
    bool ilespown;
  /*  bool crystalspown;*/

    float m_delayStage;
    float m_delayBomb;
    //float m_delayCrystal;
    float m_delayPotion;

};

inline float StageManager::GetRemainingTime() const
{
    return m_delayStage;
}

inline bool StageManager::IsPaused() const
{
    return m_paused;
}
inline bool StageManager::IsEnd() const
{
    return m_end;
}
