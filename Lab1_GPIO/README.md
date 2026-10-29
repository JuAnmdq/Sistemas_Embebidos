# Laboratorio 1 GPIO

Sistemas Embebidos, Facultad de Ingeniería, UNMdP. Control de tres LEDs mediante pulsadores consultados por polling, con antirrebote de 20 ms.

## Organización

- `Core/Src/main.c`: inicialización y ciclo principal.
- `Core/Src/pulsadores.c`: detección y procesamiento de nuevas pulsaciones.
- `Lab1_GPIO.ioc`: GPIO y configuración del microcontrolador.

## Uso

Cada nueva pulsación conmuta el LED correspondiente. Los pulsadores son activos en bajo: PA2, PA4 y PA5. Las salidas están en PD8, PD11 y PD9, respectivamente. El montaje externo debe corresponder a las conexiones de la cátedra.

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
