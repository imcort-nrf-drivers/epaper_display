#include "epd_1in54.h"


static uint8_t cmdBuf[4];
static uint8_t epd_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

static const uint8_t set[] = {1, 2, 4, 8, 16, 32, 64, 128},
                     clr[] = {(uint8_t)~1,  (uint8_t)~2,  (uint8_t)~4,
                              (uint8_t)~8,  (uint8_t)~16, (uint8_t)~32,
                              (uint8_t)~64, (uint8_t)~128};

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
    
	spi_transfer(&cmd, 1, &cmd, 0);
    
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
    pinMode(EINK_BUSY, INPUT_PULLUP);
    
    digitalWrite(EINK_CS, HIGH);
    
    spi_init();
    
    digitalWrite(EINK_RESET, LOW);
    delay(10);
    digitalWrite(EINK_RESET, HIGH);
    delay(10);
    
	epd_waitBusy();
	epd_writeCommand(0x12, NULL, 0);  //SWRESET
    delay(10);
	epd_waitBusy();
    
    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
	epd_writeCommand(0x01, cmdBuf, 3); //Driver output control

    cmdBuf[0] = 0x01;
	epd_writeCommand(0x11, cmdBuf, 1); //data entry mode
    
    cmdBuf[0] = 0x00;
    cmdBuf[1] = 0x18;
	epd_writeCommand(0x44, cmdBuf, 2); //set Ram-X address start/end position 200  

    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x00;
	epd_writeCommand(0x45, cmdBuf, 4); //set Ram-Y address start/end position 200
    
    cmdBuf[0] = 0x05;
	epd_writeCommand(0x3C, cmdBuf, 1); //Border Wavefrom
    
    cmdBuf[0] = 0x80;
	epd_writeCommand(0x18, cmdBuf, 1); //Internal temperature
    
    cmdBuf[0] = 0xf8;
	epd_writeCommand(0x22, cmdBuf, 1); //Load LUT
    
    epd_writeCommand(0x20, NULL, 0);  //Master Activation
    
    epd_waitBusy();
    
    cmdBuf[0] = 0x00;
	epd_writeCommand(0x4E, cmdBuf, 1); //set RAM x address count to 0;
    
    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
	epd_writeCommand(0x4F, cmdBuf, 2); //set RAM y address count to 0X199;
    
    epd_waitBusy();
	
}

void epd_setMemoryArea(int x_start, int y_start, int x_end, int y_end)
{
    
    cmdBuf[0] = x_start & 0xFF;
    cmdBuf[1] = x_end & 0xFF;
	epd_writeCommand(0x44, cmdBuf, 2);
    
    cmdBuf[0] = y_start & 0xFF;
    cmdBuf[1] = (y_start >> 8) & 0xFF;
    cmdBuf[2] = y_end & 0xFF;
    cmdBuf[3] = (y_end >> 8) & 0xFF;
    epd_writeCommand(0x45, cmdBuf, 4);
    
}

void epd_setMemoryPointer(int x, int y)
{
    
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    cmdBuf[0] = (x >> 3) & 0xFF;
	epd_writeCommand(0x4E, cmdBuf, 1);
    
    cmdBuf[0] = y & 0xFF;
    cmdBuf[1] = (y >> 8) & 0xFF;
	epd_writeCommand(0x4F, cmdBuf, 2);
    
    //epd_waitBusy();
}

void epd_clearScreen(void)
{
    
    cmdBuf[0] = 0x24;
    
    digitalWrite(EINK_DC, LOW);
    digitalWrite(EINK_CS, LOW);
    
    spi_transfer(cmdBuf, 1, NULL, 0);
    
    digitalWrite(EINK_DC, HIGH);
    
    cmdBuf[0] = 0xFF;
    
    for(int32_t i = 0; i < 5000; i++)
    {
        spi_transfer(cmdBuf, 1, NULL, 0);
    }
    
    digitalWrite(EINK_CS, HIGH);
    
    epd_displayFrame();

}

void epd_sendFrame(int x, int y, int w, int h, uint8_t* frame_buffer)
{
    epd_setMemoryArea(x / 8, y - 1, x / 8 + w / 8 - 1, y + h - 1);
    epd_setMemoryPointer(x, y);
    
//    cmdBuf[0] = 0x24;
//    
//    digitalWrite(EINK_DC, LOW);
//    digitalWrite(EINK_CS, LOW);
//    
//    spi_transfer(cmdBuf, 1, NULL, 0);
//    
//    digitalWrite(EINK_DC, HIGH);
//    
//    for(int32_t i = 0; i < (w * h / 8); i++)
//    {
//        spi_transfer(frame_buffer + i, 1, NULL, 0);
//    }
//    
//    digitalWrite(EINK_CS, HIGH);
    
    epd_writeCommand(0x24, frame_buffer, w * h / 8);
}

void epd_update_u8(uint8_t y, uint8_t* line_buffer)
{

    for(int i = 0; i < SCREEN_WIDTH; i++){
		
		if(!line_buffer[i]){
			
			epd_framebuffer[(25 * y) + (i / 8)] |= set[i & 7];
		
		} else {
		
			epd_framebuffer[(25 * y) + (i / 8)] &= clr[i & 7];
		
		}
	
	}
    
    
    
}

void epd_update_framebuffer(void)
{
    
    cmdBuf[0] = 0x00;
    cmdBuf[1] = 0x18;
	epd_writeCommand(0x44, cmdBuf, 2); //set Ram-X address start/end position 200  

    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
    cmdBuf[2] = 0x00;
    cmdBuf[3] = 0x00;
	epd_writeCommand(0x45, cmdBuf, 4); //set Ram-Y address start/end position 200
    
    cmdBuf[0] = 0x00;
	epd_writeCommand(0x4E, cmdBuf, 1); //set RAM x address count to 0;
    
    cmdBuf[0] = 0xC7;
    cmdBuf[1] = 0x00;
	epd_writeCommand(0x4F, cmdBuf, 2); //set RAM y address count to 0X199;
    
    epd_waitBusy();
    
    cmdBuf[0] = 0x24;
    
    digitalWrite(EINK_DC, LOW);
    digitalWrite(EINK_CS, LOW);
    
    spi_transfer(cmdBuf, 1, NULL, 0);
    
    digitalWrite(EINK_DC, HIGH);
    
    for(int32_t i = 0; i < 5000; i++)
    {
        spi_transfer(epd_framebuffer + i, 1, NULL, 0);
    }
    
    digitalWrite(EINK_CS, HIGH);
    epd_displayFrame();
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


