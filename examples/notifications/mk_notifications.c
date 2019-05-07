/** Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include "capture.h"
#include "libmk.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <Python.h>

#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_Check PyLong_Check
    #define PyInt_AsLong PyLong_AsLong
#endif

/// Global variables
bool exit_requested = false;
unsigned char target_color[3] = {0};

/// Mutexes
pthread_mutex_t exit_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t target_lock = PTHREAD_MUTEX_INITIALIZER;

/// Threads
pthread_t keyboard_thread;
pthread_t capture_thread;

/// Screen capture
CaptureArgs* args;


void keyboard_updater() {
    /** Thread controlling the color of a keyboard
     *
     * The thread sets the color of the keyboard to a color set by a
     * different thread in target_color. target_color is protected by
     * target_lock.
     *
     * Expects that the keyboard to controlled is in the global device
     * handle and control is enabled.
     */
    unsigned char target[3], previous[3];
    bool exit = false;
    
    while (true) {
        pthread_mutex_lock(&exit_lock);
        exit = exit_requested;
        pthread_mutex_unlock(&exit_lock);
        if (exit)
            break;
        
        pthread_mutex_lock(&target_lock);
        for (int i=0; i<3; i++)
            target[i] = target_color[i];
        pthread_mutex_unlock(&target_lock);
        
        bool equal = (
            target[0] == previous[0] &&
            target[1] == previous[1] &&
            target[2] == previous[2]);
        if (equal) { // If equal, sleep to prevent overactiveness
            usleep(10000);
            continue;
        }
        
        int r = libmk_set_full_color(NULL, target[0], target[1], target[2]);
        if (r != LIBMK_SUCCESS)
            pthread_exit(r);
        
        for (int i=0; i<3; i++)
            previous[i] = target[i];
        usleep(1000);
    }
    pthread_exit(LIBMK_SUCCESS);
}


static PyObject* init(PyObject* self, PyObject* args) {
    /** Initialize the library and setup for capture */
    if (!libmk_init())
        return NULL;
    
    int divider, lower, upper, sat_bias;
    bool brightness_norm;
    if (!PyArg_ParseTuple(args, "iiiip", &divider, &lower, &upper, &sat_bias,
                          &brightness_norm)) {
        libmk_exit();
        return NULL;
    }
    
    args = init_capture(divider, sat_bias, lower, upper, brightness_norm, &target_color,
        &target_lock, &exit_requested, &exit_lock);
    
    if (args == NULL) {
        libmk_exit();
        return NULL;
    }
    
    
    LibMK_Model* models;
    int n = libmk_detect_devices(&models);
    if (n <= 0) {
        libmk_exit();
        return PyInt_FromLong(1);
    }
    
    LibMK_Model model = models[0]; // Take first device found as target
    int r = libmk_set_device(model, NULL); // Use global device handle
    if (r != LIBMK_SUCCESS) {
        libmk_exit();
        return PyInt_FromLong(r);
    }
    
    r = libmk_enable_control(NULL);
    if (r != LIBMK_SUCCESS) {
        libmk_exit();
        return PyInt_FromLong(r);
    }
}


static PyObject* start(PyObject* self, PyObject* args) {
    /** Start running capture and keyboard threads */
    pthread_create(&capture_thread, NULL, capturer, (void*) args);
    pthread_create(&keyboard_thread, NULL, keyboard_updater, NULL);
}


static PyObject* stop(PyObject* self, PyObject* args) {
    /** Stop and join capture and keyboard threads */
    pthread_mutex_lock(&exit_lock);
    exit_requested = true;
    pthread_mutex_unlock(&exit_lock);
    int return_kb, return_cp;
    pthread_join(keyboard_thread, &return_kb);
    pthread_join(capture_thread, &return_cp);
    
    PyObject* tuple = PyTuple_New(2);
    int s = PyTuple_SetItem(tuple, 0, PyInt_FromLong(return_kb));
    s += PyTuple_SetItem(tuple, 1, PyInt_FromLong(return_cp));
    if (s != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set tuple item");
        return NULL;
    }
    return tuple;
}


static PyObject* py_calculate_dominant_color(PyObject* self, PyObject* args) {
    /** Python interface to fast dominant color calculation function */
    PyObject* list;
    int w, h, divider, lower, upper, sat_bias;
    bool brightness_norm;
    if (!PyArg_ParseTuple(args, "O!iiiiiip",
            &PyList_Type, &list, &divider, &w, &h, &lower, &upper,
            &sat_bias, &brightness_norm))
        return NULL;
    
    unsigned char data[w][h][3];
    PyObject* column, row, e;
    for (int x=0; x<w; x++) {
        column = PyList_GetItem(list, x);
        if (!PyList_Check(column)) {
            PyErr_SetString(PyExc_TypeError, "Invalid type in data");
            return NULL;
        }
        for (int y=0; y<h; y++) {
            row = PyList_GetItem(column, y);
            if (!PyTuple_Check(row)) {
                PyErr_SetString(PyExc_TypeError, "Invalid type in data");
                return NULL;
            }
            for (int i=0; i<3; i++) {
                e = PyTuple_GetItem(row, i);
                if (!PyInt_Check(e)) {
                    PyErr_SetString(
                        PyExc_TypeError, "Invalid type in data");
                    return NULL;
                }
                data[x][y][i] = (unsigned char) PyInt_AsLong(e);
            }
        }
    }
    
    unsigned char result[3];
    PyObject* tuple = PyTuple_New(3);
    calc_dominant_color(
        data, w, h, result, divider, sat_bias, lower, upper, brightness_norm);
    for (int i=0; i<3; i++)
        PyTuple_SetItem(tuple, i, result[i]);
    return tuple;
}
