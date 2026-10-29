#include "pulsadores.h"
#include "main.h"

#include <stdbool.h>

#define T_REBOTE_MS 20U

static GPIO_PinState ultimo_estado_valido_rojo = GPIO_PIN_SET;
static GPIO_PinState ultimo_estado_valido_amarillo = GPIO_PIN_SET;
static GPIO_PinState ultimo_estado_valido_verde = GPIO_PIN_SET;

static bool HayNuevaPulsacion(GPIO_TypeDef *puerto, uint16_t pin,
		GPIO_PinState *ultimo_estado_valido);

/*
 * Inicializa los estados de referencia con los pulsadores liberados (activo bajo)
 */
void InicializarPulsadores(void) {
	ultimo_estado_valido_rojo = GPIO_PIN_SET;
	ultimo_estado_valido_amarillo = GPIO_PIN_SET;
	ultimo_estado_valido_verde = GPIO_PIN_SET;
}

/*
 * Consulta por polling los pulsadores con antirrebote y togglea el led correspondiente
 * Procesa una nueva pulsación por iteración, en orden rojo → amarillo → verde
 */
void ProcesarPulsadores(void) {
	if (HayNuevaPulsacion(PULSADOR_GPIO_Port, PULSADOR_Pin,
			&ultimo_estado_valido_rojo)) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	} else if (HayNuevaPulsacion(PULSADOR_AMARILLO_GPIO_Port, PULSADOR_AMARILLO_Pin,
			&ultimo_estado_valido_amarillo)) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_AMARILLO_Pin);
	} else if (HayNuevaPulsacion(PULSADOR_VERDE_GPIO_Port, PULSADOR_VERDE_Pin,
			&ultimo_estado_valido_verde)) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_VERDE_Pin);
	}
}

/*
 * Valida por polling los cambios del pulsador con una espera de antirrebote
 * Actualiza el estado válido y devuelve true solo al pasar de liberado a presionado
 */
static bool HayNuevaPulsacion(GPIO_TypeDef *puerto, uint16_t pin,
		GPIO_PinState *ultimo_estado_valido) {
	bool nueva_pulsacion = false;
	GPIO_PinState lectura1 = HAL_GPIO_ReadPin(puerto, pin);

	if (lectura1 != *ultimo_estado_valido) {
		HAL_Delay(T_REBOTE_MS);
		GPIO_PinState lectura2 = HAL_GPIO_ReadPin(puerto, pin);

		if (lectura2 == lectura1) {
			GPIO_PinState estado_anterior = *ultimo_estado_valido;
			*ultimo_estado_valido = lectura2;

			nueva_pulsacion = (estado_anterior == GPIO_PIN_SET
					&& lectura2 == GPIO_PIN_RESET);
		}
	}

	return nueva_pulsacion;
}
