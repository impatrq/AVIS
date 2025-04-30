/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
 
void hid_task(void);
 
struct report{

    // uint16_t buttons;
    uint8_t z;
    // uint8_t joy1;
    // uint8_t joy2;
    uint8_t rz;
} report;
 
int main(void)
{

    board_init();

    tusb_init();

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(28);
    adc_gpio_init(27);//Trimmer
    while (1)
    {
        hid_task();
        tud_task(); // tinyusb device task
    }

    return 0;
}

void con_panic(uint16_t errcode)
{
    while (1)
    {
        tud_task(); // tinyusb device task
        // Remote wakeup
        if (tud_suspended())
        {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }

        if (tud_hid_ready())
        {
            tud_hid_n_report(0x00, 0x01, &report, sizeof(report));
        }
    }
}

void hid_task(void)
{
 
    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time8+
    start_ms += interval_ms;

    adc_select_input(0);
    uint32_t foo = 0;
    for(uint8_t i = 0; i < 100; i++) { foo += adc_read(); }
    report.z = ((uint16_t)(foo / 100)) >> 4;

    adc_select_input(2);       
    foo = 0;
    for(uint8_t i = 0; i < 64; i++) { foo += adc_read(); }
    uint16_t adc_rz = ((uint16_t)(foo / 64)) >> 4;

    adc_select_input(1);
    foo = 0;
    for (uint8_t i = 0; i < 64; i++) { foo += adc_read(); }
    uint16_t valortrimZ = ((uint16_t)(foo / 64)) >> 4;
    printf("La lectura del trimmer eje z es:%u\n",valortrimZ);

    int16_t rz_con_offset = (int16_t)adc_rz - (int16_t)valortrimZ;
    if (rz_con_offset < 0) rz_con_offset = 0; // Si quieres evitar valores negativos
    report.rz = (uint16_t)rz_con_offset;


    // Remote wakeup
    if (tud_suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    if (tud_hid_ready())
    {
        tud_hid_n_report(0x00, 0x01, &report, sizeof(report));
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT && buffer[0] == 2 && bufsize >= sizeof(light_data)) //light data
    // {
    //     size_t i = 0;
    //     for (i; i < sizeof(light_data); i++)
    //     {
    //         light_data.raw[i] = buffer[i + 1];
    //     }
    // }

    // echo back anything we received from host
    // tud_hid_report(0, buffer, bufsize);
}
