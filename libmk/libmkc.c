/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
*/
#include "libmkc.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define LIBMKC_DEBUG


LibMK_Controller* libmk_create_controller(LibMK_Handle* handle) {
    LibMK_Controller* controller = (LibMK_Controller*) malloc(
        sizeof(LibMK_Controller));
    if (controller == NULL)
        return NULL;
    controller->handle = handle;
    pthread_mutex_init(&controller->state_lock, NULL);
    pthread_mutex_init(&controller->exit_flag_lock, NULL);
    pthread_mutex_init(&controller->instr_lock, NULL);
    pthread_mutex_init(&controller->error_lock, NULL);
    controller->instr = NULL;
    controller->state = LIBMK_STATE_PRESTART;
    controller->exit_flag = false;
    controller->wait_flag = false;
    return controller;
}


LibMK_Controller_State libmk_get_controller_state(LibMK_Controller* c) {
    pthread_mutex_lock(&(c->state_lock));
    LibMK_Controller_State b = c->state;
    pthread_mutex_unlock(&c->state_lock);
    return b;
}


LibMK_Result libmk_get_controller_error(LibMK_Controller* c) {
    pthread_mutex_lock(&(c->error_lock));
    LibMK_Result r = c->error;
    pthread_mutex_unlock(&(c->error_lock));
    return r;
}


LibMK_Result libmk_free_controller(LibMK_Controller* c) {
    if (libmk_get_controller_state(c) == LIBMK_STATE_ACTIVE)
        return LIBMK_ERR_STILL_ACTIVE;
    pthread_mutex_destroy(&c->state_lock);
    pthread_mutex_destroy(&c->exit_flag_lock);
    pthread_mutex_destroy(&c->instr_lock);
    pthread_mutex_destroy(&c->error_lock);
    int r = libmk_free_handle(c->handle);
    if (r != LIBMK_SUCCESS)
        return (LibMK_Result) r;
    free(c);
    return LIBMK_SUCCESS;
    
}


LibMK_Result libmk_start_controller(LibMK_Controller* controller) {
    LibMK_Result r = (LibMK_Result) libmk_enable_control(controller->handle);
    if (r != LIBMK_SUCCESS)
        return r;
    pthread_create(
        &controller->thread, NULL,
        (void*) libmk_run_controller, (void*) controller);
    return LIBMK_SUCCESS;
}


void libmk_run_controller(LibMK_Controller* controller) {
    pthread_mutex_lock(&(controller->state_lock));
    controller->state = LIBMK_STATE_ACTIVE;
    pthread_mutex_unlock(&(controller->state_lock));
    bool exit_flag, wait_flag;
    while (true) {
        pthread_mutex_lock(&(controller->exit_flag_lock));
        exit_flag = controller->exit_flag;
        wait_flag = controller->wait_flag;
        pthread_mutex_unlock(&(controller->exit_flag_lock));
        if (exit_flag)
            break;
        pthread_mutex_lock(&(controller->instr_lock));
        if (controller->instr == NULL) {
            pthread_mutex_unlock(&(controller->instr_lock));
            if (wait_flag)
                break;
            usleep(1000);
            continue;
        }
        LibMK_Result r = (LibMK_Result) libmk_exec_instruction(
            controller->handle, controller->instr);
        if (r != LIBMK_SUCCESS) {
            libmk_set_controller_error(controller, r);
            pthread_mutex_unlock(&(controller->instr_lock));
            break;
        }
        usleep(controller->instr->duration);
        // Move on to next instruction
        LibMK_Instruction* old = controller->instr;
        controller->instr = controller->instr->next;
        libmk_free_instruction(old);
        pthread_mutex_unlock(&(controller->instr_lock));
    }
    int r = libmk_disable_control(controller->handle);
    if (r != LIBMK_SUCCESS) {
        libmk_set_controller_error(controller, (LibMK_Result) r);
    }
    pthread_mutex_lock(&(controller->state_lock));
    controller->state = LIBMK_STATE_STOPPED;
    pthread_mutex_unlock(&(controller->state_lock));
}


void libmk_set_controller_error(LibMK_Controller* c, LibMK_Result e) {
    pthread_mutex_lock(&(c->error_lock));
    if (c->error == LIBMK_SUCCESS)
        c->error = e;
    pthread_mutex_unlock(&(c->error_lock));
}


LibMK_Result libmk_exec_instruction(LibMK_Handle* h, LibMK_Instruction* i) {
    if (i == NULL)
        return libmk_send_control_packet(h);
    else if (i->type == LIBMK_INSTR_ALL) {
        return libmk_set_all_led_color(h, i->colors);
    } else if (i->type == LIBMK_INSTR_FULL) {
        return libmk_set_full_color(h, i->color[0], i->color[1], i->color[2]);
    } else if (i->type == LIBMK_INSTR_SINGLE) {
        return libmk_set_single_led(
            h, i->r, i->c, i->color[0], i->color[1], i->color[2]);
    }
}


void libmk_stop_controller(LibMK_Controller* controller) {
    pthread_mutex_lock(&(controller->exit_flag_lock));
    controller->exit_flag = true;
    pthread_mutex_unlock(&(controller->exit_flag_lock));
}


