#include "pulsadores.h"
#include "main.h"
#include "cronometro.h"

#include <stdbool.h>

#define T_REBOTE_MS                  20

static volatile bool pulsacion_exti_sw1_pendiente = false;
static volatile bool pulsacion_exti_sw2_pendiente = false;
static volatile uint32_t instante_interrupcion_sw1_ms = 0;
static volatile uint32_t instante_interrupcion_sw2_ms = 0;

static void ProcesarPulsacionExtiSW1(void);
static void ProcesarPulsacionExtiSW2(void);

/*
 * Procesa desde el main las pulsaciones registradas por interrupción, con antirrebote
 * SW1 alterna ejecución y pausa; SW2 reinicia si la cuenta no está en marcha
 */
void ProcesarPulsadores(void) {
	ProcesarPulsacionExtiSW1();
	ProcesarPulsacionExtiSW2();
}

/*
 * Guarda el instante de la interrupción de SW1 o SW2
 * Deja la pulsación pendiente para que el main la valide con antirrebote
 */
void RegistrarInterrupcionPulsador(uint16_t pin) {
	if (pin == SW1_Pin) {
		instante_interrupcion_sw1_ms = HAL_GetTick();
		pulsacion_exti_sw1_pendiente = true;
	} else if (pin == SW2_Pin) {
		instante_interrupcion_sw2_ms = HAL_GetTick();
		pulsacion_exti_sw2_pendiente = true;
	}
}

/*
 * Valida por polling desde el main la pulsación pendiente de SW1 tras el antirrebote
 * Si sigue presionado, alterna la cuenta; en ambos casos, limpia el registro pendiente
 */
static void ProcesarPulsacionExtiSW1(void) {
	if (pulsacion_exti_sw1_pendiente) {
		uint32_t tiempo_transcurrido = HAL_GetTick() - instante_interrupcion_sw1_ms;

		if (tiempo_transcurrido >= T_REBOTE_MS) {
			GPIO_PinState lectura = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin);

			if (lectura == GPIO_PIN_RESET) {
				TogglearCronometro();
			}

			pulsacion_exti_sw1_pendiente = false;
		}
	}
}

/*
 * Valida por polling desde el main la pulsación pendiente de SW2 tras el antirrebote
 * Si sigue presionado y la cuenta está pausada, reinicia; luego limpia el registro pendiente
 */
static void ProcesarPulsacionExtiSW2(void) {
	if (pulsacion_exti_sw2_pendiente) {
		uint32_t tiempo_transcurrido = HAL_GetTick() - instante_interrupcion_sw2_ms;

		if (tiempo_transcurrido >= T_REBOTE_MS) {
			GPIO_PinState lectura = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin);

			if (lectura == GPIO_PIN_RESET && !CronometroEnMarcha()) {
				ReiniciarCronometro();
			}

			pulsacion_exti_sw2_pendiente = false;
		}
	}
}
