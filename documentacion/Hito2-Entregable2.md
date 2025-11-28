Entregable 2 – Empaquetado de Software

## 1. Sistema de empaquetado elegido

Hemos elegido .deb como sistema de paquetes. 


## 2. Reglas añadidas al Makefile

Se ha añadido al Makefile del Entregable 1 las reglas install y dist.


`install`
: Crea la estructura de directorios que utiliza el empaquetador, mediante `DESTDIR`:

  ```bash
  make install DESTDIR=debian/game/
  ```


Esto genera el “staging area” donde se coloca el ejecutable y los assets siguiendo el estándar FHS:

```bash
usr/bin/game
usr/share/game/assets/...
```

`dist`
: Se encarga de la creación del paquete ejecutando para generar el archivo .deb el siguiente comando:

  ```bash
    dpkg-buildpackage -us -uc -b
  ```


## 3. Generación e instalación del paquete

El resultado obtenido al hacer `make dist` es el .deb `../game_1.0-1_amd64.deb` cuya instalcion en nuestro sistema hacemos mediante el comadno:

```bash
  sudo dpkg -i ../game_1.0-1_amd64.deb
```

Una vez instalado lo ejecutamos con `game`.

Para su desinstalación usamos:

```bash
  sudo dpkg -r game
```

## 4. GitHub Action para empaquetado automático

Hemos creado la acción `.github/workflows/build-deb.yml`.


Esta acción:

1. Se activa automáticamente cuando se crea un release.
2. Compila el proyecto.
3. Ejecuta dpkg-buildpackage.
4. Adjunta el .deb generado al release.


## 5. Pasos para probar el empaquetado

**1. Compilar el proyecto**
  ```bash
    make
  ```

**2. Generar el paquete .deb**
  ```bash
    make dist
  ```

El paquete aparecerá en el directorio padre, normalmente como:

`../game_1.0-1_amd64.deb`

**3. Instalar el paquete en el sistema**
  ```bash
    sudo dpkg -i ../game_1.0-1_amd64.deb
  ```
  
**4. Ejecutar el juego instalado**
  ```bash
    game
  ```

**5. Desinstalar el paquete**
  ```bash
    sudo dpkg -r game
  ```
**6. Generar automáticamente el .deb con GitHub Actions**

1. Crear un tag:

  ```bash
    git tag v1.0
    git push origin v1.0
  ```

2. La acción se ejecutará automáticamente en GitHub y añadirá el .deb al release correspondiente.