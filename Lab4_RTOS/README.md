# Laboratorio 4 RTOS

CMSIS-RTOS2 sobre FreeRTOS en STM32F407G-DISC1. El Hito 1 conserva tres instancias de una misma función y el Hito 2 agrega variantes para elegirlas a mano.

## Organización

`Core/Src/main.c` inicializa los periféricos y mantiene explícito el recorrido `osKernelInitialize()` → `CrearTareas()` → `osKernelStart()`.

`Core/Src/tareas_rtos.c` contiene los parámetros, atributos, creación y comportamiento de las tareas. `Core/Inc/tareas_rtos.h` declara `CrearTareas()`. Es código de aplicación separado de la plantilla `freertos.c` generada por CubeMX y del kernel ubicado en `Middlewares/Third_Party/FreeRTOS`.

Cada LED tiene su nombre para el debugger (`LED_Rojo`, `LED_Amarillo`, `LED_Verde`), una pila de 512 bytes y prioridad base Normal en sus atributos. Para las pruebas del Hito 2, la prioridad de la roja se eleva después de crearla.

El Hito 1 utiliza tres instancias de `StartDefaultTask()`, sin duplicar su implementación:

```c
defaultTaskHandle = osThreadNew(StartDefaultTask, &params_LED_rojo, &atributos_LED_rojo);
osThreadNew(StartDefaultTask, &params_LED_amarillo, &atributos_LED_amarillo);
osThreadNew(StartDefaultTask, &params_LED_verde, &atributos_LED_verde);
```

Hito 1: rojo PD14, 250 ticks; amarillo (LED naranja) PD13, 500 ticks; verde PD12, 125 ticks entre cambios. Cada instancia ejecuta el mismo código, con su propio parámetro y contexto.

FreeRTOS usa SysTick a 1000 Hz: **1 tick del kernel = 1 ms**, no un ciclo de reloj de la CPU. La HAL usa TIM6.

## Qué se demuestra en el Hito 2

El recuadro del enunciado solicita:

- **(a)** El efecto de una tarea de mayor prioridad que permanece Ready y su corrección mediante bloqueo
- **(b)** La diferencia entre retardo relativo y activación periódica usando `osDelay()` y `osDelayUntil()`

Los resultados se explican en términos de **estados, prioridades y periodicidad**. La prueba permite observar o medir la diferencia sin requerir una caracterización metrológica precisa.

## Elegir la prueba a mano

La selección está en `CrearTareas()`, dentro de `tareas_rtos.c`. Se cambia comentando y descomentando llamadas, sin flags ni condicionales. Para probar el Hito 2:

1. Comentar la creación original del rojo, que usa `osThreadNew(StartDefaultTask, ...)`
2. Descomentar **una sola** de estas creaciones del rojo:

```c
// defaultTaskHandle = osThreadNew(TareaRojaEsperaActiva, &params_LED_rojo, &atributos_LED_rojo);
// defaultTaskHandle = osThreadNew(TareaRojaRetardoRelativo, &params_LED_rojo, &atributos_LED_rojo);
// defaultTaskHandle = osThreadNew(TareaRojaInstantesAbsolutos, &params_LED_rojo, &atributos_LED_rojo);
```

3. Descomentar `osThreadSetPriority(defaultTaskHandle, osPriorityHigh);` para dar prioridad alta al rojo
4. Guardar, compilar con **Project → Build Project**, configuración **Debug**, y cargar con el lanzador original **Lab4_RTOS Debug**
5. Para comparar otra espera, comentar la creación del rojo usada y descomentar la siguiente; compilar y cargar otra vez

Mantener activas las dos creaciones originales de amarillo y verde. Deben existir exactamente **tres tareas de aplicación**, incluyendo una sola roja. No ejecutar varias esperas juntas ni crear varias tareas rojas.

Para volver al Hito 1: dejar activa la creación original del rojo y comentadas las tres alternativas y la prioridad alta. Los parámetros del Hito 1 permanecen iguales. El rojo del Hito 2 usa el trabajo de 500 ticks y el intervalo de 1000 definidos en sus funciones.

La llamada a `CrearTareas()` está dentro de `USER CODE BEGIN RTOS_THREADS` en `main.c`. El `.ioc` no declara una tarea automática: las tareas de aplicación se crean en el módulo propio. Si el depurador se detiene en un breakpoint, continuar con Resume.

## 4.1 Espera activa y bloqueo

Partimos del Hito 1: cada tarea cambia su LED y espera. Comparamos cómo espera el rojo con prioridad alta.

