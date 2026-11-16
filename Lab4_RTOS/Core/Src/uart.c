#include "uart.h"
#include "main.h"
#include "cmsis_os.h"
#include "tareas_rtos.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SEGUNDOS_MAXIMOS             5999
#define COMANDO_UART_SIZE            32
#define BUFFER_UART_SIZE             64
#define UART_TIMEOUT_MS              20
#define CAPACIDAD_COLA_COMANDOS      10

extern UART_HandleTypeDef huart3;

static uint8_t caracter_uart_rx = 0;
static volatile uint8_t buffer_uart[BUFFER_UART_SIZE];
static volatile uint8_t pos_lectura_uart = 0;
static volatile uint8_t pos_escritura_uart = 0;
static char comando_uart[COMANDO_UART_SIZE] = "";
static uint8_t pos_comando_uart = 0;
static bool descartar_comando = false;

static osMessageQueueId_t cola_comandos;
static RespuestaTemporizador respuesta_compartida;
static osSemaphoreId_t uart_respuesta_recibida;

static void ProcesarCaracterUART(uint8_t caracter);
static void ProducirComandos(const char *comando);
static bool ObtenerSegundosSet(const char *argumento, uint16_t *segundos);
static uint16_t ConvertirDigitoASCII(char caracter);
static void TransmitirRespuestaUART(const RespuestaTemporizador *respuesta);

void InicializarComunicacionTemporizador(void)
{
    cola_comandos = osMessageQueueNew(CAPACIDAD_COLA_COMANDOS, sizeof(PedidoTemporizador), NULL);
    uart_respuesta_recibida = osSemaphoreNew(1, 1, NULL);
}

static bool ProducirPedidoComando(PedidoTemporizador pedido)
{
    return uart_respuesta_recibida != NULL &&
           osMessageQueuePut(cola_comandos, &pedido, 0, 0) == osOK;
}

bool ConsumirPedidoComando(PedidoTemporizador *pedido)
{
    return osSemaphoreGetCount(uart_respuesta_recibida) == 1 &&
           osMessageQueueGet(cola_comandos, pedido, NULL, 0) == osOK;
}

void PublicarRespuestaUART(const RespuestaTemporizador *respuesta)
{
    respuesta_compartida = *respuesta;
    // Al publicar una nueva respuesta, pone la respuesta como no vista por la UART
    osSemaphoreAcquire(uart_respuesta_recibida, 0);
    NotificarTrabajoUART();
}

void IniciarRecepcionUART(void) {
	HAL_UART_Receive_IT(&huart3, &caracter_uart_rx, 1);
}

/*
 * Cuando TareaUART recibe un aviso, procesa los caracteres pendientes en el buffer:
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
			ProducirComandos(comando_uart);
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

static void ProducirComandos(const char *comando) {
	PedidoTemporizador pedido = { 0 };
	bool es_comando_valido = true;

	if (strncmp(comando, "SET ", 4) == 0) {
		uint16_t segundos = 0;
		es_comando_valido = ObtenerSegundosSet(&comando[4], &segundos);
		if (es_comando_valido) {
			pedido = (PedidoTemporizador){ .tipo = PEDIDO_SET, .segundos = segundos };
		}
	} else if (strcmp(comando, "START") == 0) {
		pedido.tipo = PEDIDO_START;
	} else if (strcmp(comando, "PAUSE") == 0) {
		pedido.tipo = PEDIDO_PAUSE;
	} else if (strcmp(comando, "RESET") == 0) {
		pedido.tipo = PEDIDO_RESET;
	} else if (strcmp(comando, "STATUS") == 0) {
		pedido.tipo = PEDIDO_STATUS;
	} else {
		es_comando_valido = false;
	}

	if (!es_comando_valido || !ProducirPedidoComando(pedido)) {
		TransmitirUART("\r\nERROR\r\n");
	}
}

// Convierte y valida los segundos recibidos después de SET
static bool ObtenerSegundosSet(const char *argumento, uint16_t *segundos) {
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
		*segundos = segundos_solicitados;
	}
	return es_comando_valido;
}

void ConsumirRespuestaDelTemporizador(void)
{
    if (uart_respuesta_recibida != NULL &&
        osSemaphoreGetCount(uart_respuesta_recibida) == 0) {
        const RespuestaTemporizador respuesta = respuesta_compartida;
        osSemaphoreRelease(uart_respuesta_recibida); // UART ya tiene su copia.
        TransmitirRespuestaUART(&respuesta);
    }
}

static void TransmitirRespuestaUART(const RespuestaTemporizador *respuesta)
{
    if (!respuesta->ok) {
        TransmitirUART("\r\nERROR\r\n");
        return;
    }

    char mensaje[32];

    const PedidoTemporizador pedido = respuesta->pedido;

    switch (pedido.tipo) {
    case PEDIDO_SET:
        snprintf(mensaje, sizeof(mensaje), "\r\nOK SET %" PRIu16 "\r\n",
                 pedido.segundos);
        break;
    case PEDIDO_START:
        TransmitirUART("\r\nOK START\r\n");
        return;
    case PEDIDO_PAUSE:
        TransmitirUART("\r\nOK PAUSE\r\n");
        return;
    case PEDIDO_RESET:
        TransmitirUART("\r\nOK RESET\r\n");
        return;
    case PEDIDO_STATUS: {
        const StatusTemporizador status_actual = respuesta->status;
        const char *nombre_estado = "PAUSED";
        if (status_actual.alarma_activa) {
            nombre_estado = "ALARM";
        } else if (status_actual.temporizador_activo) {
            nombre_estado = "RUNNING";
        }

        unsigned minutos = status_actual.segundos_restantes / 60;
        unsigned segundos = status_actual.segundos_restantes % 60;
        snprintf(mensaje, sizeof(mensaje), "\r\n%s %02u.%02u\r\n",
                 nombre_estado, minutos, segundos);
        break;
    }
    default:
        snprintf(mensaje, sizeof(mensaje), "\r\nERROR\r\n");
        break;
    }

    TransmitirUART(mensaje);
}

static uint16_t ConvertirDigitoASCII(char caracter) {
	return caracter - '0';
}
