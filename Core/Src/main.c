/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1306.h"

// https://github.com/nayuki/QR-Code-generator
// https://www.nayuki.io/page/qr-code-generator-library
#include "qrcodegen.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIN_SEND_PERIOD 80
#define TX_BUFFER_SIZE 11
#define RX_BUFFER_SIZE 10

#define INT_MODE_ON 1
#define INT_MODE_OFF 0
#define INT_SPD1 1
#define INT_SPD2 5
#define INT_SPD3 9
#define INT_SPD4 13

#define SPD1_MODE_OFF 0
#define SPD1_MODE_ON 1
#define SPD2_MODE_OFF 0
#define SPD2_MODE_ON 1

#define SST_MODE_OFF 0
#define SST_MODE_ON 1
#define SST_MANUAL_INTERVAL 8000

#define NUM_OF_MODES 9

#define USE_OLED

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const char *wiper_mode_name[] =
{ "INT1", "INT2", "INT3", "INT4", "SPD1", "SPD2", "SST", "STOP", "OFF" };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// https://controllerstech.com/stm32-uart-8-lin-protocol-part-1/
// https://controllerstech.com/stm32-uart-9-lin-protocol-part-2/
// https://controllerstech.com/stm32-uart-10-lin-protocol-part-3/
uint8_t Pid_Calc(uint8_t ID);
uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size);

static void showPartDatasheet(void);
static void printQr(const uint8_t qrcode[]);
static void displayQr(const uint8_t qrcode[]);

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&hlpuart1, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t txData[TX_BUFFER_SIZE];
uint8_t rxData[RX_BUFFER_SIZE];

uint32_t LinSendSoftTimer;

uint8_t golf7counter = 0;

uint8_t wiper_mode = 0;

uint32_t rng_number;

volatile uint8_t lin_data_received_flag = 0;

uint8_t single_stroke_flag = 0;
uint32_t SingleStrokeSoftTimer;

extern DMA_HandleTypeDef hdma_usart3_rx;

uint8_t lcd_line[64];
uint32_t uart_message_length;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */

#ifdef USE_OLED
	ssd1306_Init();

	showPartDatasheet();
	HAL_Delay(3000);

	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 2);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(18, 12);
	ssd1306_WriteString("LIN 2.0 bus demo", Font_6x8, White);
	ssd1306_SetCursor(13, 22);
	ssd1306_WriteString("VW Golf VII wiper", Font_6x8, White);
	ssd1306_UpdateScreen();
#endif

	LinSendSoftTimer = HAL_GetTick();
	SingleStrokeSoftTimer = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{

		// manual trigger simulation
		if (HAL_GetTick() - SingleStrokeSoftTimer > SST_MANUAL_INTERVAL)
		{
			SingleStrokeSoftTimer = HAL_GetTick();
			single_stroke_flag = 0;
		}

		if (lin_data_received_flag == 1)
		{
			lin_data_received_flag = 0;
//			HAL_UART_Transmit(&hlpuart1, rxData, RX_BUFFER_SIZE, 10);
//			HAL_UART_Transmit(&hlpuart1, (uint8_t *)"\r\n", sizeof("\r\n"), 10);
			uart_message_length = sprintf((char*) lcd_line,
					"Rx %02X %02X %02X %02X %02X %02X", rxData[3], rxData[4],
					rxData[5], rxData[6], rxData[7], rxData[8]);
			HAL_UART_Transmit(&hlpuart1, lcd_line, uart_message_length, 10);
			HAL_UART_Transmit(&hlpuart1, (uint8_t*) "\r\n", sizeof("\r\n"), 10);
#ifdef USE_OLED
			ssd1306_SetCursor(2, 55);
			ssd1306_WriteString((char*) lcd_line, Font_6x8, White);
			ssd1306_UpdateScreen();
#endif
		}

		if (HAL_GetTick() - LinSendSoftTimer > LIN_SEND_PERIOD)
		{
			LinSendSoftTimer = HAL_GetTick();

			if (golf7counter == 16)
			{
				golf7counter = 0;
			}
			// send status
			txData[0] = 0x55;  // sync field

			txData[1] = Pid_Calc(0x31);

			txData[2] = (0x30 | (golf7counter & 0x0F)); // byte 0

			switch (wiper_mode)
			{
			case 0: // INT1
				txData[3] =
						((SST_MODE_OFF << 4) | (INT_MODE_ON << 5)
								| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7)
								| INT_SPD1);
				break;
			case 1: // INT2
				txData[3] =
						((SST_MODE_OFF << 4) | (INT_MODE_ON << 5)
								| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7)
								| INT_SPD2);
				break;
			case 2: // INT3
				txData[3] =
						((SST_MODE_OFF << 4) | (INT_MODE_ON << 5)
								| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7)
								| INT_SPD3);
				break;
			case 3: // INT4
				txData[3] =
						((SST_MODE_OFF << 4) | (INT_MODE_ON << 5)
								| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7)
								| INT_SPD4);
				break;
			case 4: // SPD1 (slower)
				txData[3] = ((SST_MODE_OFF << 4) | (INT_MODE_OFF << 5)
						| (SPD1_MODE_ON << 6) | (SPD2_MODE_OFF << 7) | 0);
				break;
			case 5: // SPD2 (faster)
				txData[3] = ((SST_MODE_OFF << 4) | (INT_MODE_OFF << 5)
						| (SPD1_MODE_OFF << 6) | (SPD2_MODE_ON << 7) | 0);
				break;
			case 6: // SST
				if (single_stroke_flag == 0)
				{
					single_stroke_flag = 1;
					txData[3] = ((SST_MODE_ON << 4) | (INT_MODE_OFF << 5)
							| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7) | 0);
				}
				else
				{
					txData[3] = ((SST_MODE_OFF << 4) | (INT_MODE_OFF << 5)
							| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7) | 0);
				}
				break;
			case 7: // STOP
				txData[3] = ((SST_MODE_OFF << 4) | (INT_MODE_OFF << 5)
						| (SPD1_MODE_OFF << 6) | (SPD2_MODE_OFF << 7) | 0);
				break;
			case 8: // OFF
				txData[2] = (0x00 | (golf7counter & 0x0F));
				txData[3] = 0x00;
				break;
			default:
				txData[3] = 0x00;  // byte 1
			}

			txData[4] = 0x00;
			txData[5] = 0x00;
			txData[6] = 0x00;
			txData[7] = 0x00;
			txData[8] = 0x00;
			txData[9] = 0x00; // byte 7

			txData[10] = Checksum_Calc(txData[1], txData + 2, 8); // lin 2.0/2.1 includes PID

			HAL_LIN_SendBreak(&huart3);
			HAL_UART_Transmit(&huart3, txData, 11, 100);

