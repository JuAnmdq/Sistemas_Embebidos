#ifndef TEMPORIZADOR_H
#define TEMPORIZADOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PEDIDO_SET,
    PEDIDO_START,
    PEDIDO_PAUSE,
    PEDIDO_RESET,
    PEDIDO_STATUS
} TipoPedidoTemporizador;

typedef struct {
    TipoPedidoTemporizador tipo;
    uint16_t segundos;
} PedidoTemporizador;

typedef struct {
    uint16_t segundos_restantes;
    bool temporizador_activo;
    bool alarma_activa;
} StatusTemporizador;

typedef struct {
    bool ok;
    PedidoTemporizador pedido;
    StatusTemporizador status;
} RespuestaTemporizador;

extern volatile bool notificar_alarma_activa;

void TareaTemporizador(void *argumento);

#endif
