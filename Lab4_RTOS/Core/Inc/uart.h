#ifndef UART_H
#define UART_H

void IniciarRecepcionUART(void);
void ProcesarComandosBuffer(void);
void AlmacenarCaracterRecibidoEnElBuffer(void);
void TransmitirUART(char *texto);

#endif
