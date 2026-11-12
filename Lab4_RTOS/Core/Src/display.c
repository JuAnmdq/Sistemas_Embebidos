#include "display.h"
#include "main.h"

static volatile uint8_t display_buffer[4] = { 0, 0, 0, 0 };

static void ApagarSegmentosDisplay() {
	HAL_GPIO_WritePin(SEG_A_GPIO_Port, SEG_A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_B_GPIO_Port, SEG_B_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_C_GPIO_Port, SEG_C_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_D_GPIO_Port, SEG_D_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_E_GPIO_Port, SEG_E_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SEG_DP_GPIO_Port, SEG_DP_Pin, GPIO_PIN_RESET);
}

static void SeleccionarDigitoDisplay(uint8_t digito) {
	switch (digito) {
	case 0:
		HAL_GPIO_WritePin(DIGIT_SEL1_GPIO_Port, DIGIT_SEL1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT_SEL0_GPIO_Port, DIGIT_SEL0_Pin, GPIO_PIN_RESET);
		break;
	case 1:
		HAL_GPIO_WritePin(DIGIT_SEL1_GPIO_Port, DIGIT_SEL1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT_SEL0_GPIO_Port, DIGIT_SEL0_Pin, GPIO_PIN_SET);
		break;
	case 2:
		HAL_GPIO_WritePin(DIGIT_SEL1_GPIO_Port, DIGIT_SEL1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT_SEL0_GPIO_Port, DIGIT_SEL0_Pin, GPIO_PIN_RESET);
		break;
	case 3:
		HAL_GPIO_WritePin(DIGIT_SEL1_GPIO_Port, DIGIT_SEL1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DIGIT_SEL0_GPIO_Port, DIGIT_SEL0_Pin, GPIO_PIN_SET);
		break;
	default:
		HAL_GPIO_WritePin(DIGIT_SEL1_GPIO_Port, DIGIT_SEL1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DIGIT_SEL0_GPIO_Port, DIGIT_SEL0_Pin, GPIO_PIN_RESET);
		break;
	}
}

static void EscribirDigitoDisplay(uint8_t digito) {
	switch (digito) {
	case 0:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin,
				GPIO_PIN_SET);
		break;
	case 1:
		HAL_GPIO_WritePin(GPIOD, SEG_B_Pin | SEG_C_Pin, GPIO_PIN_SET);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_B_Pin | SEG_D_Pin | SEG_E_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 3:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 4:
		HAL_GPIO_WritePin(GPIOD,
				SEG_B_Pin | SEG_C_Pin | SEG_F_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 5:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_C_Pin | SEG_D_Pin | SEG_F_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 6:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 7:
		HAL_GPIO_WritePin(GPIOD, SEG_A_Pin | SEG_B_Pin | SEG_C_Pin, GPIO_PIN_SET);
		break;
	case 8:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin |
				SEG_E_Pin | SEG_F_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	case 9:
		HAL_GPIO_WritePin(GPIOD,
				SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_F_Pin | SEG_G_Pin,
				GPIO_PIN_SET);
		break;
	default:
		break;
	}
}

/*
 * Refresca un dígito por llamada
 * Recorre los cuatro dígitos de forma circular: 0 → 1 → 2 → 3 → 0…
 */
void RefrescarDisplay(void) {
	static uint8_t digito_actual = 0;
	uint8_t valor = display_buffer[digito_actual];

	ApagarSegmentosDisplay();
	SeleccionarDigitoDisplay(digito_actual);
	EscribirDigitoDisplay(valor);

	if (digito_actual == 2) {
		HAL_GPIO_WritePin(SEG_DP_GPIO_Port, SEG_DP_Pin, GPIO_PIN_SET);
	}

	digito_actual++;
	if (digito_actual == 4) {
		digito_actual = 0;
	}
}

void ActualizarTiempoDisplay(uint16_t segundos_restantes) {
	uint16_t minutos = segundos_restantes / 60;
	uint16_t segundos = segundos_restantes % 60;

	display_buffer[3] = minutos / 10;
	display_buffer[2] = minutos % 10;
	display_buffer[1] = segundos / 10;
	display_buffer[0] = segundos % 10;
}
