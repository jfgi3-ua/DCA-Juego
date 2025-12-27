#pragma once

// Componente de Pinchos retractiles
struct SpikeComponent {
    bool active;
    float interval;
    float activeOffsetY;
    float inactiveOffsetY;

    SpikeComponent(bool isActive = true, float intervalSeconds = 3.0f,
                   float activeOffset = 0.0f, float inactiveOffset = 0.0f)
        : active(isActive),
          interval(intervalSeconds),
          activeOffsetY(activeOffset),
          inactiveOffsetY(inactiveOffset) {}
};
