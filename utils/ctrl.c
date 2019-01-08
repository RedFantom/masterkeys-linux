/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "../libmkc/libmkc.h"
#include <stdio.h>
#include <unistd.h>
#include "libusb.h"


int main(void) {
    libmk_init();
    printf("Detecting devices...\n");
    LibMK_Model* models = NULL;
    int n = libmk_detect_devices(&models);
    printf("%d devices detected.\n", n);
    for (int i=0; i < n; i++) {
        printf("  Detected: %d\n", models[i]);
        LibMK_Handle* handle;
        int r = libmk_set_device(models[i], &handle);
        if (r != LIBMK_SUCCESS) {
            printf("  Could not open this device.\n");
            continue;
        }
    
        LibMK_Controller* ctrl = libmk_create_controller(handle);
        if (ctrl == NULL) {
            printf("  Could not create controller for this device.\n");
            continue;
        }
        
        libmk_start_controller(ctrl);
        unsigned char color[3] = {100, 0, 0};
        LibMK_Instruction* full = libmk_create_instruction_full(color);
        unsigned int n = libmk_sched_instruction(ctrl, full);
        printf("  Scheduled instruction %d\n", n);
        sleep(4);
        libmk_stop_controller(ctrl);
        LibMK_Controller_State s = libmk_join_controller(ctrl, 3);
        if (s != LIBMK_STATE_STOPPED)
            printf("  Could not stop the Controller: %d\n", s);
        libmk_free_handle(handle);
        printf("  Controller test ended.\n");
    }
}