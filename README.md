# Sistemas Embebidos

Laboratorios de Sistemas Embebidos, Facultad de Ingeniería, UNMdP. Un único repositorio conserva los proyectos de STM32CubeIDE y su evolución.

| Carpeta | Contenido | Estado |
| --- | --- | --- |
| [Lab1_GPIO](Lab1_GPIO/) | GPIO, pulsadores por polling y antirrebote | Compilación limpia verificada |
| [Lab2_Cronometro](Lab2_Cronometro/) | Cronómetro, TIM2, display y pulsadores por EXTI | Compilación limpia verificada |
| [Lab3_Temporizador](Lab3_Temporizador/) | Temporizador programable, UART, buffer circular y alarma PWM | Compilación limpia verificada |
| [Lab4_RTOS](Lab4_RTOS/) | CMSIS-RTOS2 sobre FreeRTOS: bloqueo, prioridades y periodicidad | Hitos 1 y 2 verificados en placa; restantes pendientes |

## Abrir los proyectos

1. Clonar este repositorio.
2. En STM32CubeIDE, abrir **File → Import → General → Existing Projects into Workspace**.
3. Seleccionar la raíz clonada y activar **Search for nested projects**.
4. Importar Lab1, Lab2, Lab3 y Lab4 y compilar cada uno con **Project → Build Project**, configuración **Debug**.

También se puede importar cada carpeta por separado. Los README de los laboratorios describen su funcionamiento y montaje. El README de Lab4 describe las tareas del Hito 1 y las pruebas manuales del Hito 2, organizadas en el módulo `tareas_rtos`.

Se incluyen `.ioc`, fuentes, HAL/CMSIS, scripts de linker y configuración del IDE. Los tres proyectos se compilaron desde copias limpias con STM32CubeIDE 2.2.0, sin errores ni advertencias. Esa verificación no reemplaza la prueba en placa.

## Trabajar desde el workspace

En el workspace actual de STM32CubeIDE, las carpetas de Lab1 a Lab4 pertenecen a este mismo repositorio. Los metadatos internos del workspace, las salidas de compilación y los proyectos ajenos a estos laboratorios quedan fuera de Git.

Desde la raíz del workspace:

```sh
git status
git add Lab1_GPIO Lab2_Cronometro Lab3_Temporizador Lab4_RTOS
git diff --staged
git commit -m "Descripción del cambio"
git push
```

Git registra los cambios localmente; GitHub recibe los commits al ejecutar `git push`. Los archivos sin commit o sin push todavía no están sincronizados con el remoto. La rama principal es `main`.

Para actualizar una copia limpia desde GitHub:

```sh
git pull --ff-only
```

Las versiones iniciales de los tres laboratorios se conservaron en el historial. Lab4 continuará aquí, sin necesitar otro repositorio. Las licencias de terceros permanecen dentro de los directorios `Drivers/` de cada proyecto.
