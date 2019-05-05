/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
 *
 * The HSV to RGB conversion algorithm was copied from:
 *    https://stackoverflow.com/questions/3018313
 *    The answer by Leszek Szary
*/
#include "../libmk/libmkc.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "libusb.h"


typedef struct RgbColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor {
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;


RgbColor* HsvToRgb(HsvColor* hsv) {
    RgbColor* rgb = malloc(sizeof(RgbColor));
    unsigned char region, remainder, p, q, t;
    
    if (hsv->s == 0) {
        rgb->r = hsv->v;
        rgb->g = hsv->v;
        rgb->b = hsv->v;
        return rgb;
    }
    
    region = hsv->h / 43;
    remainder = (hsv->h - (region * 43)) * 6;
    
    p = (hsv->v * (255 - hsv->s)) >> 8;
    q = (hsv->v * (255 - ((hsv->s * remainder) >> 8))) >> 8;
    t = (hsv->v * (255 - ((hsv->s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
        case 0:
            rgb->r = hsv->v; rgb->g = t; rgb->b = p;
            break;
        case 1:
            rgb->r = q; rgb->g = hsv->v; rgb->b = p;
            break;
        case 2:
            rgb->r = p; rgb->g = hsv->v; rgb->b = t;
            break;
        case 3:
            rgb->r = p; rgb->g = q; rgb->b = hsv->v;
            break;
        case 4:
            rgb->r = t; rgb->g = p; rgb->b = hsv->v;
            break;
        default:
            rgb->r = hsv->v; rgb->g = p; rgb->b = q;
            break;
    }
    return rgb;
}



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
        
        fprintf(stdout, "  Scheduling instructions... ");
        
        unsigned char red[3] = {255, 0, 0};
        LibMK_Instruction* full = libmk_create_instruction_flash(red, 10000, 255);
        unsigned int n = libmk_sched_instruction(ctrl, full);
        
        unsigned char yellow[3] = {255, 255, 0};
        full = libmk_create_instruction_full(yellow);
        full->duration = 1000000;
        libmk_sched_instruction(ctrl, full);
        
        unsigned char blank[3] = {0};
        LibMK_Instruction* wave = libmk_create_instruction_full(blank);
        LibMK_Instruction* a, * b;
        a = wave;
        HsvColor color;
        color.s = 255;
        color.v = 255;
        
        unsigned char map[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3];
        for (int i=0; i<1000; i++) {
            for (int r=0; r<LIBMK_MAX_ROWS; r++) {
                for (int c=0; c<LIBMK_MAX_COLS; c++) {
                    color.h = (i+c) * (255 / (LIBMK_MAX_COLS+40));
                    RgbColor* rgb = HsvToRgb(&color);
                    map[r][c][0] = rgb->r;
                    map[r][c][1] = rgb->g;
                    map[r][c][2] = rgb->b;
                    free(rgb);
                }
            }
            b = libmk_create_instruction_all(map);
            b->duration = 10000;
            a->next = b;
            a = b;
        }
        libmk_sched_instruction(ctrl, wave);
        
        
        printf("Done: %d.\n", full->id);
        
        fprintf(stdout, "  Awaiting controller... ");
        libmk_wait_controller(ctrl);
        printf("Done.\n");
        
        printf("\n  Now the controller will automatically exit after all instructions are done.\n");
        printf("  You can let your program do other stuff while the instructions execute.\n\n");
        
        LibMK_Controller_State s = libmk_join_controller(ctrl, 40);
        if (s != LIBMK_STATE_STOPPED)
            printf("  Could not stop the Controller: %d\n", s);
        libmk_free_handle(handle);
        printf("  Controller test ended.\n");
    }
}