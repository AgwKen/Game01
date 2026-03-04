#pragma once

struct RunResult;
class UIFont;

// Initialize with a font pointer (use your g_Font)
void NameEntry_Initialize(UIFont* font);

// Start name entry if qualifies Top10
void NameEntry_BeginIfQualifies(const RunResult& r);

// Update input (keyboard)
void NameEntry_Update();

// Draw overlay UI
void NameEntry_Draw();

// Is overlay active right now?
bool NameEntry_IsActive();
