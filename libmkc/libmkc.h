/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "../libmk/libmk.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


typedef enum LibMK_Controller_State {
    LIBMK_STATE_ACTIVE = 0,
    LIBMK_STATE_STOPPED = 1,
    LIBMK_STATE_PRESTART = 2,
    LIBMK_STATE_ERROR = 3,
    LIBMK_STATE_JOIN_ERRR = 4,
    LIBMK_STATE_START_ERR = 5,
    LIBMK_STATE_STARTED = 6,
} LibMK_Controller_State;


typedef struct LibMK_Instruction {
    unsigned char* colors;
    unsigned char color[3];
    unsigned int duration;
    unsigned int id;
} LibMK_Instruction;


typedef struct LibMK_Controller {
    LibMK_Handle* device;
    LibMK_Instruction* instruction; // Linked list of instructions
    pthread_t thread;
    pthread_mutex_t exit_flag_lock;
    bool exit_flag;
    pthread_mutex_t active_lock;
    bool active;
    LibMK_Controller_State exit_state;
} LibMK_Controller;


LibMK_Controller* libmk_create_controller();
void libmk_destroy_controller(LibMK_Controller* controller);
bool libmk_start_controller(LibMK_Controller* controller);
bool libmk_stop_controller(LibMK_Controller* controller);

unsigned int libmk_schedule_instruction(
    LibMK_Controller* controller, LibMK_Instruction* instruction);
bool libmk_cancel_instruction(unsigned int instruction);

LibMK_Controller* libmk_alloc_controller();
void libmk_free_controller();
