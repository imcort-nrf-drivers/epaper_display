#include "lvgl_porting.h"

#include "epd_1in54.h"
#include "app_timer.h"

static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/

APP_TIMER_DEF(lvgl_timer);
#define LVGL_TICK_PERIOD 10

static void lvgl_timer_handler(void *p_context)
{
    lv_tick_inc(LVGL_TICK_PERIOD);
}

static void my_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint8_t* pointer = &(color_p->full);
    for (int y = area->y1; y <= area->y2; y++)
    {
        epd_update_u8(y, pointer);
        pointer += SCREEN_WIDTH;
    }
    epd_update_framebuffer();
    lv_disp_flush_ready(&disp_drv);
    
}

static void my_rounder_cb(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
  /* Update the areas as needed. Can be only larger.
   * For example to always have lines 8 px height:*/
   area->x1 = 0;
   area->x2 = SCREEN_WIDTH - 1;
}

static void my_printf(const char * buf)
{
	Debug("lvgl %s",buf);
    NRF_LOG_FLUSH();
}

void lvgl_begin(void){
    
    epd_begin();
    epd_clearScreen();
    
    delay(1000);
    
//    uint8_t pointer[200];
//    memset(pointer, 0x00, 200);
//    epd_update_u8(150, pointer);
//    
//    epd_update_framebuffer();
    
 
    lv_init();
    
    /*A static or global variable to store the buffers*/
    static lv_disp_draw_buf_t disp_buf;

    /*Static or global buffer(s). The second buffer is optional*/
    static lv_color_t buf_1[SCREEN_WIDTH * 2];

    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, SCREEN_WIDTH * 2);
    
    
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
    disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
    disp_drv.rounder_cb = my_rounder_cb;
    disp_drv.hor_res = SCREEN_WIDTH;      /*Set the horizontal resolution in pixels*/
    disp_drv.ver_res = SCREEN_HEIGHT;     /*Set the vertical resolution in pixels*/

    lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	lv_log_register_print_cb(my_printf); /* register print function for debugging */
    
    ret_code_t err_code;

    err_code = app_timer_create(&lvgl_timer, APP_TIMER_MODE_REPEATED, lvgl_timer_handler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_start(lvgl_timer, APP_TIMER_TICKS(LVGL_TICK_PERIOD), NULL);
    APP_ERROR_CHECK(err_code);

}
