# Laboratorio 2 Cronómetro

Sistemas Embebidos, Facultad de Ingeniería, UNMdP. Cronómetro con pulsadores por interrupción externa y display multiplexado, con resolución de centésimas.

## Organización

- `Core/Src/main.c`: inicialización, ciclo principal y callbacks HAL.
- `Core/Src/pulsadores.c`: interrupciones de SW1/SW2 y validación del antirrebote desde el main.
- `Core/Src/cronometro.c`: cuenta y control del cronómetro.
- `Core/Src/display.c`: conversión y refresco de los cuatro dígitos.
- `Lab2_Cronometro.ioc`: configuración de GPIO, EXTI y TIM2.

## Uso

SW1 (PA2) alterna ejecución y pausa. SW2 (PA4) reinicia cuando la cuenta está pausada. El display muestra segundos y centésimas, y la cuenta vuelve a cero al completar un minuto.

TIM2 interrumpe cada 2 ms. Cada interrupción refresca un dígito; cinco interrupciones completan una centésima. El display y los pulsadores externos requieren el montaje de la cátedra.

## Abrir y compilar

1. Clonar el repositorio.
2. En STM32CubeIDE: **File → Import → General → Existing Projects into Workspace** y seleccionar la carpeta clonada.
3. Seleccionar el proyecto y ejecutar **Project → Build Project** con la configuración **Debug**.
4. Para ejecutar o depurar, conectar la placa por ST-LINK y crear la configuración de depuración de STM32.

Proyecto para **STM32F407VGT6**. Se incluyen los fuentes, los drivers HAL/CMSIS, los scripts del linker y la configuración del IDE. El archivo `.ioc` conserva la configuración de CubeMX. Referencias del proyecto: STM32CubeIDE 2.2.0, CubeMX 6.18.1 y STM32Cube FW_F4 V1.28.3.

Los archivos de `Debug/` y la configuración local de depuración se generan en cada computadora y quedan fuera de Git.

## Versionado

La rama principal es `main`. Para registrar cambios:

```sh
git status
git add <archivos>
git diff --staged
git commit -m "Descripción del cambio"
git push
```

Las licencias del código de terceros se conservan dentro de `Drivers/`.
