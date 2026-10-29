# Laboratorio 3 Temporizador programable

Sistemas Embebidos, Facultad de Ingeniería, UNMdP. Temporizador regresivo controlado por pulsadores y UART, con display y alarma mediante PWM.

## Organización

- `Core/Src/main.c`: inicialización, polling del procesamiento y callbacks HAL.
- `Core/Src/uart.c`: recepción por interrupción, buffer circular, armado e interpretación de comandos.
- `Core/Src/temporizador.c`: cuenta, estado y alarma.
- `Core/Src/pulsadores.c`: EXTI de SW1/SW2 y antirrebote desde el main.
- `Core/Src/display.c`: representación MM.SS y refresco multiplexado.
- `Lab3_Temporizador.ioc`: configuración de GPIO, EXTI, TIM2, TIM9 y USART3.

## Uso

USART3 utiliza **115200 baudios, 8 bits, sin paridad, 1 bit de stop y sin control de flujo**. PB10 es TX y PB11 es RX. Conectar un adaptador USB-UART de niveles compatibles con el STM32, cruzando TX/RX y compartiendo GND.

Enviar cada comando en mayúsculas y finalizarlo con Enter (`\r`, `\n` o `\r\n`):

| Comando | Acción |
| --- | --- |
| `SET 120` | Configura 120 segundos y deja la cuenta pausada; admite de 0 a 5999 |
| `START` | Inicia si quedan segundos |
| `PAUSE` | Pausa conservando el tiempo restante |
| `RESET` | Detiene la cuenta, apaga la alarma y deja el tiempo en cero |
| `STATUS` | Informa el estado y el tiempo restante en MM.SS |

SW1 (PA2) alterna ejecución y pausa. SW2 (PA4) reinicia cuando la cuenta está pausada. Al llegar a cero se activa el buzzer y se envía `ALARM` por UART. `RESET` o un nuevo `SET` apagan la alarma; `PAUSE` conserva su estado.

TIM2 interrumpe cada 2 ms; 500 interrupciones completan un segundo. TIM9 canal 2 genera PWM en PA3: período 56817 y pulso 28409, con ciclo de trabajo del 50 % y frecuencia aproximada de 440 Hz. El display y el buzzer requieren el montaje externo correspondiente.

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
