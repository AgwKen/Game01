#pragma once
#include <DirectXMath.h>

class UIFont;

// Simple soccer HUD (time/goals/prompt) draw module.
// Call Initialize once, Update each frame (optional), Draw in RenderPass_UI.
namespace MatchHUDUI
{
    void Initialize(UIFont* font);
    void Finalize();

    void Update(float dt);

    // Draw HUD.
    // timeLeftSec: remaining time in seconds
    // goals: current goals
    // runActive: Run_IsActive()
    // runFinished: Run_IsFinished()
    void Draw(float timeLeftSec, int goals, bool runActive, bool runFinished);
}
