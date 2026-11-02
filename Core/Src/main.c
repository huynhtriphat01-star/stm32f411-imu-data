/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32F411 + MPU6500 + I2C DMA + TIM2 100Hz
  *          + TIM5 1us timestamp + USB CDC + 21-byte frame + CRC8
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

/* -------------------------------------------------------------------------- */
/* Peripheral handles                                                         */
/* -------------------------------------------------------------------------- */

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;

/* -------------------------------------------------------------------------- */
/* MPU6500                                                                    */
/* -------------------------------------------------------------------------- */

#define MPU_ADDR              (0x68 << 1)

#define MPU_REG_SMPLRT_DIV    0x19
#define MPU_REG_CONFIG        0x1A
#define MPU_REG_GYRO_CONFIG   0x1B
#define MPU_REG_ACCEL_CONFIG  0x1C
#define MPU_REG_ACCEL_CONFIG2 0x1D
#define MPU_REG_ACCEL_XOUT_H  0x3B
#define MPU_REG_PWR_MGMT_1    0x6B
#define MPU_REG_WHO_AM_I      0x75

/* -------------------------------------------------------------------------- */
/* Protocol                                                                   */
/* -------------------------------------------------------------------------- */

#define FRAME_SIZE 21

/*
 * 21-byte frame:
 *
 * Byte  0       : 0xAA
 * Byte  1       : 0x55
 * Byte  2..3    : sequence        uint16 little-endian
 * Byte  4..7    : timestamp_us    uint32 little-endian
 * Byte  8..9    : ax              int16 little-endian
 * Byte 10..11   : ay
 * Byte 12..13   : az
 * Byte 14..15   : gx
 * Byte 16..17   : gy
 * Byte 18..19   : gz
 * Byte 20       : CRC-8 polynomial 0x07
 */

/* -------------------------------------------------------------------------- */
/* Debug / status variables                                                   */
/* -------------------------------------------------------------------------- */

volatile uint8_t sample_request = 0;
volatile uint8_t dma_busy       = 0;
volatile uint8_t dma_done       = 0;

volatile uint32_t timer_irq_count       = 0;
volatile uint32_t dma_start_count       = 0;
volatile uint32_t dma_complete_count    = 0;
volatile uint32_t dma_error_count       = 0;
volatile uint32_t missed_deadline_count = 0;

volatile uint32_t usb_tx_count   = 0;
volatile uint32_t usb_busy_count = 0;

volatile uint32_t timestamp_us = 0;

uint8_t who_am_i = 0;
uint8_t mpu_ok   = 0;

HAL_StatusTypeDef i2c_status;

/* MPU raw burst: accel 6 + temperature 2 + gyro 6 = 14 bytes */
uint8_t imu_data[14];

/* Raw sensor values */
int16_t ax_raw = 0;
int16_t ay_raw = 0;
int16_t az_raw = 0;

int16_t gx_raw = 0;
int16_t gy_raw = 0;
int16_t gz_raw = 0;

/* Converted values for Keil Watch */
float ax_g = 0.0f;
float ay_g = 0.0f;
float az_g = 0.0f;

float gx_dps = 0.0f;
float gy_dps = 0.0f;
float gz_dps = 0.0f;

/* Protocol */
uint16_t sequence = 0;
uint8_t tx_frame[FRAME_SIZE];

/* -------------------------------------------------------------------------- */
/* Function prototypes                                                        */
/* -------------------------------------------------------------------------- */

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM5_Init(void);

static HAL_StatusTypeDef MPU6500_Write(uint8_t reg, uint8_t value);
static HAL_StatusTypeDef MPU6500_Init(void);

static void Start_IMU_DMA(void);
static void Process_IMU_Data(void);

static uint8_t CRC8(const uint8_t *data, uint32_t length);

static void Put_U16_LE(uint8_t *p, uint16_t value);
static void Put_U32_LE(uint8_t *p, uint32_t value);
static void Put_I16_LE(uint8_t *p, int16_t value);

