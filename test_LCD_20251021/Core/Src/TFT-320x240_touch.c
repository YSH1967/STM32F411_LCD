#include "stm32f4xx_hal.h"
#include "TFT-320x240_touch.h"

// ==================================
// 터치 컨트롤러 명령 정의 (XPT2046 등)
// ==================================
#define TFT_TOUCH_READ_X 0xD0  // X 좌표 읽기 명령
#define TFT_TOUCH_READ_Y 0x90  // Y 좌표 읽기 명령


// ==================================
// 터치 입력 확인 함수
// ==================================
static bool TFT_TouchPressed()
{
    // 터치 IRQ 핀을 읽어서 터치 여부 반환
    // LOW(RESET) 상태이면 터치됨
    return HAL_GPIO_ReadPin(TFT_TOUCH_IRQ_GPIO_PORT, TFT_TOUCH_IRQ_PIN) == GPIO_PIN_RESET;
}


// ==================================
// 터치 좌표 읽기 함수
// ==================================
bool TFT_TouchGetCoordinates(uint16_t* x, uint16_t* y)
{
    // X, Y 읽기 명령 배열
    static const uint8_t cmd_read_x[] = { TFT_TOUCH_READ_X };
    static const uint8_t cmd_read_y[] = { TFT_TOUCH_READ_Y };
    // SPI에서 읽기 위해 보내는 더미 바이트
    static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

    // ==================================
    // 터치 선택(CS LOW)
    // ==================================
    HAL_GPIO_WritePin(TFT_TOUCH_CS_GPIO_PORT, TFT_TOUCH_CS_PIN, GPIO_PIN_RESET);

    uint32_t avg_x = 0;
    uint32_t avg_y = 0;
    uint8_t nsamples = 0;

    // ==================================
    // 여러 번 샘플링해서 평균 좌표 계산
    // ==================================
    for (uint8_t i = 0; i < 16; i++)
    {
        // 터치가 안되면 바로 루프 종료
        if (!TFT_TouchPressed())
            break;

        nsamples++;

        // Y 좌표 읽기
        HAL_SPI_Transmit(&TFT_TOUCH_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
        uint8_t y_raw[2];
        HAL_SPI_TransmitReceive(&TFT_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);

        // X 좌표 읽기
        HAL_SPI_Transmit(&TFT_TOUCH_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
        uint8_t x_raw[2];
        HAL_SPI_TransmitReceive(&TFT_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);

        // 읽은 raw 값 누적
        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
    }

    // ==================================
    // 터치 선택 해제(CS HIGH)
    // ==================================
    HAL_GPIO_WritePin(TFT_TOUCH_CS_GPIO_PORT, TFT_TOUCH_CS_PIN, GPIO_PIN_SET);

    // 샘플이 충분하지 않으면 터치 없다고 판단
    if (nsamples < 16)
        return false;

    // ==================================
    // 읽은 raw 좌표 정규화
    // ==================================
    uint32_t raw_x = (avg_x / 16);
    if (raw_x < TFT_TOUCH_MIN_RAW_X) raw_x = TFT_TOUCH_MIN_RAW_X;
    if (raw_x > TFT_TOUCH_MAX_RAW_X) raw_x = TFT_TOUCH_MAX_RAW_X;

    uint32_t raw_y = (avg_y / 16);
    if (raw_y < TFT_TOUCH_MIN_RAW_Y) raw_y = TFT_TOUCH_MIN_RAW_Y;
    if (raw_y > TFT_TOUCH_MAX_RAW_Y) raw_y = TFT_TOUCH_MAX_RAW_Y;

    // ==================================
    // 화면 크기에 맞게 좌표 스케일링
    // ==================================
    *x = (raw_x - TFT_TOUCH_MIN_RAW_X) * TFT_TOUCH_SCALE_X / (TFT_TOUCH_MAX_RAW_X - TFT_TOUCH_MIN_RAW_X);
    *y = (raw_y - TFT_TOUCH_MIN_RAW_Y) * TFT_TOUCH_SCALE_Y / (TFT_TOUCH_MAX_RAW_Y - TFT_TOUCH_MIN_RAW_Y);

    // 화면 좌우 반전 보정
    *y = TFT_TOUCH_SCALE_Y - 1 - *y;

    return true;
}
