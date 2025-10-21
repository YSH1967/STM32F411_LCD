#include "stm32f4xx_hal.h"
#include "TFT-320x240.h"
#include <string.h>


extern UART_HandleTypeDef huart2;



uint8_t img_buffer[IMG_H][IMG_W];  // AI 입력 버퍼 (28x28)


// =============================
// TFT LCD 드라이버 내부 함수 선언
// =============================

// 명령어 전송 함수
static void TFT_WriteCommand(uint8_t cmd);
// 데이터 전송 함수
static void TFT_WriteData(uint8_t* buffer, size_t buffer_size);
// 화면에 그릴 영역 지정 함수
static void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
// 문자 출력 함수
static void TFT_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);


// =============================
// TFT LCD 내부 동작 함수
// =============================

// TFT에 명령어 전송
static void TFT_WriteCommand(uint8_t cmd)
{
    // DC 핀을 LOW로 설정 -> 명령어 모드
    HAL_GPIO_WritePin(TFT_DC_GPIO_PORT, TFT_DC_PIN, GPIO_PIN_RESET);
    // SPI로 명령어 전송
    HAL_SPI_Transmit(&TFT_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

// TFT에 데이터 전송
static void TFT_WriteData(uint8_t* buffer, size_t buffer_size)
{
    // DC 핀을 HIGH로 설정 -> 데이터 모드
    HAL_GPIO_WritePin(TFT_DC_GPIO_PORT, TFT_DC_PIN, GPIO_PIN_SET);

    // HAL_SPI_Transmit는 한 번에 64KB 이상 전송할 수 없으므로 분할 전송
    while (buffer_size > 0)
    {
        uint16_t chunk_size = buffer_size > 32768 ? 32768 : buffer_size; // 최대 32KB씩 전송
        HAL_SPI_Transmit(&TFT_SPI_PORT, buffer, chunk_size, HAL_MAX_DELAY);
        buffer += chunk_size;
        buffer_size -= chunk_size;
    }
}

// 화면에서 그릴 영역(x0,y0 ~ x1,y1) 지정
static void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // 컬럼 주소 설정 (가로)
    TFT_WriteCommand(0x2A); // CASET 명령
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        TFT_WriteData(data, sizeof(data));
    }

    // 행 주소 설정 (세로)
    TFT_WriteCommand(0x2B); // RASET 명령
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        TFT_WriteData(data, sizeof(data));
    }

    // RAM에 쓰기 시작
    TFT_WriteCommand(0x2C); // RAMWR 명령
}


// =============================
// TFT 초기화 함수
// =============================
void TFT_Init()
{
    // TFT 선택(CS LOW)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    // TFT 리셋
    HAL_GPIO_WritePin(TFT_RESET_GPIO_PORT, TFT_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(TFT_RESET_GPIO_PORT, TFT_RESET_PIN, GPIO_PIN_SET);

    // ILI9341 초기화 명령어 시퀀스
    // GitHub 예제 기준: https://github.com/martnak/STM32-ILI9341

    // 소프트웨어 리셋
    TFT_WriteCommand(0x01);
    HAL_Delay(1000);

    // 전원 제어 A
    TFT_WriteCommand(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        TFT_WriteData(data, sizeof(data));
    }

    // 전원 제어 B
    TFT_WriteCommand(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        TFT_WriteData(data, sizeof(data));
    }

    // 드라이버 타이밍 제어 A
    TFT_WriteCommand(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        TFT_WriteData(data, sizeof(data));
    }

    // 드라이버 타이밍 제어 B
    TFT_WriteCommand(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        TFT_WriteData(data, sizeof(data));
    }

    // 전원 ON 시퀀스 제어
    TFT_WriteCommand(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        TFT_WriteData(data, sizeof(data));
    }

    // 펌프 비율 제어
    TFT_WriteCommand(0xF7);
    {
        uint8_t data[] = { 0x20 };
        TFT_WriteData(data, sizeof(data));
    }

    // 전원 제어 VRH
    TFT_WriteCommand(0xC0);
    {
        uint8_t data[] = { 0x23 };
        TFT_WriteData(data, sizeof(data));
    }

    // 전원 제어 SAP/BT
    TFT_WriteCommand(0xC1);
    {
        uint8_t data[] = { 0x10 };
        TFT_WriteData(data, sizeof(data));
    }

    // VCM 제어
    TFT_WriteCommand(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        TFT_WriteData(data, sizeof(data));
    }

    // VCM 제어 2
    TFT_WriteCommand(0xC7);
    {
        uint8_t data[] = { 0x86 };
        TFT_WriteData(data, sizeof(data));
    }

    // 메모리 액세스 제어 (화면 회전)
    TFT_WriteCommand(0x36);
    {
        uint8_t data[] = { TFT_ROTATION };
        TFT_WriteData(data, sizeof(data));
    }

    // 픽셀 포맷 16비트
    TFT_WriteCommand(0x3A);
    {
        uint8_t data[] = { 0x55 };
        TFT_WriteData(data, sizeof(data));
    }

    // 프레임 비율 제어
    TFT_WriteCommand(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        TFT_WriteData(data, sizeof(data));
    }

    // 디스플레이 기능 제어
    TFT_WriteCommand(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        TFT_WriteData(data, sizeof(data));
    }

    // 3감마 함수 비활성화
    TFT_WriteCommand(0xF2);
    {
        uint8_t data[] = { 0x00 };
        TFT_WriteData(data, sizeof(data));
    }

    // 감마 곡선 선택
    TFT_WriteCommand(0x26);
    {
        uint8_t data[] = { 0x01 };
        TFT_WriteData(data, sizeof(data));
    }

    // 양(+) 감마 보정
    TFT_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        TFT_WriteData(data, sizeof(data));
    }

    // 음(-) 감마 보정
    TFT_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        TFT_WriteData(data, sizeof(data));
    }

    // 슬립 모드 종료
    TFT_WriteCommand(0x11);
    HAL_Delay(120);

    // 디스플레이 ON
    TFT_WriteCommand(0x29);

    // TFT 선택 해제(CS HIGH)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}


