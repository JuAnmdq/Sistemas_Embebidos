#ifndef TEMPORIZADOR_H
#define TEMPORIZADOR_H

#include <stdbool.h>
#include <stdint.h>

void ConfigurarTemporizador(uint16_t segundos_solicitados);
void IniciarTemporizador(void);
void PausarTemporizador(void);
void ReiniciarTemporizador(void);
void TogglearTemporizador(void);
bool TemporizadorEnMarcha(void);
void ActualizarCuentaTemporizador(void);
void ContarInterrupcionTimer(void);
void EnviarEstadoTemporizador(void);

#endif
