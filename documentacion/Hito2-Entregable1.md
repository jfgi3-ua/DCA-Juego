Entregable 1 — Makefiles, Paralelismo y ccache

## 1. Compilación con `-jN`

Se han medido los tiempos de compilación usando distintos valores de `N` en `make -jN`, sin ccache.

| N (jobs) | Tiempo real (s) | Comentario                            |
| -------: | --------------: | ------------------------------------- |
|        1 |           3.763 | Compilación secuencial (referencia)   |
|        2 |           2.208 | Mejora significativa                  |
|        4 |           1.401 | Punto óptimo para esta máquina        |
|        8 |           1.261 | Leve mejora respecto a 4              |
|       16 |           1.185 | Mejora mínima; saturación de recursos |

**Conclusión:** aumentar el número de trabajos mejora el tiempo hasta aproximadamente el número de **núcleos** de la máquina.
En este equipo, a partir de `N = 4` las mejoras dejan de ser sustanciales.

---

## 2. ¿Hay fallos de compilación con ejecuciones en paralelo?

No se han observado fallos al compilar con `make -j2`, `-j4`, `-j8` ni `-j16`.

El Makefile es seguro en paralelo porque:

* Cada archivo `.cpp` genera **un único** `.o` independiente dentro de `obj/`.
* El binario final depende de que **todos** los `.o` estén creados.
* No se generan ficheros temporales compartidos entre reglas.
* La estructura `obj/%.o : src/%.cpp` evita condiciones de carrera.

**Conclusión:** la compilación paralela funciona correctamente sin errores.

---

## 3. Comparación con y sin ccache

Las siguientes pruebas se realizaron usando `make -j4`.

| Configuración             | Tiempo real (s) | Comentario                                      |
| ------------------------- | --------------: | ----------------------------------------------- |
| Sin ccache                |           1.263 | Compilación completa                            |
| Con ccache (1ª ejecución) |           1.561 | Similar a sin ccache; la caché se está llenando |
| Con ccache (2ª ejecución) |           0.017 | Extremadamente rápida; se reutilizan resultados |

**Conclusión:**
`ccache` reduce de forma drástica el tiempo de compilación cuando repetimos builds sin cambios en el código, siendo especialmente útil durante el desarrollo iterativo.

---

## 4. Estadísticas de ccache

Pasos realizados:

1. Limpieza de estadísticas y caché:

   ```bash
   ccache -C
   ccache -z
   ```
2. Primera compilación (`make -j4 USE_CCACHE=1`):

   * Caché vacía ⇒ predominan los *cache miss*.
   * Estadísticas:

     ```
     Cacheable calls:     11 / 12 (91.67%)
       Hits:               0 / 11 (0.00%)
       Misses:            11 / 11 (100.0%)
     Uncacheable calls:    1 / 12 (8.33%)
     ```
3. Segunda compilación sin cambios:

   * La caché ya tiene resultados ⇒ aumentan los *cache hits*.
   * Estadísticas:

     ```
     Cacheable calls:     22 / 24 (91.67%)
       Hits:              11 / 22 (50.00%)
         Direct:          11 / 11 (100.0%)
       Misses:            11 / 22 (50.00%)
     Uncacheable calls:    2 / 24 (8.33%)
     ```

**Conclusión:**
Tras la primera compilación, `ccache` almacena los objetos generados.
En la segunda ejecución, el número de *hits* aumenta y el tiempo se reduce de forma notable, lo que confirma el buen funcionamiento de la herramienta.
