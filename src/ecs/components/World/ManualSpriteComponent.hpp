#pragma once
extern "C" {
  #include <raylib.h>
}

//Componente de sprites estaticos (sin animacion)
//calculados a mano, no en base a una rejilla
//activo/inactivo (spikes y mecanismos) o fijo
struct ManualSpriteComponent {
  Rectangle src;
  Rectangle srcActive;
  Rectangle srcInactive;

  // Constructor para sprite FIJO como mapa
  explicit ManualSpriteComponent(Rectangle fixed)
      : src(fixed),
        srcActive({0,0,0,0}),
        srcInactive({0,0,0,0}) {}

  // Constructor para sprite ACTIVO / INACTIVO como spikes y mecanismos
  ManualSpriteComponent(Rectangle active, Rectangle inactive)
      : src({0,0,0,0}),
        srcActive(active),
        srcInactive(inactive) {}
};