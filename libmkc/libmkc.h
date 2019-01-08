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
    /** Enum describing LibMK_Controller state */
    LIBMK_STATE_ACTIVE = 0,
    LIBMK_STATE_STOPPED = 1,
    LIBMK_STATE_PRESTART = 2,
    LIBMK_STATE_ERROR = 3,
    LIBMK_STATE_JOIN_ERR = 4,
    LIBMK_STATE_START_ERR = 5,
} LibMK_Controller_State;


typedef struct LibMK_Instruction {
    /** Single instruction that can be executed by a controller
     *
     * An instruction should not be executed multiple times. An
     * instruction is bound to a specific controller as its id attribute
     * is bound to the linked list of instructions of a controller.
    */
    unsigned char* colors;
    unsigned char color[3];
    unsigned int duration;
    unsigned int id;
    struct LibMK_Instruction* next;
} LibMK_Instruction;


typedef struct LibMK_Controller {
    /** Controller that contains information to async control a keyboard
     *
     * Thread-safe state access is available through pthread mutexes.
    */
    LibMK_Handle* handle;
    LibMK_Instruction* instr; // Linked list of instructions
    pthread_mutex_t instr_lock;
    pthread_t thread;
    pthread_mutex_t exit_flag_lock;
    bool exit_flag;
    pthread_mutex_t state_lock;
    LibMK_Controller_State state;
    pthread_mutex_t error_lock;
    LibMK_Result error;
} LibMK_Controller;


LibMK_Controller* libmk_create_controller(LibMK_Handle* handle);
LibMK_Result libmk_free_controller(LibMK_Controller* c);

unsigned int libmk_sched_instruction(
    LibMK_Controller* controller, LibMK_Instruction* instruction);
LibMK_Result libmk_cancel_instruction(LibMK_Controller* c, unsigned int id);

LibMK_Result libmk_start_controller(LibMK_Controller* controller);
void libmk_run_controller(LibMK_Controller* controller);
void libmk_stop_controller(LibMK_Controller* controller);
LibMK_Controller_State libmk_join_controller(LibMK_Controller* c, double t);
void libmk_set_controller_error(LibMK_Controller* c, LibMK_Result r);

LibMK_Instruction* libmk_create_instruction();
LibMK_Instruction* libmk_create_instruction_full(unsigned char c[3]);
LibMK_Instruction* libmk_create_instruction_all(
    unsigned char c[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3]);
void libmk_free_instruction(LibMK_Instruction* i);


int libmk_exec_instruction(LibMK_Handle* h, LibMK_Instruction* i);
