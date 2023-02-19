#ifndef __EPAPER_H
#define __EPAPER_H

#include "transfer_handler.h"

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 200

void epd_begin(void);
void epd_sendFrame(int x, int y, int w, int h, uint8_t* frame_buffer);
void epd_displayFrame(void);
void epd_displayPartFrame(void);
void epd_lowPower(void);
void epd_clearScreen(bool white);
void epd_update_u8(uint8_t y, uint8_t* line_buffer);
void epd_update_framebuffer(void);
void epd_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void nextFullrefresh(void);

#endif


