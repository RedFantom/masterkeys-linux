/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "../libmk/libmk.h"
#include <stdio.h>
#include <unistd.h>
#include "libusb.h"


int main(void) {
    /** Run some tests on the masterkeys library */
    libmk_init();
    printf("Device: %d\n", libmk_ident_model("MasterKeys Pro S White"));
    LibMK_Model* models = NULL;
    int n = libmk_detect_devices(&models);
    printf("Detected %d devices.\n", n);
    for (int i = 0; i < n; i++) {
        printf("  Detected: %d\n", models[i]);
        int r = libmk_set_device(models[i], NULL);
        if (r != LIBMK_SUCCESS)
            printf("  Enabling this device failed: %d.\n", r);
        else
            printf("  Enabled this device.\n");
        r = libmk_enable_control(NULL);
        if (r != LIBMK_SUCCESS) {
            printf("  Enabling LED control failed: %d.\n", r);
            libmk_reset(NULL);
            continue;
        } else
            printf("  Enabled LED control.\n");
        r = libmk_set_full_color(NULL, 255, 255, 0);
        if (r != LIBMK_SUCCESS) {
            printf("    Setting full color failed.\n");
        } else {
            printf("    Setting full color done.\n");
        }
        sleep(1);
        r = libmk_set_effect(NULL, LIBMK_EFF_WAVE);
        if (r != LIBMK_SUCCESS)
            printf("  Failed to set LED effect: %d\n", r);
        else {
            printf("  LED Effect set.\n");
            sleep(2);
        }

        int color = 200;
        unsigned char colors[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3] = {0};
        for (short row=0; row < 1; row++)
            for (short col=0; col < LIBMK_MAX_COLS; col++) {
                color += 8;
                colors[row][col][(color / 256 + 1) % 3] = 0xFF -(color % 256);
                colors[row][col][(color / 256) % 3] = 0xFF - (color % 128);
                colors[row][col][(color / 256 + 2) % 3] = 0xFF - (color % 64);
            }
        libmk_set_all_led_color(NULL, (unsigned char*) colors);

        sleep(2);
        printf("Enabling single LED.\n");
        r = libmk_set_single_led(NULL, 0, 0, 255, 255, 0);
        sleep(2);

        r = libmk_disable_control(NULL);
        if (r != LIBMK_SUCCESS)
            printf("  Disabling LED control failed: %d.\n", r);
        else
            printf("  Disabled LED control.\n");
        libmk_reset(NULL);

        libmk_exit();
    }
    return 0;
}
