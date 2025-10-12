#include"math.h"
#include"tft.h"
#include"main.h"

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  160


#define LCD_CMD_END 0xFF
static const uint8_t u8InitCmdList[] = {
    // Command, Length, Data...
    0xB1, 0x03, 0x01, 0x2C, 0x2D, // Frame Rate Control (In normal mode)
    0xB2, 0x03, 0x01, 0x2C, 0x2D, // Frame Rate Control (In Idle mode)
    0xB3, 0x06, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D, // Frame Rate Control (In Partial mode)
    0xB4, 0x01, 0x07, // Display Inversion Control
    0xC0, 0x03, 0xA2, 0x02, 0x84, // Power Control 1
    0xC1, 0x01, 0xC5, // Power Control 2
    0xC2, 0x02, 0x0A, 0x00, // Power Control 3 (in normal mode)
    0xC3, 0x02, 0x8A, 0x2A, // Power Control 4 (in Idle mode)
    0xC4, 0x02, 0x8A, 0xEE, // Power Control 5 (in Partial mode)
    0xC5, 0x01, 0x0E, // VCOM Control 1

    // E0: Positive Gamma (0x10 byte data)
    0xE0, 0x10, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,

    // E1: Negative Gamma (0x10 byte data)
    0xE1, 0x10, 0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,

    LCD_CMD_END, LCD_CMD_END // Dấu kết thúc danh sách
};
void writeCMDTFT(uint8_t cmd)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0); // CS = 0
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 0); // A0 = 0 -> CMD
    //HAL_SPI_Transmit(hspi1, &cmd, 1, 1000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1); // CS = 1
}

void writeDataTFT(uint8_t data)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 1); // A0 = 1 -> DATA
    //HAL_SPI_Transmit(hspi1, &data, 1, 1000);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
}
void sendCMDList(const uint8_t * cmdList) {
    uint8_t index = 0;
    uint8_t cmd = 0;
    uint8_t num = 0;

    while(1) {
        cmd = *cmdList++;
        num = *cmdList++;

        if (cmd == LCD_CMD_END) {
            break;
        } else {
            writeCMDTFT(cmd);
            for(index = 0; index < num; index++) {
                writeDataTFT(*cmdList++);
            }
        }
    }
}
void setPos(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    writeCMDTFT(0x2A);
    writeDataTFT(0x00);
    writeDataTFT(x1);
    writeDataTFT(0x00);
    writeDataTFT(x2);
    writeCMDTFT(0x2B);
    writeDataTFT(0x00);
    writeDataTFT(y1);
    writeDataTFT(0x00);
    writeDataTFT(y2);
}
// RGB 16bit 565


// Hàm drawPixel uint16_t
void drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) { // 128x160
        return;
    }
    setPos(x, y, x+1, y+1);
    writeCMDTFT(0x2C);
    writeDataTFT(color >> 8);

    writeDataTFT(color & 0xFF);

}

// Hàm fullDisplay cũng cần được cập nhật để sử dụng kích thước mới
void fullDisplay(uint16_t color) {
    writeCMDTFT(0x2C);
    int i;
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        writeDataTFT(color >> 8); // Byte Cao

        writeDataTFT(color & 0xFF); // Byte Thấp
    }
}


void drawChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bg)
{
    uint16_t i, j;
    uint16_t pixelData;
    for(i = 0; i < font.height; i++) {
        pixelData = font.data[(ch - 32)*font.height + i];
        for(j = 0; j < font.width; j++){
            if ((pixelData << j) & 0x8000 ) {
                drawPixel(x+j, y+i, color);
            } else {
                drawPixel(x+j, y+i, bg);
            }
        }
    }
}
void drawString(uint8_t x, uint8_t y, char *str, FontDef font, uint16_t color, uint16_t bg) {
    while (*str) {
        drawChar(x, y, *str, font, color, bg);
        x += font.width;
        str++;
    }
}
void initTFT()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
    HAL_Delay(150);

    // SW RESET
    writeCMDTFT(0x01);
    HAL_Delay(150);

    // Sleep out
    writeCMDTFT(0x11);
    HAL_Delay(255);

    sendCMDList(u8InitCmdList);

    writeCMDTFT(0x36); // Lệnh MADCTL
    writeDataTFT(0x00);


    writeCMDTFT(0x3A); // Interface Pixel Format
    writeDataTFT(0x05);

    writeCMDTFT(0x20); // Display inversion off

    setPos(0, 0, 128, 160);
    //EN display
    writeCMDTFT(0x29);
    HAL_Delay(100);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

