#include "temporizador.h"
#include "display.h"
#include "pulsadores.h"
#include "main.h"
#include "cmsis_os.h"
#include "tareas_rtos.h"
#include "uart.h"

extern TIM_HandleTypeDef htim9;

static StatusTemporizador status;
volatile bool notificar_alarma_activa;

static void ActualizarDisplay(void)
{
    // Evita que el display (consumidor) lea mientras el temporizador actualiza los dígitos
    int32_t estado_scheduler = osKernelLock();
    ActualizarTiempoDisplay(status.segundos_restantes);
    osKernelRestoreLock(estado_scheduler);
}

// Carga los segundos, deja la cuenta detenida y apaga la alarma si estaba activa
static void ConfigurarTemporizador(uint16_t segundos)
{
    status.temporizador_activo = false;
    status.segundos_restantes = segundos;
    if (status.alarma_activa) {
        HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2);
        status.alarma_activa = false;
    }
    ActualizarDisplay();
}

// Si quedan segundos, activa la cuenta y toma el tick actual como referencia
static void IniciarTemporizador(uint32_t tick_actual, uint32_t *tick_inicio_cuenta)
{
    if (status.segundos_restantes > 0) {
        *tick_inicio_cuenta = tick_actual;
        status.temporizador_activo = true;
    }
}

static void ProcesarPedidoComando(uint32_t tick_actual,
                                  uint32_t *tick_inicio_cuenta,
                                  uint16_t segundos_iniciales)
{
    PedidoTemporizador pedido;

    if (ConsumirPedidoComando(&pedido)) {
        RespuestaTemporizador respuesta = { .ok = true, .pedido = pedido };
        switch (pedido.tipo) {
        case PEDIDO_SET:
            ConfigurarTemporizador(pedido.segundos);
            break;
        case PEDIDO_START:
            IniciarTemporizador(tick_actual, tick_inicio_cuenta);
            break;
        case PEDIDO_PAUSE:
            status.temporizador_activo = false;
            break;
        case PEDIDO_RESET:
            ConfigurarTemporizador(segundos_iniciales);
            break;
        case PEDIDO_STATUS:
            respuesta.status = status;
            break;
        default:
            respuesta.ok = false;
            break;
        }

        PublicarRespuestaUART(&respuesta);
    }
}

void TareaTemporizador(void *argumento)
{
    const uint16_t segundos_iniciales = 0;
    const uint32_t ticks_por_segundo = osKernelGetTickFreq();
    const uint32_t ticks_entre_consultas = 5;
    uint32_t tick_inicio_cuenta = osKernelGetTickCount();

    ConfigurarTemporizador(segundos_iniciales);

    while (1) {
        uint32_t tick_actual = osKernelGetTickCount();
        if (ProcesarPulsacionExtiSW1()) {
            if (status.temporizador_activo) {
                status.temporizador_activo = false;
            } else {
                IniciarTemporizador(tick_actual, &tick_inicio_cuenta);
            }
        }
        if (ProcesarPulsacionExtiSW2() && !status.temporizador_activo) {
            ConfigurarTemporizador(segundos_iniciales);
        }

        if (status.temporizador_activo &&
            tick_actual - tick_inicio_cuenta >= ticks_por_segundo) {
            // Mantiene sincronizada la referencia aunque la consulta llegue tarde
            tick_inicio_cuenta += ticks_por_segundo;
            status.segundos_restantes--;
            if (status.segundos_restantes == 0) {
                status.temporizador_activo = false;
                HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
                status.alarma_activa = true;
                notificar_alarma_activa = true;
                NotificarTrabajoUART();
            }
            ActualizarDisplay();
        }

        ProcesarPedidoComando(tick_actual, &tick_inicio_cuenta, segundos_iniciales);

        osDelay(ticks_entre_consultas);
    }
}
