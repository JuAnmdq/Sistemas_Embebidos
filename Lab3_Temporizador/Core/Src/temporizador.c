#include "temporizador.h"
#include "main.h"
#include "display.h"
#include "uart.h"

#define INTERRUPCIONES_TIMER_POR_SEGUNDO 500

extern TIM_HandleTypeDef htim9;

static volatile uint16_t segundos_restantes = 0;
static volatile bool temporizador_activo = false;
static bool alarma_activa = false;
static volatile uint16_t contador_interrupciones_timer = 0;

static void ActivarAlarma(void);
static void DesactivarAlarma(void);
static char ConvertirDigitoAASCII(uint16_t digito);

/*
 * Carga el tiempo solicitado y lo muestra con la cuenta pausada
 * Desactiva la alarma y reinicia el contador de interrupciones
 */
void ConfigurarTemporizador(uint16_t segundos_solicitados) {
	temporizador_activo = false;
	segundos_restantes = segundos_solicitados;
	DesactivarAlarma();
	contador_interrupciones_timer = 0;
	ActualizarTiempoDisplay(segundos_restantes);
}

/*
 * Si quedan segundos:
 * - Reinicia el contador de interrupciones con la cuenta detenida
 * - Pone en marcha la cuenta regresiva
 */
void IniciarTemporizador(void) {
	if (segundos_restantes > 0) {
		temporizador_activo = false;
		contador_interrupciones_timer = 0;
		temporizador_activo = true;
	}
}

/*
 * Pausa la cuenta conservando los segundos restantes
 * Reinicia el contador de interrupciones del timer
 */
void PausarTemporizador(void) {
	temporizador_activo = false;
	contador_interrupciones_timer = 0;
}

/*
 * Detiene la cuenta, apaga la alarma y deja el display en cero
 * Reinicia el contador de interrupciones del timer
 */
void ReiniciarTemporizador(void) {
	temporizador_activo = false;
	DesactivarAlarma();
	segundos_restantes = 0;
	contador_interrupciones_timer = 0;
	ActualizarTiempoDisplay(segundos_restantes);
}

void TogglearTemporizador(void) {
	temporizador_activo = !temporizador_activo;
}

bool TemporizadorEnMarcha(void) {
	return temporizador_activo;
}

/*
 * Consulta por polling el tiempo restante y actualiza el display
 * Al llegar a cero, detiene la cuenta, activa la alarma y notifica por UART
 */
void ActualizarCuentaTemporizador(void) {
	if (temporizador_activo && segundos_restantes == 0) {
		temporizador_activo = false;

		if (!alarma_activa) {
			ActivarAlarma();
			TransmitirUART("ALARM\r\n");
		}
	}

	ActualizarTiempoDisplay(segundos_restantes);
}

/*
 * Cuenta las interrupciones de TIM2 mientras el temporizador está activo
 * Al completar un segundo, lo descuenta directamente de la cuenta
 */
void ContarInterrupcionTimer(void) {
	if (temporizador_activo) {
		contador_interrupciones_timer++;

		if (contador_interrupciones_timer >= INTERRUPCIONES_TIMER_POR_SEGUNDO) {
			contador_interrupciones_timer = 0;
			if (segundos_restantes > 0) {
				segundos_restantes--;
			}
		}
	} else {
		contador_interrupciones_timer = 0;
	}
}

/*
 * Envía por UART el estado del temporizador y el tiempo restante
 * Expresa el tiempo en minutos y segundos con formato MM.SS
 */
void EnviarEstadoTemporizador(void) {
	char tiempo[8];
	uint16_t tiempo_restante = segundos_restantes;
	uint16_t minutos = tiempo_restante / 60;
	uint16_t segundos = tiempo_restante % 60;

	if (alarma_activa) {
		TransmitirUART("\r\nALARM ");
	} else if (temporizador_activo) {
		TransmitirUART("\r\nRUNNING ");
	} else {
		TransmitirUART("\r\nPAUSED ");
	}

	tiempo[0] = ConvertirDigitoAASCII(minutos / 10);
	tiempo[1] = ConvertirDigitoAASCII(minutos % 10);
	tiempo[2] = '.';
	tiempo[3] = ConvertirDigitoAASCII(segundos / 10);
	tiempo[4] = ConvertirDigitoAASCII(segundos % 10);
	tiempo[5] = '\r';
	tiempo[6] = '\n';
	tiempo[7] = '\0';

	TransmitirUART(tiempo);
}

static void ActivarAlarma(void) {
	alarma_activa = true;
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
}

static void DesactivarAlarma(void) {
	alarma_activa = false;
	HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2);
}

static char ConvertirDigitoAASCII(uint16_t digito) {
	return '0' + digito;
}
