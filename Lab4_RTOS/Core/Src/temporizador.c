#include "temporizador.h"
#include "display.h"
#include "pulsadores.h"
#include "main.h"
#include "cmsis_os.h"

extern TIM_HandleTypeDef htim9;

static EstadoTemporizador estado;
volatile bool notificar_alarma_activa;

// TareaUART publica un solo pedido y espera que el temporizador lo atienda
static volatile struct {
    TipoPedidoTemporizador tipo;
    uint16_t segundos;
    EstadoTemporizador respuesta;
} pedido_temporizador = {
    .tipo = SIN_PEDIDO_AL_TEMPORIZADOR,
};

static void ActualizarDisplay(void)
{
    // Evita que el display (consumidor) lea mientras el temporizador actualiza los dígitos
    int32_t estado_scheduler = osKernelLock();
    ActualizarTiempoDisplay(estado.segundos_restantes);
    osKernelRestoreLock(estado_scheduler);
}

// Carga los segundos, deja la cuenta detenida y apaga la alarma si estaba activa
static void ConfigurarTemporizador(uint16_t segundos)
{
    estado.temporizador_activo = false;
    estado.segundos_restantes = segundos;
    if (estado.alarma_activa) {
        HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2);
        estado.alarma_activa = false;
    }
    ActualizarDisplay();
}

// Si quedan segundos, activa la cuenta y toma el tick actual como referencia
static void IniciarTemporizador(uint32_t tick_actual, uint32_t *tick_inicio_cuenta)
{
    if (estado.segundos_restantes > 0) {
        *tick_inicio_cuenta = tick_actual;
        estado.temporizador_activo = true;
    }
}

// Publica el pedido y cede la CPU mientras espera que Temporizador lo atienda
void EnviarPedidoTemporizador(TipoPedidoTemporizador tipo)
{
    pedido_temporizador.tipo = tipo;
    while (pedido_temporizador.tipo != SIN_PEDIDO_AL_TEMPORIZADOR) {
        osDelay(1);
    }
}

// Guarda los segundos que tiene que cargar Temporizador antes de publicar el SET
void SolicitarConfiguracionTemporizador(uint16_t segundos_solicitados)
{
    pedido_temporizador.segundos = segundos_solicitados;
    EnviarPedidoTemporizador(PEDIDO_SET);
}

// Solicita y devuelve el estado actual del temporizador
EstadoTemporizador ConsultarEstadoTemporizador(void)
{
    EnviarPedidoTemporizador(PEDIDO_STATUS);
    return pedido_temporizador.respuesta;
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
            if (estado.temporizador_activo) {
                estado.temporizador_activo = false;
            } else {
                IniciarTemporizador(tick_actual, &tick_inicio_cuenta);
            }
        }
        if (ProcesarPulsacionExtiSW2() && !estado.temporizador_activo) {
            ConfigurarTemporizador(segundos_iniciales);
        }

        if (estado.temporizador_activo &&
            tick_actual - tick_inicio_cuenta >= ticks_por_segundo) {
            // Mantiene sincronizada la referencia aunque la consulta llegue tarde
            tick_inicio_cuenta += ticks_por_segundo;
            estado.segundos_restantes--;
            if (estado.segundos_restantes == 0) {
                estado.temporizador_activo = false;
                HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
                estado.alarma_activa = true;
                notificar_alarma_activa = true;
            }
            ActualizarDisplay();
        }

        // Ejecuta el pedido y confirma a UART que ya fue atendido
        TipoPedidoTemporizador tipo = pedido_temporizador.tipo;
        switch (tipo) {
        case PEDIDO_SET:
            ConfigurarTemporizador(pedido_temporizador.segundos);
            break;
        case PEDIDO_START:
            IniciarTemporizador(tick_actual, &tick_inicio_cuenta);
            break;
        case PEDIDO_PAUSE:
            estado.temporizador_activo = false;
            break;
        case PEDIDO_RESET:
            ConfigurarTemporizador(segundos_iniciales);
            break;
        case PEDIDO_STATUS:
            pedido_temporizador.respuesta = estado;
            break;
        case SIN_PEDIDO_AL_TEMPORIZADOR:
            break;
        }
        pedido_temporizador.tipo = SIN_PEDIDO_AL_TEMPORIZADOR;

        osDelay(ticks_entre_consultas);
    }
}
