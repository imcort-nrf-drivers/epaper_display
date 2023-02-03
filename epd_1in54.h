#ifndef __EPAPER_H
#define __EPAPER_H

#include "transfer_handler.h"

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 200

void epd_begin(void);
void epd_setMemoryArea(int x_start, int y_start, int x_end, int y_end);
void epd_setMemoryPointer(int x, int y);
void epd_sendFrame(int x, int y, int w, int h, uint8_t* frame_buffer);
void epd_displayFrame(void);
void epd_displayPartFrame(void);
void epd_lowPower(void);
void epd_clearScreen(void);
void epd_update_u8(uint8_t y, uint8_t* line_buffer);
void epd_update_framebuffer(void);

#endif


