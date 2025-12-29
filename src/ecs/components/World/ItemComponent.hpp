#pragma once

// Componente de Item (Llaves, etc.)
struct ItemComponent {
    bool isKey;
    int value;
    bool collected;

    ItemComponent(bool key = true) : isKey(key), value(1), collected(false) {}
};