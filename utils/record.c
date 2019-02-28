/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include "../libmk/libmk.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


void write_file(unsigned char layout[LIBMK_MAX_ROWS][LIBMK_MAX_COLS],
                int bVendor, int bDevice) {
    /** Write a layout matrix with device details to a file
     *
     * The layout is stored in a file so that it can be submitted into the code
     * and then more devices can be supported by the library. The vendor and
     * device id are included in hex format, followed by the layout in the same
     * format as it is included in libmk.c for each layout.
     */
    int r, c;
    FILE* file = fopen("layout.c", "w");
    fprintf(file, "{\n");
    fprintf(file, "Vendor: 0x%04x, Device: 0x%04x\n", bVendor, bDevice);
    for (r=0; r < LIBMK_MAX_ROWS; r++) {
        fprintf(file, "\t{");
        for (c=0; c < LIBMK_MAX_COLS; c++) {
            fprintf(file, "0x%02x", layout[r][c]);
            if (c != LIBMK_MAX_COLS - 1)
                fprintf(file, ", ");
        }
        fprintf(file, "},\n");
    }
    fprintf(file, "}\n");
    fclose(file);
}


int main(void) {
    /** Loop over all offsets for keys and match them to key coordinates
     *
     * Requires user input in the form of 'r,c' to read the key
     * coordinates. The key coordinates must be matched to the layout
     * of the specific keyboard.
     */
    libmk_init();
    LibMK_Model* models;
    int result;
    result = libmk_detect_devices(&models);
    if (result < 0)
        return -1;

    LibMK_Model model;
    if (result == 0) {
        printf("No devices detected.\n");
        return 0;
    } else if (result == 1) {
        printf("Detected model %d.\n", models[0]);
        model = models[0];
    } else {
        printf("Detected %d devices:\n", result);
        for (short i=0; i < result; i++)
            printf("Model: %d\n", models[i]);
        printf("Please select the model to analyze: ");
        int r = scanf("%d", &model);
        if (r != 1)
            return -3;
    }

    LibMK_Handle* handle;
    result = libmk_set_device(model, &handle);
    if (result < 0)
        return -2;

    unsigned char* packet = libmk_build_packet(
        8, 0xC0, 0x01, 0x01, 0x00, 0x00, 0xFF, 0x00, 0x00);

    unsigned char layout[LIBMK_MAX_ROWS][LIBMK_MAX_COLS];
    int r, c;
    for (r=0; r < LIBMK_MAX_ROWS; r++)
        for (c=0; c < LIBMK_MAX_COLS; c++)
            layout[r][c] = 0x00;
    libmk_enable_control(handle);
    printf("Now, one by one, the keys will be turned on in the color red.\n"
           "Please enter the coordinates of the key that was turned on in \n"
           "the format `row,col\\n`. Use `-1,-1` to indicate that no led \n"
           "was turned on. Use `-2,-2` to indicate that all leds have \n"
           "passed.\n\n");
    
    for (unsigned char offset=0x00; offset < 0xFF; offset++) {
        packet[4] = offset;
        packet[5] = 0xFF;
        result = libmk_send_packet(handle, packet);
        if (result != LIBMK_SUCCESS)
            return -3;
        printf("Offset %d, color red: ", offset);
        int read = 0;
        do {
            read = scanf("%d,%d", &r, &c);
        } while (read != 2);
        if (r == -1 || c == -1)
            continue;
        else if (r == -2 || c == -2)
            break;
        layout[r][c] = offset;
        packet[5] = 0x00;
        result = libmk_send_packet(handle, packet);
        if (result != LIBMK_SUCCESS)
            return -3;
        write_file(layout, handle->bVendor, handle->bDevice);
    }
    libmk_disable_control(handle);
    libmk_free_handle(handle);
    printf("\nPlease share the created file along with your model on the \n"
           "GitHub repository to help support more devices.\n");
    return 0;
}