static void Build_Frame(void);

/* ========================================================================== */
/* MAIN                                                                       */
/* ========================================================================== */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    /*
     * DMA must be initialized before I2C is used.
     */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();
    MX_TIM5_Init();

    /*
     * Initialize USB CDC.
     */
    MX_USB_DEVICE_Init();

    /*
     * Allow Windows enough time to enumerate the CDC device.
     */
    HAL_Delay(1000);

    /* ---------------------------------------------------------------------- */
    /* MPU6500 initialization                                                 */
    /* ---------------------------------------------------------------------- */

    if (MPU6500_Init() == HAL_OK)
    {
        mpu_ok = 1;
    }
    else
    {
        mpu_ok = 0;

        /*
         * Sensor init failed.
         * Stay here so the error is obvious in debugger.
         */
        while (1)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(200);
        }
    }

    /* ---------------------------------------------------------------------- */
    /* Start TIM5: 1 MHz -> 1 tick = 1 us                                     */
    /* ---------------------------------------------------------------------- */

    __HAL_TIM_SET_COUNTER(&htim5, 0);

    if (HAL_TIM_Base_Start(&htim5) != HAL_OK)
    {
        Error_Handler();
    }

    /* ---------------------------------------------------------------------- */
    /* Start TIM2: 100 Hz sampling interrupt                                  */
    /* ---------------------------------------------------------------------- */

    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    /* ====================================================================== */
    /* Main loop                                                              */
    /* ====================================================================== */

    while (1)
    {
        /*
         * TIM2 ISR only requests a sample.
         *
         * Blocking I2C work is NOT performed inside TIM2 ISR.
         */
        if (sample_request != 0U)
        {
            sample_request = 0;

            if ((dma_busy == 0U) && (dma_done == 0U))
            {
                Start_IMU_DMA();
            }
            else
            {
                missed_deadline_count++;
            }
        }

        /*
         * DMA callback indicates that one complete 14-byte IMU burst
         * is available.
         */
        if (dma_done != 0U)
        {
            uint8_t usb_status;

            dma_done = 0;

            Process_IMU_Data();

            Build_Frame();

            /*
             * Non-blocking USB CDC transmission.
             */
            usb_status = CDC_Transmit_FS(tx_frame, FRAME_SIZE);

            if (usb_status == USBD_OK)
            {
                usb_tx_count++;
            }
            else
            {
                /*
                 * Usually USBD_BUSY.
                 * Sequence still increments so PC can detect dropped frames.
                 */
                usb_busy_count++;
            }

            sequence++;
        }
    }
}

/* ========================================================================== */
/* MPU6500                                                                    */
/* ========================================================================== */

static HAL_StatusTypeDef MPU6500_Write(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(
        &hi2c1,
        MPU_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        100
    );
}