void libmk_wait_controller(LibMK_Controller* controller) {
    pthread_mutex_lock(&(controller->exit_flag_lock));
    controller->wait_flag = true;
    pthread_mutex_unlock(&(controller->exit_flag_lock));
}


LibMK_Controller_State libmk_join_controller(
        LibMK_Controller* controller, double timeout) {
    LibMK_Controller_State s;
    clock_t t = clock();
    double elapsed;
    while (true) {
        pthread_mutex_lock(&(controller->state_lock));
        s = controller->state;
        pthread_mutex_unlock(&(controller->state_lock));
        if (s != LIBMK_STATE_ACTIVE)
            break;
        elapsed = ((double) (clock() - t)) / CLOCKS_PER_SEC;
        if (elapsed > timeout)
            return LIBMK_STATE_JOIN_ERR;
    }
    return s;
}


void libmk_free_instruction(LibMK_Instruction* i) {
    fflush(stdout);
    if (i->colors != NULL)
        free(i->colors);
    free(i);
}


LibMK_Instruction* libmk_create_instruction() {
    LibMK_Instruction* i =
        (LibMK_Instruction*) malloc(sizeof(LibMK_Instruction));
    i->duration = 0;
    i->id = -1;
    i->type = -1;
    i->next = NULL;
    i->colors = NULL;
    return i;
}


LibMK_Instruction* libmk_create_instruction_single(
        unsigned char row, unsigned char column, unsigned char color[3]) {
    LibMK_Instruction* i = libmk_create_instruction();
    i->type = LIBMK_INSTR_SINGLE;
    for (unsigned char j=0; j<3; j++)
        i->color[j] = color[j];
    i->r = row;
    i->c = column;
    return i;
}


LibMK_Instruction* libmk_create_instruction_full(unsigned char c[3]) {
    LibMK_Instruction* i = libmk_create_instruction();
    for (unsigned char j=0; j<3; j++)
        i->color[j] = c[j];
    i->type = LIBMK_INSTR_FULL;
    return i;
}


LibMK_Instruction* libmk_create_instruction_all(
        unsigned char c[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3]) {
    LibMK_Instruction* i = libmk_create_instruction();
    i->colors = (unsigned char*) malloc(
        sizeof(unsigned char) * LIBMK_MAX_ROWS * LIBMK_MAX_COLS * 3);
    memcpy(i->colors, c, LIBMK_MAX_ROWS * LIBMK_MAX_COLS * 3);
    i->type = LIBMK_INSTR_ALL;
    return i;
}


LibMK_Instruction* libmk_create_instruction_flash(
        unsigned char c[3], unsigned int delay, unsigned char n) {
    unsigned char color[3] = {0};
    LibMK_Instruction* i = libmk_create_instruction_full(color);
    LibMK_Instruction* k = i;
    LibMK_Instruction* l;
    for (unsigned char j=0; j<n; j++) {
        color[0] = ((double) j / (double) n) * c[0];
        color[1] = ((double) j / (double) n) * c[1];
        color[2] = ((double) j / (double) n) * c[2];
        l = libmk_create_instruction_full(color);
        l->duration = delay;
        k->next = l;
        k = l;
    }
    for (unsigned char j=n; j>0; j--) {
        color[0] = ((double) j / (double) n) * c[0];
        color[1] = ((double) j / (double) n) * c[1];
        color[2] = ((double) j / (double) n) * c[2];
        l = libmk_create_instruction_full(color);
        l->duration = delay;
        k->next = l;
        k = l;
    }
    return i;
}


int libmk_sched_instruction(
        LibMK_Controller* c, LibMK_Instruction* i) {
    pthread_mutex_lock(&(c->instr_lock));
    if (i->id != -1)
        return LIBMK_ERR_INVALID_ARG; // Instruction already scheduled!
    if (c->instr == NULL) {
        c->instr = i;
        c->instr->id = 1;
    } else {
        LibMK_Instruction* t = c->instr;
        while (t != NULL) {
            if (t->next == NULL) {
                t->next = i;
                t->next->id = t->id + 1;
                break;
            }
            t = t->next;
        }
    }
    if (i->next != NULL) {
        LibMK_Instruction *k = i->next;
        unsigned int id = i->id;
        while (k != NULL) {
            id += 1;
            k->id = id;
            k = k->next;
        }
    }
    int first_id = i->id;
    pthread_mutex_unlock(&(c->instr_lock));
    return first_id;
}


LibMK_Result libmk_cancel_instruction(LibMK_Controller* c, unsigned int id) {
    pthread_mutex_lock(&(c->instr_lock));
    LibMK_Instruction* prev = NULL;
    LibMK_Instruction* curr = c->instr;
    while (curr != NULL) {
        if (curr->id == id) {
            // Cancel this instruction
            if (prev != NULL)
                prev->next = curr->next;
            else  // First instruction in Linked List
                c->instr = curr->next;
            libmk_free_instruction(curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&(c->instr_lock));
    return LIBMK_SUCCESS;
}
