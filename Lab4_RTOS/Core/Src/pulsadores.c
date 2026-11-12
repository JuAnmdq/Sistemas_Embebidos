#include "pulsadores.h"
#include "main.h"

#define T_REBOTE_MS 20U

typedef struct {
    volatile uint32_t instante_ms;
    // Acumula los flancos desde el arranque para detectar nuevos durante la validación
    volatile uint32_t flancos_registrados;
    uint32_t ultimo_flanco_atendido;
} PulsadorExti;

static PulsadorExti pulsador_sw1;
static PulsadorExti pulsador_sw2;

void RegistrarInterrupcionPulsador(uint16_t pin)
{
    if (pin == SW1_Pin) {
        pulsador_sw1.instante_ms = HAL_GetTick();
        pulsador_sw1.flancos_registrados++;
    } else if (pin == SW2_Pin) {
        pulsador_sw2.instante_ms = HAL_GetTick();
        pulsador_sw2.flancos_registrados++;
    }
}

static bool EsPulsacionValida(PulsadorExti *pulsador, GPIO_TypeDef *puerto, uint16_t pin)
{
    bool pulsacion_valida = false;
    uint32_t flanco = pulsador->flancos_registrados;
    uint32_t instante_ms = pulsador->instante_ms;

    if (flanco != pulsador->ultimo_flanco_atendido &&
        HAL_GetTick() - instante_ms >= T_REBOTE_MS) {
        GPIO_PinState lectura = HAL_GPIO_ReadPin(puerto, pin);

        // Valida la pulsación si no llegó otro flanco durante la lectura del GPIO
        if (flanco == pulsador->flancos_registrados) {
            pulsacion_valida = lectura == GPIO_PIN_RESET;
            pulsador->ultimo_flanco_atendido = flanco;
        }
    }

    return pulsacion_valida;
}

bool ProcesarPulsacionExtiSW1(void)
{
    return EsPulsacionValida(&pulsador_sw1, SW1_GPIO_Port, SW1_Pin);
}

bool ProcesarPulsacionExtiSW2(void)
{
    return EsPulsacionValida(&pulsador_sw2, SW2_GPIO_Port, SW2_Pin);
}
