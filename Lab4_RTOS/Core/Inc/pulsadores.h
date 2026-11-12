#ifndef PULSADORES_H
#define PULSADORES_H

#include <stdbool.h>
#include <stdint.h>

void RegistrarInterrupcionPulsador(uint16_t pin);
bool ProcesarPulsacionExtiSW1(void);
bool ProcesarPulsacionExtiSW2(void);

#endif
