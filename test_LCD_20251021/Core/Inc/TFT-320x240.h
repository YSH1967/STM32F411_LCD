#include "fonts.h"
#include <stdbool.h>

// LCD 디스플레이 방향 설정 관련 비트 정의
#define TFT_MADCTL_MY  0x80  // Y축 반전
#define TFT_MADCTL_MX  0x40  // X축 반전
#define TFT_MADCTL_MV  0x20  // X, Y 교환 (화면 회전)
#define TFT_MADCTL_ML  0x10  // 좌우 스캔 순서 제어
#define TFT_MADCTL_RGB 0x00  // RGB 색상 순서
#define TFT_MADCTL_BGR 0x08  // BGR 색상 순서
#define TFT_MADCTL_MH  0x04  // 상하 스캔 순서 제어

// 기본 디스플레이 해상도 및 초기 회전 방향
#define TFT_WIDTH  320   // 가로 해상도
#define TFT_HEIGHT 240   // 세로 해상도
#define TFT_ROTATION (TFT_MADCTL_MX | TFT_MADCTL_MY | TFT_MADCTL_MV | TFT_MADCTL_BGR)
// 기본 회전 설정: X축 반전 + Y축 반전 + XY 교환 + BGR 모드

// SPI 및 GPIO 핀 설정
#define TFT_SPI_PORT hspi1            // SPI 포트 핸들러
extern SPI_HandleTypeDef TFT_SPI_PORT;

#define TFT_RESET_PIN       GPIO_PIN_14   // 리셋 핀
#define TFT_RESET_GPIO_PORT GPIOB

#define TFT_CS_PIN          GPIO_PIN_13   // Chip Select 핀
#define TFT_CS_GPIO_PORT    GPIOB

#define TFT_DC_PIN          GPIO_PIN_15   // Data/Command 핀
#define TFT_DC_GPIO_PORT    GPIOB

// 색상 정의 (16비트 RGB565 형식)
#define TFT_BLACK   0x0000  // 검정
#define TFT_BLUE    0x001F  // 파랑
#define TFT_RED     0xF800  // 빨강
#define TFT_GREEN   0x07E0  // 초록
#define TFT_CYAN    0x07FF  // 청록
#define TFT_MAGENTA 0xF81F  // 자홍
#define TFT_YELLOW  0xFFE0  // 노랑
#define TFT_WHITE   0xFFFF  // 흰색

// RGB888(24비트) → RGB565(16비트) 변환 매크로
#define TFT_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


#define IMG_W 28
#define IMG_H 28

extern uint8_t img_buffer[IMG_H][IMG_W];



// 함수 프로토타입 (헤더에서 선언, 소스파일에서 정의)
void TFT_Init(void);   // TFT 초기화
void TFT_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor); // 문자열 출력
void TFT_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color); // 사각형 채우기
void TFT_FillScreen(uint16_t color);  // 화면 전체를 지정 색상으로 채우기
void LCD_Clear(void);   // 화면 지우기
void Debug_PrintBuffer(void);   // 디버그 프린트 함수
void Clear_ImageBuffer(void);   // 버퍼 초기화 함수