static HAL_StatusTypeDef MPU6500_Init(void)
{
    uint8_t id = 0;

    HAL_Delay(100);

    /*
     * WHO_AM_I
     * MPU6500 should return 0x70.
     */
    if (HAL_I2C_Mem_Read(
            &hi2c1,
            MPU_ADDR,
            MPU_REG_WHO_AM_I,
            I2C_MEMADD_SIZE_8BIT,
            &id,
            1,
            100
        ) != HAL_OK)
    {
        return HAL_ERROR;
    }

    who_am_i = id;

    if (who_am_i != 0x70)
    {
        return HAL_ERROR;
    }

    /*
     * PWR_MGMT_1
     *
     * CLKSEL = 1
     * PLL with X axis gyroscope reference.
     */
    if (MPU6500_Write(MPU_REG_PWR_MGMT_1, 0x01) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_Delay(100);

    /*
     * SMPLRT_DIV = 0
     */
    if (MPU6500_Write(MPU_REG_SMPLRT_DIV, 0x00) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /*
     * CONFIG:
     * DLPF_CFG = 3
     */
    if (MPU6500_Write(MPU_REG_CONFIG, 0x03) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /*
     * GYRO_CONFIG:
     * FS_SEL = 1
     * +/-500 deg/s
     * sensitivity = 65.5 LSB/(deg/s)
     */
    if (MPU6500_Write(MPU_REG_GYRO_CONFIG, 0x08) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /*
     * ACCEL_CONFIG:
     * AFS_SEL = 1
     * +/-4g
     * sensitivity = 8192 LSB/g
     */
    if (MPU6500_Write(MPU_REG_ACCEL_CONFIG, 0x08) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /*
     * ACCEL_CONFIG2:
     * accelerometer DLPF = 3
     */
    if (MPU6500_Write(MPU_REG_ACCEL_CONFIG2, 0x03) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_Delay(100);

    return HAL_OK;
}

/* ========================================================================== */
/* DMA IMU acquisition                                                        */
/* ========================================================================== */

static void Start_IMU_DMA(void)
{
    HAL_StatusTypeDef status;

    dma_busy = 1;

    status = HAL_I2C_Mem_Read_DMA(
        &hi2c1,
        MPU_ADDR,
        MPU_REG_ACCEL_XOUT_H,
        I2C_MEMADD_SIZE_8BIT,
        imu_data,
        14
    );

    if (status == HAL_OK)
    {
        dma_start_count++;
    }
    else
    {
        dma_busy = 0;
        dma_error_count++;
    }
}

/* ========================================================================== */
/* Sensor conversion                                                          */
/* ========================================================================== */

static void Process_IMU_Data(void)
{
    ax_raw = (int16_t)(
        ((uint16_t)imu_data[0] << 8) |
        imu_data[1]
    );

    ay_raw = (int16_t)(
        ((uint16_t)imu_data[2] << 8) |
        imu_data[3]
    );

    az_raw = (int16_t)(
        ((uint16_t)imu_data[4] << 8) |
        imu_data[5]
    );

    /*
     * imu_data[6], imu_data[7] = temperature
     */

    gx_raw = (int16_t)(
        ((uint16_t)imu_data[8] << 8) |
        imu_data[9]
    );

    gy_raw = (int16_t)(
        ((uint16_t)imu_data[10] << 8) |
        imu_data[11]
    );

    gz_raw = (int16_t)(
        ((uint16_t)imu_data[12] << 8) |
        imu_data[13]
    );

    /*
     * Accelerometer +/-4g
     * 8192 LSB/g
     */
    ax_g = (float)ax_raw / 8192.0f;
    ay_g = (float)ay_raw / 8192.0f;
    az_g = (float)az_raw / 8192.0f;

    /*
     * Gyroscope +/-500 deg/s
     * 65.5 LSB/(deg/s)
     */
    gx_dps = (float)gx_raw / 65.5f;
    gy_dps = (float)gy_raw / 65.5f;
    gz_dps = (float)gz_raw / 65.5f;
}

/* ========================================================================== */
/* Binary frame                                                               */
/* ========================================================================== */

static void Put_U16_LE(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void Put_U32_LE(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xFFU);
    p[1] = (uint8_t)((value >> 8) & 0xFFU);
    p[2] = (uint8_t)((value >> 16) & 0xFFU);
    p[3] = (uint8_t)((value >> 24) & 0xFFU);
}

static void Put_I16_LE(uint8_t *p, int16_t value)
{
    Put_U16_LE(p, (uint16_t)value);
}

static void Build_Frame(void)
{
    tx_frame[0] = 0xAA;
    tx_frame[1] = 0x55;

    Put_U16_LE(&tx_frame[2], sequence);

    Put_U32_LE(&tx_frame[4], timestamp_us);

    Put_I16_LE(&tx_frame[8],  ax_raw);
    Put_I16_LE(&tx_frame[10], ay_raw);
    Put_I16_LE(&tx_frame[12], az_raw);

    Put_I16_LE(&tx_frame[14], gx_raw);
    Put_I16_LE(&tx_frame[16], gy_raw);
    Put_I16_LE(&tx_frame[18], gz_raw);

    /*
     * CRC calculated over byte 0..19.
     */
    tx_frame[20] = CRC8(tx_frame, 20);
}

/* ========================================================================== */
/* CRC-8                                                                      */
/* Polynomial: 0x07                                                           */
/* Initial value: 0x00                                                        */
/* ========================================================================== */

static uint8_t CRC8(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0x00;

    uint32_t i;
    uint8_t bit;

    for (i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (bit = 0; bit < 8; bit++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1) ^ 0x07U);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/* ========================================================================== */
/* HAL callbacks                                                              */
/* ========================================================================== */

/*
 * TIM2 interrupt:
 * exactly 100 Hz -> one request every 10 ms.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_irq_count++;

        if ((sample_request == 0U) &&
            (dma_busy == 0U) &&
            (dma_done == 0U))
        {
            sample_request = 1;
        }
        else
        {
            missed_deadline_count++;
        }
    }
}

/*
 * I2C DMA transfer complete.
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        /*
         * Capture high-resolution timestamp immediately
         * when the IMU burst completes.
         */
        timestamp_us =
            __HAL_TIM_GET_COUNTER(&htim5);

        dma_busy = 0;
        dma_done = 1;

        dma_complete_count++;
    }
}

/*
 * I2C error callback.
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        dma_busy = 0;
        dma_done = 0;

        dma_error_count++;
    }
}

/* ========================================================================== */
/* CLOCK                                                                      */
/* HSE = 25 MHz                                                               */
/* SYSCLK = 84 MHz                                                            */
/* USB clock = 48 MHz                                                         */
/* ========================================================================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState =
        RCC_HSE_ON;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    /*
     * APB1 = 42 MHz
     * TIM2/TIM5 clock = 84 MHz
     */
    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ========================================================================== */
/* I2C1                                                                       */
/* ========================================================================== */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;

    hi2c1.Init.OwnAddress1 = 0;

    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c1.Init.OwnAddress2 = 0;

    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ========================================================================== */
/* TIM2                                                                       */
/* TIM clock = 84 MHz                                                         */
/* PSC = 8399 -> 10 kHz                                                       */
/* ARR = 99   -> 100 Hz                                                       */
/* ========================================================================== */

static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;

    htim2.Init.Prescaler = 8399;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 99;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource =
        TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(
            &htim2,
            &sClockSourceConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger =
        TIM_TRGO_RESET;

    sMasterConfig.MasterSlaveMode =
        TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(
            &htim2,
            &sMasterConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ========================================================================== */
/* TIM5                                                                       */
/* TIM clock = 84 MHz                                                         */
/* PSC = 83 -> 1 MHz                                                          */
/* 1 tick = 1 microsecond                                                     */
/* ========================================================================== */

static void MX_TIM5_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim5.Instance = TIM5;

    htim5.Init.Prescaler = 83;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;

    htim5.Init.Period = 0xFFFFFFFFU;

    htim5.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;

    htim5.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource =
        TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(
            &htim5,
            &sClockSourceConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger =
        TIM_TRGO_RESET;

    sMasterConfig.MasterSlaveMode =
        TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(
            &htim5,
            &sMasterConfig
        ) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ========================================================================== */
/* DMA                                                                        */
/* ========================================================================== */

static void MX_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    /*
     * I2C1_RX = DMA1 Stream0
     */
    HAL_NVIC_SetPriority(
        DMA1_Stream0_IRQn,
        0,
        0
    );

    HAL_NVIC_EnableIRQ(
        DMA1_Stream0_IRQn
    );
}

/* ========================================================================== */
/* GPIO                                                                       */
/* ========================================================================== */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*
     * PC13 onboard LED.
     */
    HAL_GPIO_WritePin(
        GPIOC,
        GPIO_PIN_13,
        GPIO_PIN_SET
    );

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(
        GPIOC,
        &GPIO_InitStruct
    );
}

/* ========================================================================== */
/* ERROR HANDLER                                                              */
/* ========================================================================== */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}

#endif