// =============================
// 문자 출력 함수
// =============================
static void TFT_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;

    // 문자 크기만큼 주소 윈도우 설정
    TFT_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    // 글자 픽셀 데이터 출력
    for (i = 0; i < font.height; i++)
    {
        b = font.data[(ch - 32) * font.height + i]; // ASCII - 32 = 폰트 배열 시작 인덱스
        for (j = 0; j < font.width; j++)
        {
            if ((b << j) & 0x8000)  // 글자 색
            {
                uint8_t data[] = { color >> 8, color & 0xFF };
                TFT_WriteData(data, sizeof(data));
            }
            else      // 배경 색
            {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                TFT_WriteData(data, sizeof(data));
            }
        }
    }
}

// 문자열 출력 함수
void TFT_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    // TFT 선택(CS LOW)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    while (*str)
    {
        // 가로 끝을 넘어가면 다음 줄로
        if (x + font.width >= TFT_WIDTH)
        {
            x = 0;
            y += font.height;
            if (y + font.height >= TFT_HEIGHT) break; // 세로 끝이면 종료
        }

        TFT_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    // TFT 선택 해제(CS HIGH)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}


// =============================
// 도형 출력 함수
// =============================

// 사각형 채우기
void TFT_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // 화면 범위 체크
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;
    if (x + w > TFT_WIDTH) w = TFT_WIDTH - x;
    if (y + h > TFT_HEIGHT) h = TFT_HEIGHT - y;

    // TFT 선택(CS LOW)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    // 주소 윈도우 설정
    TFT_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    // 픽셀 데이터 전송
    uint8_t data[] = { color >> 8, color & 0xFF };
    HAL_GPIO_WritePin(TFT_DC_GPIO_PORT, TFT_DC_PIN, GPIO_PIN_SET); // 데이터 모드
    for (uint32_t i = 0; i < w * h; i++)
    {
        HAL_SPI_Transmit(&TFT_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
    }

    // TFT 선택 해제(CS HIGH)
    HAL_GPIO_WritePin(TFT_CS_GPIO_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

// 화면 전체 채우기
void TFT_FillScreen(uint16_t color)
{
    TFT_FillRectangle(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

// 화면 전체 지우기
void LCD_Clear(void)
{
    TFT_FillScreen(TFT_WHITE);
    memset(img_buffer, 0, sizeof(img_buffer));
}

// 디버그 프린트 함수
void Debug_PrintBuffer(void)
{
    for (int y = 0; y < IMG_H; y++) {
        for (int x = 0; x < IMG_W; x++) {
            char c = (img_buffer[y][x] > 128) ? '#' : '.';
            HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, 10);
        }
        char newline = '\n';
        HAL_UART_Transmit(&huart2, (uint8_t*)&newline, 1, 10);
    }
}

// 버퍼 초기화 함수
void Clear_ImageBuffer(void)
{
    for (int y = 0; y < IMG_H; y++) {
        for (int x = 0; x < IMG_W; x++) {
            img_buffer[y][x] = 0;  // 배경은 0
        }
    }
}
