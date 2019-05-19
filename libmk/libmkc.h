/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "libmk.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


/// @brief Controller States
typedef enum LibMK_Controller_State {
    LIBMK_STATE_ACTIVE = 0, ///< Controller is active
    LIBMK_STATE_STOPPED = 1, ///< Controller was active, but is now stopped
    LIBMK_STATE_PRESTART = 2, ///< Controller has not yet been active
    LIBMK_STATE_ERROR = 3, ///< Controller was stopped by an error
    LIBMK_STATE_JOIN_ERR = 4, ///< Failed to join the controller
    LIBMK_STATE_START_ERR = 5, ///< Failed to start the controller
} LibMK_Controller_State;

/// @brief Instruction types
typedef enum LibMK_Instruction_Type {
    LIBMK_INSTR_FULL = 0, ///< Full keyboard color instruction
    LIBMK_INSTR_ALL = 1, ///< All LEDs individually instruction
    LIBMK_INSTR_SINGLE = 2, ///< Instruction for a single key
} LibMK_Instruction_Type;

/** @brief Single instruction that can be executed by a controller
 *
 * An instruction should not be executed multiple times. An instruction
 * is bound to a specific controller as its id attribute is bound to the
 * linked list of instructions of a controller.
 */
typedef struct LibMK_Instruction {
    unsigned char r, c; ///< LIBMK_INSTR_SINGLE, row and column coords
    unsigned char* colors; ///< LIBMK_INSTR_ALL, key color matrix
    unsigned char color[3]; ///< LIBMK_INSTR_SINGLE, LIBMK_INSTR_FULL
    unsigned int duration; ///< Delay after execution of instruction
    unsigned int id; ///< ID number set by the scheduler
    struct LibMK_Instruction* next; ///< Linked list attribute
    LibMK_Instruction_Type type; ///< For the instruction execution
} LibMK_Instruction;

/** @brief Controller for a keyboard managing a single handle
 *
 * Access to the various attributes of the Controller is
 * protected by mutexes and the attributes of the controller should
 * therefore not be accessed directly.
 */
typedef struct LibMK_Controller {
    LibMK_Handle* handle; ///< Handle of the keyboard to control
    LibMK_Instruction* instr; ///< Linked list of instructions
    pthread_mutex_t instr_lock; ///< Protects LibMK_Instruction* instr
    pthread_t thread; ///< Thread for libmk_run_controller
    pthread_mutex_t exit_flag_lock; ///< Protects bool exit_flag and wait_flag
    bool exit_flag; ///< Exit event: Thread exits immediately
    bool wait_flag; ///< Wait event: Thread exits when all instructions are done
    pthread_mutex_t state_lock; ///< Protects LibMK_Controller_State state
    LibMK_Controller_State state; ///< Stores current state of controller
    pthread_mutex_t error_lock; ///< Protects LibMK_Result error
    LibMK_Result error; ///< Set for LIBMK_STATE_ERROR
} LibMK_Controller;

/** @brief Create a new LibMK_Controller for a defined handle
 *
 * After initialization of the Controller, the Handle may no longer be
 * used directly. The lifecycle of the created controller is the
 * responsibility of the user.
 */
LibMK_Controller* libmk_create_controller(LibMK_Handle* handle);

/** @brief Free a LibMK_Controller
 *
 * First performs a check to see if the controller is still active. An
 * active controller may not be freed. Returns LIBMK_ERR_STILL_ACTIVE
 * if the controller is still active.
 */
LibMK_Result libmk_free_controller(LibMK_Controller* c);

/** @brief Schedule a linked-list of instructions
 *
 * Instruction scheduler than schedules the given linked-list of
 * instructions at the end of the list of the controller in the given
 * order. After scheduling, all instructions are given an ID number
 * and they may not be scheduled again. After execution, the
 * instructions are freed and thus after scheduling an instruction may
 * not be accessed again.
 *
 * Returns the instruction ID of the first instruction in the linked
 * list (it is the user's responsibility to derive the ID number of the
 * other instructions) upon success (postive integer) or a LibMK_Result
 * (negative integer) upon failure.
 */
int libmk_sched_instruction(
    LibMK_Controller* controller, LibMK_Instruction* instruction);

