#ifndef __TFT_TOUCH_H__
#define __TFT_TOUCH_H__

#include <stdbool.h>

// ⚠️ 주의: SPI 통신 속도는 반드시 1.3 Mbit 이하로 설정해야 함.
//          안전하게는 약 650 Kbit 정도 권장
#define TFT_TOUCH_SPI_PORT hspi1
extern SPI_HandleTypeDef TFT_TOUCH_SPI_PORT;

// 터치패널 제어용 GPIO 핀 정의
#define TFT_TOUCH_IRQ_PIN        GPIO_PIN_1   // 터치 이벤트 감지용 인터럽트 핀
#define TFT_TOUCH_IRQ_GPIO_PORT  GPIOB

#define TFT_TOUCH_CS_PIN         GPIO_PIN_2   // Chip Select 핀
#define TFT_TOUCH_CS_GPIO_PORT   GPIOB

// 화면 방향(회전 여부)에 따라 변경해야 하는 값
#define TFT_TOUCH_SCALE_X 240   // X축 해상도 (TFT 가로 크기)
#define TFT_TOUCH_SCALE_Y 320   // Y축 해상도 (TFT 세로 크기)

// 보정(calibration) 값 설정
// ※ 실제 사용하는 터치스크린 패널에 따라 최소/최대 RAW 값 다를 수 있음
//    필요시 TFT-320x240_touch.c 파일에서 UART_Printf를 활성화하여 측정 가능
#define TFT_TOUCH_MIN_RAW_X 1500   // 터치 X축 최소 RAW 값
#define TFT_TOUCH_MAX_RAW_X 31000  // 터치 X축 최대 RAW 값
#define TFT_TOUCH_MIN_RAW_Y 3276   // 터치 Y축 최소 RAW 값
#define TFT_TOUCH_MAX_RAW_Y 30110  // 터치 Y축 최대 RAW 값

// 터치 좌표 읽기 함수
// 성공 시 true 반환, *x / *y 에 보정된 화면 좌표 값 저장
bool TFT_TouchGetCoordinates(uint16_t* x, uint16_t* y);

#endif // __TFT_TOUCH_H__