#ifdef USE_OLED
			ssd1306_SetCursor(2, 32);
			sprintf((char*) lcd_line, "actuator mode %s ",
					wiper_mode_name[wiper_mode]);
			ssd1306_WriteString((char*) lcd_line, Font_6x8, White);

			ssd1306_SetCursor(2, 45);
			sprintf((char*) lcd_line, "Tx %02X %02X %02X %02X %02X %02X",
					txData[2], txData[3], txData[4], txData[5], txData[6],
					txData[7]);
			ssd1306_WriteString((char*) lcd_line, Font_6x8, White);

			ssd1306_UpdateScreen();
#endif

			golf7counter++;

			HAL_Delay(5);
			// ask for diagnostics
			txData[0] = 0x55;  // sync field
			txData[1] = Pid_Calc(0x32);

			memset(rxData, 0x00, RX_BUFFER_SIZE);
			HAL_UARTEx_ReceiveToIdle_DMA(&huart3, rxData, RX_BUFFER_SIZE);
			__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);

			HAL_LIN_SendBreak(&huart3);
			HAL_UART_Transmit_DMA(&huart3, txData, 2);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == B1_Pin)
	{
		wiper_mode++;
		wiper_mode %= NUM_OF_MODES;
//		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3)
	{
		lin_data_received_flag = 1;
//		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	}
}

uint8_t Pid_Calc(uint8_t ID)
{
	if (ID > 0x3F)
		Error_Handler();
	uint8_t IDBuf[6];
	for (int i = 0; i < 6; i++)
	{
		IDBuf[i] = (ID >> i) & 0x01;
	}

	uint8_t P0 = (IDBuf[0] ^ IDBuf[1] ^ IDBuf[2] ^ IDBuf[4]) & 0x01;
	uint8_t P1 = ~((IDBuf[1] ^ IDBuf[3] ^ IDBuf[4] ^ IDBuf[5]) & 0x01);

	ID = ID | (P0 << 6) | (P1 << 7);
	return ID;
}

uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size)
{
	uint8_t buffer[size + 2];
	uint16_t sum = 0;
	buffer[0] = PID;
	for (int i = 0; i < size; i++)
	{
		buffer[i + 1] = data[i];
	}

	for (int i = 0; i < size + 1; i++)
	{
		sum = sum + buffer[i];
		if (sum > 0xff)
			sum = sum - 0xff;
	}

	sum = 0xff - sum;
	return sum;
}

static void showPartDatasheet(void)
{
	//const char *text = "Enjoy your life. There is no second take on it.";
	const char *text = "https://allegro.pl/listing?string=5G1955119A";
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

	// Make and print the QR Code symbol
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
	qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (ok)
		printQr(qrcode);
	displayQr(qrcode);

}

static void printQr(const uint8_t qrcode[])
{
	int size = qrcodegen_getSize(qrcode);
	int border = 4;
	for (int y = -border; y < size + border; y++)
	{
		for (int x = -border; x < size + border; x++)
		{
			printf("%s", qrcodegen_getModule(qrcode, x, y) ? "##" : "  ");
		}
		printf("\r\n");
	}
	printf("\r\n");
}

static void displayQr(const uint8_t qrcode[])
{
	int size = qrcodegen_getSize(qrcode);
	int border = 4;
	ssd1306_Fill(Black);
	for (int y = -border; y < size + border; y++)
	{
		for (int x = -border; x < size + border; x++)
		{
			if (qrcodegen_getModule(qrcode, x, y) == 1)
			{
				ssd1306_DrawPixel(2 * x + 40, 2 * y + 2, White);
				ssd1306_DrawPixel(2 * x + 1 + 40, 2 * y + 1 + 2, White);
				ssd1306_DrawPixel(2 * x + 40, 2 * y + 1 + 2, White);
				ssd1306_DrawPixel(2 * x + 1 + 40, 2 * y + 2, White);
				//ssd1306_FillRectangle(x, y, x+1, y+1,White);
			}
			//	printf("%s",qrcodegen_getModule(qrcode, x, y) ? "##" : "  ");
		}
		//	printf("\r\n");
	}
//	printf("\r\n");
	ssd1306_UpdateScreen();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
