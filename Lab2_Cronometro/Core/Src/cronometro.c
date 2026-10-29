#include "cronometro.h"
#include "display.h"

#define INTERRUPCIONES_TIMER_POR_CENTESIMA 5
#define CENTESIMAS_POR_MINUTO 6000

static volatile uint16_t centesimas_totales = 0;
static volatile bool cronometro_activo = false;
static volatile uint8_t contador_interrupciones_timer = 0;

/*
 * Reinicia el tiempo y el contador de interrupciones y deja el display en cero
 */
void ReiniciarCronometro(void) {
	centesimas_totales = 0;
	contador_interrupciones_timer = 0;
	ActualizarTiempoDisplay(centesimas_totales);
}

void TogglearCronometro(void) {
	cronometro_activo = !cronometro_activo;
}

bool CronometroEnMarcha(void) {
	return cronometro_activo;
}

/*
 * Actualiza desde el main el tiempo mostrado en el display
 */
void ActualizarCuentaCronometro(void) {
	ActualizarTiempoDisplay(centesimas_totales);
}

/*
 * Cuenta las interrupciones de TIM2 mientras el cronómetro está activo
 * Al completar una centésima, la suma a la cuenta; al llegar al minuto, vuelve a cero
 */
void ContarInterrupcionTimer(void) {
	if (cronometro_activo) {
		contador_interrupciones_timer++;

		if (contador_interrupciones_timer >= INTERRUPCIONES_TIMER_POR_CENTESIMA) {
			contador_interrupciones_timer = 0;
			centesimas_totales = (centesimas_totales + 1) % CENTESIMAS_POR_MINUTO;
		}
	} else {
		contador_interrupciones_timer = 0;
	}
}
