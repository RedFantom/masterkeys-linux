/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include "libmk.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/X.h>


#define MAX_WIDTH -1  // 0: Full width, -1: / 2, n_pixels otherwise
#define SATURATION_BIAS 60
#define BRIGHTNESS_NORM
#define UPPER_TRESHOLD 700
#define LOWER_TRESHOLD 25


bool exit_requested = false;
unsigned char target_color[3] = {0};
pthread_mutex_t exit_req_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t target_color_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t keyboard_lock = PTHREAD_MUTEX_INITIALIZER;
Display* display;
Window root;
XWindowAttributes gwa;


typedef struct Screenshot {
    unsigned char* data;
    unsigned int width, height;
} Screenshot;


void interrupt_handler(int signal) {
    /** Handle a Control-C command to exit the loop */
    pthread_mutex_lock(&exit_req_lock);
    exit_requested = true;
    pthread_mutex_unlock(&exit_req_lock);
}


int capture_screenshot(Screenshot** screenshot) {
    /** Capture a screenshot and store it in an array */
    (*screenshot) = (Screenshot*) malloc(sizeof(Screenshot));
    int width = gwa.width, height = gwa.height;
    (*screenshot)->data = (unsigned char*) malloc(
        width * height * 3 * sizeof(unsigned char));
    
    XImage* img = XGetImage(
        display, root, 0, 0, width, height, AllPlanes, ZPixmap);

    unsigned long rm = img->red_mask, gm = img->green_mask, bm = img->blue_mask;
    unsigned long masks[3] = {rm, gm, bm};
    unsigned char pixel[3];

    for (int x=0; x < width; x++)
        for (int y = 0; y < height; y++) {
            unsigned long pix = XGetPixel(img, x, y);
            for (int i = 0; i  < 3; i++)
                (*screenshot)->data[(x + width * y) * 3 + i] =
                    (unsigned char) (pix >> (2 - i) * 8);
        }
    XDestroyImage(img);
    return 0;
}


void* calculate_keyboard_color(void *void_ptr) {
    /** Continuously capture screens and calculate the dominant colour
     *
     * Options are given in defines:
     * MAX_WIDTH: If 0, uses full screen width. If -1, uses only the
     *   first half of the pixel columns for double monitors. Otherwise,
     *   limited to value of MAX_WIDTH.
     * LOWER_THRESHOLD: Minimum summed value of the RGB triplet, used
     *   for filtering out dark pixels.
     * UPPER_THRESHOLD: Maximum summed value of the RGB triplet, used
     *   for filtering out bright pixels.
     * SATURATION_BIAS: Minimum required difference between any pair
     *   of bytes of the RGB triplet of a pixel.
     * BRIGHTNESS_NORM: If defined, target colors sent to the keyboard
     *   are scaled so that at least one of the RGB values of the
     *   triplet is the maximum of 255.
     */
    Screenshot* screen;

    while (true) {

        pthread_mutex_lock(&exit_req_lock);
        if (exit_requested) {
            printf("Exit requested.\n");
            pthread_mutex_unlock(&exit_req_lock);
            break;
        }
        pthread_mutex_unlock(&exit_req_lock);
        if (capture_screenshot(&screen) < 0) {
            int code = -2;
            pthread_exit(&code);
        }

        int w = gwa.width, h = gwa.height;

        unsigned char temp[3];
        unsigned long colors[3] = {0};
        unsigned long n_pixels = 0;
        unsigned int sum;

        int lim;
        if (MAX_WIDTH == 0) {
            lim = w;
        } else if (MAX_WIDTH == -1) {
            lim = w / 2;
        } else {
            lim = MAX_WIDTH;
        }

        // Sum
        for (int x = 0; x < lim; x++) {
            for (int y = 0; y < h; y++) {
                sum = 0;
                int max_diff = 0;
                for (int i = 0; i < 3; i++) {
                    int first = i, second = i + 1;
                    if (i == 2)
                        second = 0;
                    int diff = screen->data[(x + y * w) * 3 + first] -
                               screen->data[(x + y * w) * 3 + second];
                    if (diff > max_diff)
                        max_diff = diff;

                    temp[i] = screen->data[(x + y * w) * 3 + i];
                    sum += temp[i];
                }
                if (sum < LOWER_TRESHOLD ||
                    sum > UPPER_TRESHOLD ||
                    max_diff < SATURATION_BIAS)
                    continue;
                for (int i = 0; i < 3; i++)
                    colors[i] += temp[i];
                n_pixels += 1;
            }
        }
        unsigned char color[3];
        unsigned char max = 0;

        // Average
        for (int i = 0; i < 3; i++) {
            if (n_pixels == 0) {
                color[i] = 0xFF;
                continue;
            }
            color[i] = (unsigned char) (colors[i] / n_pixels);
            if (color[i] > max)
                max = color[i];
        }

#ifdef BRIGHTNESS_NORM
        // Normalize
        if (max != 0)
            for (int i = 0; i < 3; i++)
                color[i] = (unsigned char) ((int) color[i] * (255.0 / max));
#endif

        // Copy color over to thread-safe variable
        pthread_mutex_lock(&target_color_lock);
        for (int i=0; i < 3; i++)
            target_color[i] = color[i];
        pthread_mutex_unlock(&target_color_lock);

        // Clean up
        free(screen->data);
        free(screen);
    }
    pthread_exit(0);
}


void* update_keyboard_color(void* ptr) {
    unsigned char color[3] = {0}, prev[3] = {0};
    while (true) {
        pthread_mutex_lock(&exit_req_lock);
        if (exit_requested) {
            pthread_mutex_unlock(&exit_req_lock);
            break;
        }
        pthread_mutex_unlock(&exit_req_lock);

        int diff;
        bool equal = true;
        pthread_mutex_lock(&target_color_lock);
        for (int i=0; i < 3; i++) {
            diff = (int) target_color[i] - color[i];
            prev[i] = color[i];
            color[i] += (unsigned char) (diff / 20.0);
            equal = (prev[i] == target_color[i]) && equal;
        }
        pthread_mutex_unlock(&target_color_lock);
        
        if (equal)
            continue;
    
        pthread_mutex_lock(&keyboard_lock);
        int r = libmk_set_full_color(NULL, color[0], color[1], color[2]);
        if (r != LIBMK_SUCCESS)
            printf("LibMK Error: %d\n", r);
        pthread_mutex_unlock(&keyboard_lock);
        
        struct timespec time;
        time.tv_nsec = 100000000 / 4;
        nanosleep(&time, NULL);
    }
    pthread_exit(0);
}


int main() {
    /** Run a loop that grabs a screenshot and updated lighting */
    signal(SIGINT, interrupt_handler);
    libmk_init();
    
    // Set up libmk
    LibMK_Model* devices;
    int n = libmk_detect_devices(&devices);
    if (n < 0) {
        printf("libmk_detect_devices failed: %d\n", n);
        return n;
    }
    libmk_set_device(devices[0], NULL);
    libmk_enable_control(NULL);
    
    // Open the XDisplay
    display = XOpenDisplay(NULL);
    root = DefaultRootWindow(display);
    if (XGetWindowAttributes(display, root, &gwa) < 0)
        return -1;

    pthread_t keyboard, screenshot;

    // Run the loop
    pthread_create(&screenshot, NULL, calculate_keyboard_color, NULL);
    pthread_create(&keyboard, NULL, update_keyboard_color, NULL);

    pthread_join(screenshot, NULL);
    pthread_join(keyboard, NULL);
    
    // Perform closing actions
    libmk_disable_control(NULL);
    libmk_exit();
    return 0;
}
