#include "uart.h"
#include "main.h"
#include "temporizador.h"

#include <stdbool.h>
#include <string.h>

#define SEGUNDOS_MAXIMOS             5999
#define COMANDO_UART_SIZE            32
#define BUFFER_UART_SIZE             64
#define UART_TIMEOUT_MS              20

extern UART_HandleTypeDef huart3;

static uint8_t caracter_uart_rx = 0;
static volatile uint8_t buffer_uart[BUFFER_UART_SIZE];
static volatile uint8_t pos_lectura_uart = 0;
static volatile uint8_t pos_escritura_uart = 0;
static char comando_uart[COMANDO_UART_SIZE] = "";
static uint8_t pos_comando_uart = 0;
static bool descartar_comando = false;

static void ProcesarCaracterUART(uint8_t caracter);
static void ProcesarComandoUART(const char *comando);
static void ProcesarComandoSet(const char *argumento);
static uint16_t ConvertirDigitoASCII(char caracter);
static void EnviarEstadoTemporizador(void);

void IniciarRecepcionUART(void) {
	HAL_UART_Receive_IT(&huart3, &caracter_uart_rx, 1);
}

/*
 * Se consulta por polling; mientras haya caracteres pendientes en el buffer:
 * - Lee el carácter
 * - Actualiza la posición de lectura del buffer circular
 * - Procesa el carácter
 */
void ProcesarComandosBuffer(void) {
	while (pos_lectura_uart != pos_escritura_uart) {
		uint8_t caracter = buffer_uart[pos_lectura_uart];
		pos_lectura_uart++;

		if (pos_lectura_uart == BUFFER_UART_SIZE) {
			pos_lectura_uart = 0;
		}

		ProcesarCaracterUART(caracter);
	}
}

/*
 * Al completarse la recepción de un carácter:
 * - Lo guarda en el buffer si hay espacio
 * - Actualiza la posición de escritura del buffer circular
 * - Deja USART3 escuchando de forma asíncrona el próximo carácter
 *
 * TareaUART lee y procesa de a un carácter a la vez
 */
void AlmacenarCaracterRecibidoEnElBuffer(void) {
	uint8_t sig_pos = pos_escritura_uart + 1;

	if (sig_pos == BUFFER_UART_SIZE) {
		sig_pos = 0;
	}

	if (sig_pos != pos_lectura_uart) {
		buffer_uart[pos_escritura_uart] = caracter_uart_rx;
		pos_escritura_uart = sig_pos;
	}

	IniciarRecepcionUART();
}

void TransmitirUART(char* texto) {
	uint16_t n_caracteres = (uint16_t) strlen(texto);
	HAL_UART_Transmit(&huart3, (uint8_t *) texto, n_caracteres, UART_TIMEOUT_MS);
}

/*
 * Por cada carácter leído del buffer circular:
 * - Si es Enter y el comando no está vacío, lo procesa y reinicia su posición para evitar procesar dos veces el mismo Enter (\r\n)
 * - Si no es Enter, lo muestra en la terminal como eco y lo agrega al comando
 * - Si no hay espacio para agregar el carácter, descarta el comando e informa un error
 */
static void ProcesarCaracterUART(uint8_t caracter) {
	bool enter_presionado = caracter == '\r' || caracter == '\n';
	if (enter_presionado) {
		if (!descartar_comando && pos_comando_uart > 0) {
			comando_uart[pos_comando_uart] = '\0';
			ProcesarComandoUART(comando_uart);
		}
		pos_comando_uart = 0;
		descartar_comando = false;
	} else if (!descartar_comando) {
		HAL_UART_Transmit(&huart3, &caracter, 1, UART_TIMEOUT_MS);

		if (pos_comando_uart < COMANDO_UART_SIZE - 1) {
			comando_uart[pos_comando_uart++] = caracter;
		} else {
			descartar_comando = true;
			TransmitirUART("\r\nERROR\r\n");
		}
	}
}

static void ProcesarComandoUART(const char *comando) {
	if (strncmp(comando, "SET ", 4) == 0) {
		ProcesarComandoSet(&comando[4]);
	} else if (strcmp(comando, "START") == 0) {
		EnviarPedidoTemporizador(PEDIDO_START);
		TransmitirUART("\r\nOK\r\n");
	} else if (strcmp(comando, "PAUSE") == 0) {
		EnviarPedidoTemporizador(PEDIDO_PAUSE);
		TransmitirUART("\r\nOK\r\n");
	} else if (strcmp(comando, "RESET") == 0) {
		EnviarPedidoTemporizador(PEDIDO_RESET);
		TransmitirUART("\r\nOK\r\n");
	} else if (strcmp(comando, "STATUS") == 0) {
		EnviarEstadoTemporizador();
	} else {
		TransmitirUART("\r\nERROR\r\n");
	}
}

/*
 * Convierte y valida los segundos recibidos después de SET
 * Si son válidos, configura el temporizador y responde OK; de lo contrario, responde ERROR
 */
static void ProcesarComandoSet(const char *argumento) {
	bool es_comando_valido = argumento[0] != '\0';
	uint16_t segundos_solicitados = 0;
	uint16_t posicion = 0;
	uint16_t n = (uint16_t) strlen(argumento);

	while (es_comando_valido && posicion < n) {
		if (argumento[posicion] >= '0' && argumento[posicion] <= '9') {
			uint16_t digito = ConvertirDigitoASCII(argumento[posicion]);
			segundos_solicitados = segundos_solicitados * 10 + digito;

			if (segundos_solicitados > SEGUNDOS_MAXIMOS) {
				es_comando_valido = false;
			}
		} else {
			es_comando_valido = false;
		}

		posicion++;
	}

	if (es_comando_valido) {
		SolicitarConfiguracionTemporizador(segundos_solicitados);
		TransmitirUART("\r\nOK\r\n");
	} else {
		TransmitirUART("\r\nERROR\r\n");
	}
}

static void EnviarEstadoTemporizador(void) {
    EstadoTemporizador estado = ConsultarEstadoTemporizador();
    uint16_t minutos = estado.segundos_restantes / 60;
    uint16_t segundos = estado.segundos_restantes % 60;
    char tiempo[] = {
        '0' + minutos / 10, '0' + minutos % 10, '.',
        '0' + segundos / 10, '0' + segundos % 10, '\r', '\n', '\0'
    };

    if (estado.alarma_activa) {
        TransmitirUART("\r\nALARM ");
    } else if (estado.temporizador_activo) {
        TransmitirUART("\r\nRUNNING ");
    } else {
        TransmitirUART("\r\nPAUSED ");
    }
    TransmitirUART(tiempo);
}

static uint16_t ConvertirDigitoASCII(char caracter) {
	return caracter - '0';
}