```c
// Con prioridad alta, sigue Ready/Running y acapara la CPU (starvation)
HAL_Delay(1000);
```

`HAL_Delay()` consulta el tiempo sin bloquear la tarea. Rojo sigue ejecutable y el scheduler lo elige por su prioridad; amarillo y verde quedan **Ready** sin recibir CPU: starvation. Las interrupciones siguen ocurriendo.

```c
// Pasa a Blocked, independientemente de la prioridad, permitiendo que las demás tareas ejecuten
osDelay(1000);
```

Rojo queda **Blocked** y permite ejecutar a las demás. Al vencer la espera vuelve a **Ready**; por su prioridad alta puede retomar **Running**. La prioridad importa entre tareas listas, no mientras están bloqueadas.

Con prioridades iguales puede haber turnos entre tareas Ready. En esta prueba, la starvation la provoca una tarea continuamente ejecutable con prioridad superior.

## 4.2 Retardo relativo e instantes absolutos

Partimos del ejercicio 6 del primer parcial de 2026, manteniendo `Procesar()` y `tref`. Para poder verlo a simple vista, ampliamos los tiempos: **C = 500 ticks (0,5 s)** y **T = 1000 ticks (1 s)**.

`Procesar()` simula trabajo manteniendo ocupada la CPU hasta que avanzan 500 ticks del kernel. Es un trabajo simulado y acotado; después, la espera bloqueante deja ejecutar a las demás tareas. Como rojo tiene prioridad alta, amarillo y verde pueden demorarse durante ese trabajo.

**Retardo relativo:** después de `Procesar()`, `osDelay(1000)` espera 1000 ticks desde la llamada. El trabajo se suma en cada vuelta: aproximadamente **C + T = 1500 ticks** entre cambios del rojo.

**Activación periódica en instantes absolutos:** antes del `while`, se toma una sola referencia:

```c
uint32_t tref = osKernelGetTickCount();
```

Después de `Procesar()`, en cada vuelta:

```c
tref += 1000;
osDelayUntil(tref);
```

Los objetivos quedan en `t0 + 1000`, `t0 + 2000`, etc. Los 500 ticks de trabajo quedan dentro de cada intervalo de 1000. **Actualizar `tref` modifica un número; no agrega otra espera.**

`osDelay()` recibe un intervalo; `osDelayUntil()` recibe un tick puntual. Ambas pasan la tarea a Ready al vencer la espera; el scheduler decide cuándo vuelve a Running. La referencia absoluta se conserva, pero una tarea de mayor prioridad o una interrupción pueden retrasar la ejecución.

## Observación a simple vista o con cronómetro

Comparar el rojo manteniendo el mismo trabajo, prioridad e intervalo en ambas variantes:

| Espera | Tiempo entre cambios del LED | Ciclo completo encendido-apagado |
| --- | --- | --- |
| `osDelay(1000)` | Aproximadamente 1,5 s | Aproximadamente 3 s |
| `osDelayUntil(tref)` | Aproximadamente 1 s | Aproximadamente 2 s |

Un Toggle cambia el estado; **un ciclo completo necesita dos cambios**. Para observarlo, tomar un encendido como inicio y medir hasta el siguiente encendido. Repetir tres ciclos completos da aproximadamente **9 s con retardo relativo** y **6 s con instantes absolutos**. Esta comparación satisface el objetivo conceptual del enunciado sin instrumental de precisión.

## Resultados y evidencias

- **4.1, HAL_Delay:** Con prioridad alta, sigue Ready/Running y acapara la CPU (starvation)
- **4.1, osDelay:** Pasa a Blocked, independientemente de la prioridad, permitiendo que las demás tareas ejecuten
- **4.2:** el retardo relativo suma el trabajo en cada vuelta; los instantes absolutos conservan los objetivos periódicos

Con los tiempos visibles se midieron estos comienzos:

| Variante | Comienzos medidos, en ticks |
| --- | --- |
| Relativa | 0, 1500, 3000, 4500 |
| Absoluta | 0, 1000, 2000, 3000 |

Con los tiempos originales del parcial (3 y 20 ticks), se observaron comienzos **0, 23, 46, 69** frente a **0, 20, 40, 60**.

Para repetir una medición con el depurador, colocar un breakpoint al inicio de `Procesar()`, observar `xTickCount` y continuar. Para la observación humana, ejecutar sin detenerse en breakpoints. Son comprobaciones conceptuales; no se realizó una medición externa de precisión.
