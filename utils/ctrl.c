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
        
        fprintf(stdout, "  Starting controlller... ");
        libmk_start_controller(ctrl);
        printf("Done.\n");
        
        fprintf(stdout, "  Scheduling instruction... ");
        unsigned char color[3] = {100, 0, 0};
        LibMK_Instruction* full = libmk_create_instruction_full(color);
        unsigned int n = libmk_sched_instruction(ctrl, full);
        printf("Done: %d.\n", full->id);
        
        fprintf(stdout, "  Waiting for instruction execution...");
        sleep(1);
        printf(" Done.\n");
        
        fprintf(stdout, "  Stopping controller... ");
        libmk_stop_controller(ctrl);
        printf("Done.\n");
        
        LibMK_Controller_State s = libmk_join_controller(ctrl, 3);
        if (s != LIBMK_STATE_STOPPED)
            printf("  Could not stop the Controller: %d\n", s);
        libmk_free_handle(handle);
        printf("  Controller test ended.\n");
    }
}