/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "libmkc.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


void sleep(double t) {
    /** Sleep for a specified amount of seconds */
    clock_t start = clock();
    while(((double) (clock() - start)) / CLOCKS_PER_SEC < t)
}


LibMK_Controller* libmk_create_controller(
        LibMK_Device* identifier, LibMK_Model* model) {
    /** Build a new LibMK Controller for a Device */
    LibMK_Handle* device;
    int r = libmk_create_handle(&device, identifier);
    if (r != LIBMK_SUCCESS)
        return NULL;
    LibMK_Controller* controller = libmk_alloc_controller();
    controller->device = device;
    controller->pid = 0;
    return controller;
}


LibMK_Controller* libmk_alloc_controller() {
    /** Allocate a LibMK_Controller struct */
    LibMK_Controller* controller = (LibMK_Controller*) malloc(
        sizeof(LibMK_Controller));
    return controller;
}


LibMK_Result libmk_start_controller(LibMK_Controller* controller) {
    /** Start a LibMK_Controller in a new thread */
    LibMK_Result r = libmk_enable_control(controller->device);
    if (r != LIBMK_SUCCESS)
        return r;
    pthread_create(&controller->thread, NULL, libmk_run_controller, controller);
    return LIBMK_SUCCESS;
}


void libmk_run_controller(LibMK_Controller* controller) {
    /** Execute instructions on a LibMK_Controller in a separate thread */
    bool exit_flag;
    while (true) {
        pthread_mutex_lock(controller->exit_flag_lock);
        exit_flag = controller->exit_flag;
        pthread_mutex_unlock(controller->exit_flag_lock);
        if (exit_flag)
            break;
    }
}


LibMK_Result libmk_exec_instruction(LibMK_Handle* h, LibMK_Instruction* i) {
    /** Execute a single instruction on the keyboard controlled */
    
}


void libmk_stop_controller(LibMK_Controller* controller) {
    /** Stop a LibMK_Controller running in a separate thread */
    pthread_mutex_lock(controller->exit_flag_lock);
    controller->exit_flag = true;
    pthread_mutex_unlock(controller->exit_flag_lock);
}


LibMK_Controller_State libmk_join_controller(
        LibMK_Controller* controller, double timeout) {
    /** Join a LibMK_Controller into the current thread (return state) */
    bool active;
    clock_t t = clock();
    double elapsed;
    while (true) {
        pthread_mutex_lock(controller->active_lock);
        active = controller->active;
        pthread_mutex_unlock(controller->active_lock);
        if (!active)
            break;
        elapsed = ((double) (clock() - t)) / CLOCKS_PER_SEC;
        if (elapsed > timeout)
            return LIBMK_STATE_JOIN_ERR;
    }
    return controller->exit_state;
}
