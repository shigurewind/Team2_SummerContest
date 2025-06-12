#pragma once






struct DebugCameraControl
{
    float pos[3] = { 0.0f, 0.0f, 0.0f };
    float rot[3] = { 0.0f, 0.0f, 0.0f };
    bool freezeTime = false;
};

struct DebugModelControl
{
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 1.0f, 1.0f, 1.0f };
};


void ShowDebugUI();