#include "stm32f4xx_hal.h"
#include "rc522.h"

#define GREEN_LED_PIN GPIO_PIN_0
#define RED_LED_PIN GPIO_PIN_1
#define GREEN_LED_PORT GPIOB
#define RED_LED_PORT GPIOB

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

SPI_HandleTypeDef hspi1;

// Remplacer avec l'ID de la carte RFID
uint8_t correctID[5] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

void setup() {
    if (HAL_Init() != HAL_OK) {
        Error_Handler();
    }
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MFRC522_Init(&hspi1);
}

void loop() {
    uint8_t status;
    uint8_t str[MAX_LEN];
    
    status = MFRC522_Request(PICC_REQIDL, str);
    if (status == MI_OK) {
        status = MFRC522_Anticoll(str);
        if (status == MI_OK) {
            if (memcmp(str, correctID, 5) == 0) {
                HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
            } else {
                HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET);
            }
        }
    }
    HAL_Delay(100); // Ajout d'un délai de 100 ms
}

int main(void) {
    setup();
    while (1) {
        loop();
    }
}

// Configuration de l'horloge système, initialisation des GPIO et initialisation SPI

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure la tension de sortie du régulateur interne principal
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Initialise les oscillateurs RCC selon les paramètres spécifiés dans la structure RCC_OscInitTypeDef
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Initialise les horloges CPU, AHB et APB
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    // Activation de l'horloge des ports GPIO
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure le niveau de sortie des broches GPIO
    HAL_GPIO_WritePin(GPIOB, GREEN_LED_PIN | RED_LED_PIN, GPIO_PIN_RESET);

    // Configure les broches GPIO : GREEN_LED_PIN et RED_LED_PIN
    GPIO_InitStruct.Pin = GREEN_LED_PIN | RED_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void MX_SPI1_Init(void) {
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        // Erreur d'initialisation
        Error_Handler();
    }

    // Configuration des broches SPI1
    // Par exemple, SPI1 utilise :
    // - PA5 pour SCK
    // - PA6 pour MISO
    // - PA7 pour MOSI
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void) {
    // Implémentation de la gestion des erreurs
    while (1) {
        // Optionnellement faire clignoter une LED ou envoyer un message de débogage
    }
}
