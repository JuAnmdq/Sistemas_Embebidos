#include "tareas_rtos.h"
#include "main.h"
#include "cmsis_os.h"

typedef struct {
	uint16_t pin;
	uint32_t ticks_entre_cambios;
} params_tarea_LED;

static params_tarea_LED params_LED_rojo = {
    .pin = GPIO_PIN_14,
    .ticks_entre_cambios = 250
};

static params_tarea_LED params_LED_amarillo = {
    .pin = GPIO_PIN_13,
    .ticks_entre_cambios = 500
};

static params_tarea_LED params_LED_verde = {
    .pin = GPIO_PIN_12,
    .ticks_entre_cambios = 125
};

static const osThreadAttr_t atributos_LED_rojo = {
    .name = "LED_Rojo",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t atributos_LED_amarillo = {
    .name = "LED_Amarillo",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t atributos_LED_verde = {
    .name = "LED_Verde",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static osThreadId_t defaultTaskHandle;

void StartDefaultTask(void *argument);
void TareaRojaEsperaActiva(void *argument);
void TareaRojaRetardoRelativo(void *argument);
void TareaRojaInstantesAbsolutos(void *argument);
static void Procesar();

void CrearTareas()
{
    // Hito 1: Crear 3 tareas reutilizando, 3 instancias de la misma función, parametrizando
	defaultTaskHandle = osThreadNew(StartDefaultTask, &params_LED_rojo, &atributos_LED_rojo);
    osThreadNew(StartDefaultTask, &params_LED_amarillo, &atributos_LED_amarillo);
    osThreadNew(StartDefaultTask, &params_LED_verde, &atributos_LED_verde);

    // Hito 2: elegir una sola de estas tres en lugar del rojo del Hito 1
    // defaultTaskHandle = osThreadNew(TareaRojaEsperaActiva, &params_LED_rojo, &atributos_LED_rojo);
    // defaultTaskHandle = osThreadNew(TareaRojaRetardoRelativo, &params_LED_rojo, &atributos_LED_rojo);
    // defaultTaskHandle = osThreadNew(TareaRojaInstantesAbsolutos, &params_LED_rojo, &atributos_LED_rojo);

     osThreadSetPriority(defaultTaskHandle, osPriorityHigh);

}

void StartDefaultTask(void *argument)
{
  while(1) {
	  HAL_GPIO_TogglePin(GPIOD, ((params_tarea_LED*)argument)->pin);
	  osDelay(((params_tarea_LED*)argument)->ticks_entre_cambios);
  }
}

// Hito 2 (4.1) Con prioridad alta, sigue Ready/Running y acapara la CPU (starvation)
void TareaRojaEsperaActiva(void *argument) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
        HAL_Delay(1000);
    }
}

// Hito 2 (4.2): el retardo relativo suma C a T en cada vuelta: C + T entre cambios
void TareaRojaRetardoRelativo(void *argument)
{
    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
        Procesar(500); // 5ms de procesamiento
        osDelay(1000); // Pasa a Blocked, independientemente de la prioridad, permitiendo que las demás tareas ejecuten
    }
}

// Hito 2 (4.2): la activación periódica incluye C dentro de T: cambios cada 1 s, si alcanza el tiempo y recibe CPU
void TareaRojaInstantesAbsolutos(void *argument)
{
    uint32_t tref = osKernelGetTickCount();

    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
        Procesar(500);
        // Calcula el próximo instante desde la referencia anterior
        tref += 1000;
        osDelayUntil(tref);
    }
}

// Simula tiempo computacional
static void Procesar(uint32_t C) {
	uint32_t inicio = osKernelGetTickCount();

    while (osKernelGetTickCount() - inicio < C) {
    	// Consume ciclos de CPU
    }
}
