#include "epd_1in54.h"

static uint8_t cmdBuf[10];
static uint8_t epd_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

static const uint8_t set[] = {1, 2, 4, 8, 16, 32, 64, 128},
                     clr[] = {(uint8_t)~1,  (uint8_t)~2,  (uint8_t)~4,
                              (uint8_t)~8,  (uint8_t)~16, (uint8_t)~32,
                              (uint8_t)~64, (uint8_t)~128};
                     
static bool full_refresh = true;

void epd_waitBusy(void)
{ 
    
    while(digitalRead(EINK_BUSY))
    {
        __WFE;
    }
        
}

void epd_writeCommand(uint8_t cmd, uint8_t* data, int32_t len)
{

    digitalWrite(EINK_CS, LOW);
    digitalWrite(EINK_DC, LOW);
    
	spi_transfer(&cmd, 1, NULL, 0);
    
    if(len)
    {
        digitalWrite(EINK_DC, HIGH);
        spi_send(data, len);
    }
    
	digitalWrite(EINK_CS, HIGH);
}

void epd_begin(void)
{
    
    pinMode(EINK_CS, OUTPUT);
    pinMode(EINK_DC, OUTPUT);
    pinMode(EINK_RESET, OUTPUT);
    pinMode(EINK_BUSY, INPUT);
    
    digitalWrite(EINK_CS, HIGH);
    
    spi_init();
    
    digitalWrite(EINK_RESET, LOW);
    delay(10);
    digitalWrite(EINK_RESET, HIGH);
    delay(10);
    
	epd_writeCommand(0x12, NULL, 0);  //SWRESET
    delay(10);
    
    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
	epd_writeCommand(0x01, cmdBuf, 3); //Driver output control

    cmdBuf[0] = 0x05;
	epd_writeCommand(0x3C, cmdBuf, 1); //Border Wavefrom
    
    cmdBuf[0] = 0x80;
	epd_writeCommand(0x18, cmdBuf, 1); //Internal temperature
    
    memset(epd_framebuffer, 0, sizeof(epd_framebuffer));
	
}

void epd_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    
    cmdBuf[0] = 0x03;
    epd_writeCommand(0x11, cmdBuf, 1); //data entry mode

    cmdBuf[0] = x / 8;
    cmdBuf[1] = (x + w - 1) / 8;
    epd_writeCommand(0x44, cmdBuf, 2);

    cmdBuf[0] = y % 256;
    cmdBuf[1] = y / 256;
    cmdBuf[2] = (y + h - 1) % 256;
    cmdBuf[3] = (y + h - 1) / 256;
    epd_writeCommand(0x45, cmdBuf, 4);
    
    cmdBuf[0] = x / 8;
	epd_writeCommand(0x4E, cmdBuf, 1);
    
    cmdBuf[0] = y % 256;
    cmdBuf[1] = y / 256;
	epd_writeCommand(0x4F, cmdBuf, 2);
    
    epd_waitBusy();
}

void epd_clearScreen(bool white)
{
    
    epd_setPartialRamArea(0, 0, 200, 200);
    
    cmdBuf[0] = 0x24;
    
    digitalWrite(EINK_DC, LOW);
    digitalWrite(EINK_CS, LOW);
    
    spi_transfer(cmdBuf, 1, NULL, 0);
    
    digitalWrite(EINK_DC, HIGH);
    
    cmdBuf[0] = white ? 0xFF : 0x00;
    
    for(int32_t i = 0; i < 5000; i++)
    {
        spi_transfer(cmdBuf, 1, NULL, 0);
    }
    
    digitalWrite(EINK_CS, HIGH);
    
    epd_displayFrame();

}

void epd_sendFrame(int x, int y, int w, int h, uint8_t* frame_buffer)
{
    
    epd_setPartialRamArea(x, y, w, h);
    
    epd_writeCommand(0x24, frame_buffer, w * h / 8);
}

void epd_update_u8(uint8_t y, uint8_t* line_buffer)
{

    for(int i = 0; i < SCREEN_WIDTH; i++){
		
		if(line_buffer[199 - i]){
			
			epd_framebuffer[(25 * y) + 24 - (i / 8)] |= set[i & 7];
		
		} else {
		
			epd_framebuffer[(25 * y) + 24 - (i / 8)] &= clr[i & 7];
		
		}
	
	}
    
    
    
}

void nextFullrefresh(void)
{
    
    full_refresh = true;

}

void epd_update_framebuffer(void)
{
    
    epd_sendFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, epd_framebuffer);
    
    if(full_refresh)
    {
        epd_displayFrame();
        full_refresh = false;
    
    } else
    {
        epd_displayPartFrame();
    }
    
}

void epd_displayFrame(void)
{
	//DISPLAY REFRESH
	cmdBuf[0] = 0xF7;
	epd_writeCommand(0x22, cmdBuf, 1);
	epd_writeCommand(0x20, NULL, 0);  //Master Activation
	epd_waitBusy();
}

void epd_displayPartFrame(void)
{
	cmdBuf[0] = 0xFF;
	epd_writeCommand(0x22, cmdBuf, 1);
	epd_writeCommand(0x20, NULL, 0);  //Master Activation
	epd_waitBusy();
}


