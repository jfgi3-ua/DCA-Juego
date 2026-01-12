# Creación de distribuibles multiplataforma con CPack
## Requisitos previos
Para poder hacer nuestro proyecto instalable en varios sistemas operativos, requerimos de un build system, en este caso, usaremos CMake.

Actualmente tenemos ya de un fichero CMake configurado para poder compilar tanto en Windows como en Unix.

Para no añadir todo el código tal como el enlace a las librerías de raylib, los ficheros del proyecto y las características de este. Vamos a añadir la condición que considero más importante del proyecto, detecta el sistema en el cual se está compilando y ejecuta la instrucción necesaria.
```c++
# Configuración multiplataforma
if (WIN32)
    message(STATUS "Configurando para Windows")
    # En Windows, raylib trae soporte para WinMM, gdi32, etc.
    target_link_libraries(game PRIVATE raylib winmm gdi32 opengl32 user32)
elseif (UNIX)
    message(STATUS "Configurando para Linux")
    # En Linux, raylib necesita enlazar con librerías del sistema
    target_link_libraries(game PRIVATE m pthread GL X11)
endif()
```

## Aplicación de instalación multiplataforma
Una vez tengamos la compilación de los ejecutables, necesitamos que creen los instaladores, para que los usuarios puedan instalar el proyecto de una forma cómoda y sencilla.

Para ello, usamos la misma condición que la compilación.

```c++
# Configuración específica por plataforma
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
endif()

include(CPack)
```

## Instrucciones de instalación
### CMake
#### Linux
```
cmake -S . -B build
cmake --build build --config Release
cd build
cpack
```

### Windows
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
cd build
cpack -C Release
```

#### De esta forma generaremos para:
- Linux, un archivo .deb
- Windows, un archivo .exe