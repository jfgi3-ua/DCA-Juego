#include "raylib_stubs.hpp"

// Incluimos raylib para igualar firmas de funciones.
extern "C" {
    #include <raylib.h>
}

namespace {
    int g_load_calls = 0;
    int g_unload_calls = 0;
}

int RaylibStub_GetLoadTextureCalls() {
    return g_load_calls;
}

int RaylibStub_GetUnloadTextureCalls() {
    return g_unload_calls;
}

void RaylibStub_ResetCounters() {
    g_load_calls = 0;
    g_unload_calls = 0;
}

/* --- Stubs de raylib ---
 * En CI/headless no queremos enlazar ni ejecutar GPU, así que las
 * sustituimos por stubs que no hacen nada y nos permiten contar llamadas.
 */
extern "C" Texture2D LoadTexture(const char* fileName) {
    (void)fileName;
    ++g_load_calls;

    Texture2D tex{};
    tex.id = 1;
    tex.width = 0;
    tex.height = 0;
    tex.mipmaps = 1;
    tex.format = 0;
    return tex;
}

extern "C" void UnloadTexture(Texture2D texture) {
    (void)texture;
    ++g_unload_calls;
}

extern "C" void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest,
                               Vector2 origin, float rotation, Color tint) {
    (void)texture;
    (void)source;
    (void)dest;
    (void)origin;
    (void)rotation;
    (void)tint;
}
