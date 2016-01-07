/*
 * Copyright (C) 2016 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INIT_BOARD_DEVICE_H__
#define __INIT_BOARD_DEVICE_H__

#include "init_board_common.h"
#include "init_prototypes.h"

// Constants: devices controls
#define DEV_BLOCK_FOTA_NUM 27

// Class: init_board_device
class init_board_device : public init_board_common
{
public:
    // Board: introduction for keycheck
    virtual void introduce_keycheck()
    {
        // Short vibration
        vibrate(100);

        // LED boot selection colors
        led_color(255, 0, 255);

        // Keycheck timeout
        msleep(3000);
    }

    // Board: introduction for Recovery
    virtual void introduce_recovery()
    {
        // Short vibration
        vibrate(50);

        // Align buffer parameter
        write_int("/sys/module/msm_fb/parameters/align_buffer", 0);
    }

    // Board: set led colors
    virtual void led_color(uint8_t r, uint8_t g, uint8_t b)
    {
        write_int("/sys/class/leds/led:rgb_red/brightness", r);
        write_int("/sys/class/leds/led:rgb_green/brightness", g);
        write_int("/sys/class/leds/led:rgb_blue/brightness", b);
    }

    // Board: set hardware vibrator
    virtual void vibrate(unsigned int strength)
    {
        write_int("/sys/class/timed_output/vibrator/enable", strength);
    }
};

#endif //__INIT_BOARD_DEVICE_H__