/** @brief Cancel a scheduled instruction by its ID number
 *
 * If the instruction has already been executed, the instruction is not
 * cancelled and the function fails quietly. Does not cancel any
 * successive instructions even if the instruction was scheduled as
 * part of a linked-list.
 */
LibMK_Result libmk_cancel_instruction(LibMK_Controller* c, unsigned int id);

/** @brief Start a new Controller thread
 *
 * Start the execution of instructions upon the keyboard in a different
 * thread. This function enables control of the keyboard and initializes
 * the thread.
 */
LibMK_Result libmk_start_controller(LibMK_Controller* controller);

/** @brief Internal Function. Execute Controller instructions. */
void libmk_run_controller(LibMK_Controller* controller);

/** @brief Request an exit on a running controller
 *
 * The request is passed using the exit_flag, and thus the Controller
 * may not immediately be stopped. To assure that the controller has
 * stopped, use libmk_join_controller.
 */
void libmk_stop_controller(LibMK_Controller* controller);

/** @brief Indicate the controller to finish only pending instructions
 *
 * If instructions are scheduled in the mean-time, they are added to the
 * linked list and still executed by the Controller before exiting. Only
 * after the linked list has become empty does the Controller exit.
 *
 * This function sets the wait_flag, and thus the Controller has not
 * necessarily stopped after this function ends. To assure that the
 * Controller has stopped, use libmk_join_controller.
 */
void libmk_wait_controller(LibMK_Controller* controller);

/** @brief Join the Controller thread
 *
 * @param t: Timeout in seconds
 * @returns LIBMK_STATE_JOIN_ERR upon timeout, controller state after
 *    exiting upon success.
 */
LibMK_Controller_State libmk_join_controller(LibMK_Controller* c, double t);

/** @brief Internal Function. */
void libmk_set_controller_error(LibMK_Controller* c, LibMK_Result r);

/** @brief Allocate a new LibMK_Instruction struct */
LibMK_Instruction* libmk_create_instruction();

/** @brief Create a new instruction to set the full keyboard color
 *
 * @param c: RGB color triplet that is copied to the instruction.
 * @returns Pointer to single LibMK_Instruction. Duration may be set
 *    by the caller.
 */
LibMK_Instruction* libmk_create_instruction_full(unsigned char c[3]);

/** @brief Create a new instruction to set all leds individually
 *
 * @param c: RGB color matrix that is copied to the instruction.
 * @returns Pointer to single LibMK_Instruction. Duration may be set
 *    by the caller.
 */
LibMK_Instruction* libmk_create_instruction_all(
    unsigned char c[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3]);

/** @brief Create a new list of instructions to flash the keyboard
 *
 * Makes use of LIBMK_INSTR_FULL type instructions. Builds a linked list
 * of n instructions.
 *
 * @param c: RGB color triplet that is copied to the instruction.
 * @param delay: Duration set for each individual instruction of the
 *    linked list in microseconds, excluding the time required for
 *    instruction execution itself.
 * @param n: Number of instructions in the linked list to be built.
 *
 * @returns Pointer to linked list of LibMK_Instruction.
 */
LibMK_Instruction* libmk_create_instruction_flash(
    unsigned char c[3], unsigned int delay, unsigned char n);


/** @brief Create a new instruction to set the color of a single key
 *
 * Overridden by a LIBMK_INSTR_FULL, just as in synchronous keyboard
 * control. When changing six or more keys, using a LIBMK_INSTR_ALL is
 * faster.
 *
 * @param row: Row coordinate of the key
 * @param column: Column coordinate of the key
 * @param c: RGB triplet to be copied to the instruction
 *
 * @returns Single LibMK_Instruction, duration may be set by the user.
 */
LibMK_Instruction* libmk_create_instruction_single(
    unsigned char row, unsigned char column, unsigned char c[3]);

/** @brief Free a single LibMK_Instruction
 *
 * The instruction is expected to longer be part of a linked list. This
 * instruction is automatically called after an instruction has been
 * executed and should only be called by the user if an instruction must
 * be freed before it is scheduled. Use libmk_cancel_instruction to
 * cancel scheduled instructions, which are then freed after cancelling.
 */
void libmk_free_instruction(LibMK_Instruction* i);

/** @brief Internal Function. Execute a single instruction. NOT THREAD-SAFE. */
LibMK_Result libmk_exec_instruction(LibMK_Handle* h, LibMK_Instruction* i);
