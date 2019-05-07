/** Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/X.h>


typedef struct CaptureArgs {
    unsigned char* target_color;
    pthread_mutex_t* target_lock;
    pthread_mutex_t* exit_lock;
    pthread_mutex_t* keyboard_lock;
    bool* exit_flag;
    Display* display;
    Window root;
    XWindowAttributes gwa;
    int divider;
    int saturation_bias;
    int upper_threshold;
    int lower_threshold;
    bool brightness_norm;
} CaptureArgs;


typedef struct Screenshot {
    unsigned char* data;
    unsigned int w, h;
} Screenshot;


unsigned char calc_diff(unsigned char one, unsigned char two) {
    /** Calculate the absolute difference between two values */
    int diff = (int) one - (int) two;
    return diff < 0 ? -diff : diff;
}


CaptureArgs* init_capture(int divider, int sat_bias, int lower, int upper,
                          bool brightness_norm, unsigned char* target_color,
                          pthread_mutex_t* target_lock, bool* exit_flag,
                          pthread_mutex_t* exit_lock, pthread_mutex_t* kb_lock) {
    /** Initialize a CaptureArgs struct that can be passed as thread argument */
    CaptureArgs* args = (CaptureArgs*) malloc(sizeof(CaptureArgs));
    
    args->divider = divider;
    args->saturation_bias = sat_bias;
    args->lower_threshold = lower;
    args->upper_threshold = upper;
    args->brightness_norm = brightness_norm;
    args->target_color = target_color;
    args->target_lock = target_lock;
    args->exit_flag = exit_flag;
    args->exit_lock = exit_lock;
    args->keyboard_lock = kb_lock;
    
    args->display = XOpenDisplay(NULL);
    args->root = DefaultRootWindow(args->display);
    if (XGetWindowAttributes(args->display, args->root, &(args->gwa)) < 0) {
        XCloseDisplay(args->display);
        free(args);
        return NULL;
    }
    
    return args;
}


void capture(Screenshot** screenshot, XWindowAttributes gwa,
             Display* display, Window root) {
    /** Capture screenshot and save it to Screenshot struct
     *
     * The data captured from X.org is in long int format (32-bit
     * integers), which have to be converted to three separate 8-bit
     * integers representing the pixels.
     */
    (*screenshot) = (Screenshot*) malloc(sizeof(Screenshot));
    int width = gwa.width, height = gwa.height;
    
    (*screenshot)->data = (unsigned char*) malloc(
        width*height*3*sizeof(unsigned char));
    (*screenshot)->w = width;
    (*screenshot)->h = height;
    
    XImage* img = XGetImage(
        display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    unsigned long masks[3] = {
        img->red_mask, img->green_mask, img->blue_mask};
    
    for (int x=0; x<width; x++)
        for (int y=0; y<height; y++) {
            unsigned long pix = XGetPixel(img, x, y);
            for (int i=0; i<3; i++)
                (*screenshot)->data[(x+width*y) * 3 + i] =
                    (unsigned char) (pix >> (2-i) * 8);
        }
        
    XDestroyImage(img);
}


void calc_dominant_color(unsigned char* data, int w, int h,
                         unsigned char* target, int divider, int sat_bias,
                         int lower, int upper, bool brightness_norm) {
    /** Calculate the dominant color in an array of pixels
     *
     *
     */
    unsigned char temp[3];
    unsigned long colors[3] = {0};
    unsigned long n_pixels = 0;
    
    divider = divider == 0 ? 1 : divider;
    int width = w / divider;
    
    /// Summing of pixels fitting criteria
    for (int x=0; x<width; x++) {
        for (int y=0; y<h; y++) {
            unsigned int sum = 0;
            int max_diff = 0;
            unsigned char* pixel = &(data[(x+y*w)*3]);
            for (int i=0; i<3; i++) {
                int first = i, second = i+1<3 ? i+1 : 0;
                unsigned char diff = calc_diff(pixel[first], pixel[second]);
                max_diff = diff > max_diff ? diff : max_diff;
                sum += pixel[i];
            }
            if (sum < lower || sum > upper || max_diff < sat_bias)
                continue;
            for (int i=0; i<3; i++)
                colors[i] += pixel[i];
            n_pixels += 1;
        }
    }
    
    /// Averaging
    unsigned char max = 0;
    for (int i=0; i<3; i++) {
        if (n_pixels == 0) {
            target[i] = 0xFF;  // Error condition
            continue;
        }
        target[i] = (unsigned char) (colors[i] / n_pixels);
        max = target[i] > max ? target[i] : max;
    }
    
    max = max == 0 ? 0xFF : max;
    
    /// Normalization
    if (brightness_norm && max != 0xFF)
        for (int i=0; i<3; i++)
            target[i] = (unsigned char) ((double) target[i] * (255.0 / (double) max));
        
}


void capturer(struct CaptureArgs* args) {
    /** Function designed to be run in a thread, captures screenshots
     *
     */
    unsigned char target[3], previous[3];
    
    while (true) {
        pthread_mutex_lock(args->exit_lock);
        bool exit = *(args->exit_flag);
        pthread_mutex_unlock(args->exit_lock);
        if (exit)
            break;
        
        Screenshot* screenshot;
        
        capture(&screenshot, args->gwa, args->display, args->root);
        
        calc_dominant_color(screenshot->data, screenshot->w, screenshot->h,
                            target, args->divider, args->saturation_bias,
                            args->lower_threshold, args->upper_threshold,
                            args->brightness_norm);
        
        free(screenshot->data);
        free(screenshot);
        
        pthread_mutex_lock(args->keyboard_lock);
        pthread_mutex_lock(args->target_lock);
        for (int i=0; i<3; i++) {
            args->target_color[i] = target[i];
            previous[i] = target[i];
        }
        pthread_mutex_unlock(args->target_lock);
        pthread_mutex_unlock(args->keyboard_lock);
    }
}
