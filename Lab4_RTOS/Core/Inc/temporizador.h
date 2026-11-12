#ifndef TEMPORIZADOR_H
#define TEMPORIZADOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SIN_PEDIDO_AL_TEMPORIZADOR,
    PEDIDO_SET,
    PEDIDO_START,
    PEDIDO_PAUSE,
    PEDIDO_RESET,
    PEDIDO_STATUS
} TipoPedidoTemporizador;

typedef struct {
    uint16_t segundos_restantes;
    bool temporizador_activo;
    bool alarma_activa;
} EstadoTemporizador;

extern volatile bool aviso_alarma_pendiente;

void TareaTemporizador(void *argumento);
void SolicitarConfiguracionTemporizador(uint16_t segundos_solicitados);
void EnviarPedidoTemporizador(TipoPedidoTemporizador tipo);
EstadoTemporizador ConsultarEstadoTemporizador(void);

#endif
