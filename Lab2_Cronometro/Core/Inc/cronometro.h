#ifndef CRONOMETRO_H
#define CRONOMETRO_H

#include <stdbool.h>
#include <stdint.h>

void ReiniciarCronometro(void);
void TogglearCronometro(void);
bool CronometroEnMarcha(void);
void ActualizarCuentaCronometro(void);
void ContarInterrupcionTimer(void);

#endif
