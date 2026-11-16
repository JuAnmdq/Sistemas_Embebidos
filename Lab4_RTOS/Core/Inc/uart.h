#ifndef UART_H
#define UART_H

#include "temporizador.h"

void InicializarComunicacionTemporizador(void);
bool ConsumirPedidoComando(PedidoTemporizador *pedido);
void PublicarRespuestaUART(const RespuestaTemporizador *respuesta);
void IniciarRecepcionUART(void);
void ProcesarComandosBuffer(void);
void ConsumirRespuestaDelTemporizador(void);
void AlmacenarCaracterRecibidoEnElBuffer(void);
void TransmitirUART(char *texto);

#endif
