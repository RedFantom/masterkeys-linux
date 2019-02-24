/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include "../libmk/libmk.h"
#include <stdio.h>
#include <unistd.h>
#include "libusb.h"


int main(void) {
    /** Run some tests on the masterkeys library */
    bool c = libmk_init();
    if (!c) {
        printf("Failed to initialize LibMK Library.\n");
        return -1;
    }
    LibMK_Model* models = NULL;
    int n = libmk_detect_devices(&models);
    printf("Detected %d devices.\n", n);
    for (int i = 0; i < n; i++) {
        printf("  Detected device: %d, %s\n", models[i], LIBMK_MODEL_STRINGS[i]);
        
        printf("  Enabling device... ");
        int r = libmk_set_device(models[i], NULL);
        if (r == LIBMK_SUCCESS)
            printf("Done.\n");
        else {
            printf("Failed: %d.\n", r);
            continue;
        }
        
        printf("  Acquiring control... ");
        r = libmk_enable_control(NULL);
        if (r != LIBMK_SUCCESS) {
            printf("Failed: %d.\n", r);
            libmk_reset(NULL);
            continue;
        } else
            printf("Done.\n");
        
        LibMK_Firmware* fw;
        printf("  Retrieving firmware info... ");
        r = libmk_get_firmware_version(NULL, &fw);
        if (r == LIBMK_SUCCESS) {
            printf("Done.\n");
            printf("    Keyboard firmware version: %s\n", fw->string);
            printf("    Keyboard firmware layout: %d\n", fw->layout);
        } else
            printf("Failed: %d.\n", r);
        
        printf("  Setting full color... ");
        r = libmk_set_full_color(NULL, 255, 255, 0);
        if (r != LIBMK_SUCCESS)
            printf("Failed: %d\n", r);
        else
            printf("Done.\n");
        sleep(1);
        
        printf("  Setting LED effect... ");
        r = libmk_set_effect(NULL, LIBMK_EFF_WAVE);
        if (r != LIBMK_SUCCESS)
            printf("Failed: %d.\n", r);
        else
            printf("Done.\n");
        sleep(2);

        int color = 200;
        unsigned char colors[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3] = {0};
        for (short row=0; row < LIBMK_MAX_ROWS; row++)
            for (short col=0; col < LIBMK_MAX_COLS; col++) {
                color += 8;
                colors[row][col][(color / 256 + 1) % 3] = 0xFF -(color % 256);
                colors[row][col][(color / 256) % 3] = 0xFF - (color % 128);
                colors[row][col][(color / 256 + 2) % 3] = 0xFF - (color % 64);
            }
        printf("  Setting LED pattern... ");
        r = libmk_set_all_led_color(NULL, (unsigned char*) colors);
        if (r != LIBMK_SUCCESS)
            printf("Failed: %d.\n", r);
        else
            printf("Done.\n");
        sleep(4);
        
        printf("  Setting single LED... ");
        r = libmk_set_single_led(NULL, 0, 0, 255, 255, 0);
        if (r == LIBMK_SUCCESS)
            printf("Done.\n");
        else
            printf("Failed: %d.\n", r);
        sleep(2);
        
        printf("  Retrieving active profile. ");
        char profile;
        r = libmk_get_active_profile(NULL, &profile);
        if (r == LIBMK_SUCCESS)
            printf("Done.\n    Profile: %d.\n", profile);
        else
            printf("Failed: %d.\n", r);
        
        printf("  Disabling LED control... ");
        r = libmk_disable_control(NULL);
        if (r != LIBMK_SUCCESS)
            printf("Failed: %d.\n", r);
        else
            printf("Done.\n");
        libmk_reset(NULL);

        libmk_exit();
    }
    return 0;
}
