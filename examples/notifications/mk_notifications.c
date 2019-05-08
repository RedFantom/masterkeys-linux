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
#include <unistd.h>
#include <Python.h>

#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_Check PyLong_Check
    #define PyInt_AsLong PyLong_AsLong
#endif

/// Global variables
bool exit_requested = false;
unsigned char target_color[3] = {0};
bool target_override = false;
double speed = 20.0;
int flash_repeat = 2;
double flash_time = 1.0;

/// Mutexes
pthread_mutex_t exit_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t target_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t keyboard_lock = PTHREAD_MUTEX_INITIALIZER;

/// Threads
pthread_t keyboard_thread;
pthread_t capture_thread;

/// Screen capture
CaptureArgs* capture_args;


void mkn_exit() {
    pthread_mutex_lock(&exit_lock);
    exit_requested = true;
    pthread_mutex_unlock(&exit_lock);
    pthread_join(keyboard_thread, NULL);
    pthread_join(capture_thread, NULL);
    libmk_disable_control(NULL);
    libmk_exit();
}


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
    bool exit = false, override = false;
    
    while (true) {
        pthread_mutex_lock(&exit_lock);
        exit = exit_requested;
        pthread_mutex_unlock(&exit_lock);
        if (exit)
            break;
        
        pthread_mutex_lock(&target_lock);
        for (int i=0; i<3; i++)
            target[i] = target_color[i];
        override = target_override;
        pthread_mutex_unlock(&target_lock);
        
        bool equal = (
            target[0] == previous[0] &&
            target[1] == previous[1] &&
            target[2] == previous[2]);
        if (equal) { // If equal, sleep to prevent overactiveness
            continue;
        }
        
        if (!target_override)
            for (int i=0; i<3; i++) {
                int diff = (int) target[i] - (int) previous[i];
                target[i] = previous[i] + (unsigned char) (diff / speed);
                previous[i] = target[i];
            }
        int r = libmk_set_full_color(NULL, target[0], target[1], target[2]);
        int* return_code = (int*) malloc(sizeof(int));
        *(return_code) = r;
        if (r != LIBMK_SUCCESS)
            pthread_exit((void*) return_code);
        
        usleep(10000);
    }
    pthread_exit(LIBMK_SUCCESS);
}


static PyObject* init(PyObject* self, PyObject* args) {
    /** Initialize the library and setup for capture */
    if (!libmk_init()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to init libmk");
        return NULL;
    }
    
    int divider, lower, upper, sat_bias;
    int brightness_norm;
    if (!PyArg_ParseTuple(args, "iiiiidid", &divider, &lower, &upper, &sat_bias,
                          &brightness_norm, &speed, &flash_repeat, &flash_time)) {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        libmk_exit();
        return NULL;
    }
    
    capture_args = init_capture(divider, sat_bias, lower, upper,
        brightness_norm != 0, &target_color, &target_lock, &exit_requested,
        &exit_lock, &keyboard_lock);
    
    if (args == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to build CaptureArgs struct");
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
    
    Py_AtExit((void*) mkn_exit);
    return PyInt_FromLong(LIBMK_SUCCESS);
}


static PyObject* start(PyObject* self, PyObject* args) {
    /** Start running capture and keyboard threads */
    pthread_create(&capture_thread, NULL, (void*) capturer, (void*) capture_args);
    pthread_create(&keyboard_thread, NULL, (void*) keyboard_updater, NULL);
    return PyInt_FromLong(0);
}


static PyObject* stop(PyObject* self, PyObject* args) {
    /** Stop and join capture and keyboard threads */
    pthread_mutex_lock(&exit_lock);
    exit_requested = true;
    pthread_mutex_unlock(&exit_lock);
    int return_kb, return_cp;
    pthread_join(keyboard_thread, NULL);
    pthread_join(capture_thread, NULL);
    
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
    int w, h, divider, lower, upper, sat_bias, brightness_norm;
    if (!PyArg_ParseTuple(args, "O!iiiiiii",
            &PyList_Type, &list, &divider, &w, &h, &lower, &upper,
            &sat_bias, &brightness_norm))
        return NULL;
    unsigned char data[w][h][3];
    PyObject* column, *row, *e;
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
        data, w, h, result, divider, sat_bias, lower, upper, brightness_norm==1);
    for (int i=0; i<3; i++)
        PyTuple_SetItem(tuple, i, PyInt_FromLong(result[i]));
    return tuple;
}


void __flash_keyboard(unsigned char* color) {
    pthread_mutex_lock(&target_lock);
    target_override = true;
    pthread_mutex_unlock(&target_lock);
    unsigned char r = color[0], g = color[1], b = color[2];
    for (int i=0; i<256; i++) {
        pthread_mutex_unlock(&target_lock);
        for (int j=0; j<3; j++) {
            target_color[j] = (unsigned char) (
                (double) color[j] * (double) i / 255.0);
            usleep((int) (flash_time / (2*255.0) * 1000000));
        }
        pthread_mutex_unlock(&target_lock);
    }
    for (int i=255; i>-1; i--) {
        pthread_mutex_unlock(&target_lock);
        for (int j=0; j<3; j++) {
            target_color[j] = (unsigned char) (
                (double) color[j] * (double) i / 255.0);
            usleep((int) (flash_time / (2*255.0) * 1000000));
        }
        pthread_mutex_unlock(&target_lock);
    }
    pthread_mutex_lock(&target_lock);
    target_override = false;
    pthread_mutex_unlock(&target_lock);
}


void _flash_keyboard(unsigned char* color) {
    pthread_mutex_lock(&keyboard_lock);
    for (int i=0; i<flash_repeat; i++)
        __flash_keyboard(color);
    pthread_mutex_unlock(&keyboard_lock);
    free(color);
}


static PyObject* flash_keyboard(PyObject* self, PyObject* args) {
    /** Asynchronously flash the keyboard from Python in a sepcific color */
    pthread_t thread;
    int r, g, b;
    if (!PyArg_ParseTuple(args, "iii", &r, &g, &b)) {
        PyErr_SetString(PyExc_ValueError, "Failed to parse arguments");
        return NULL;
    }
    unsigned char* color = (unsigned char*) malloc(sizeof(unsigned char) * 3);
    color[0] = r; color[1] = g; color[2] = b;
    pthread_create(&thread, NULL, (void*) _flash_keyboard, (void*) color);
    return PyInt_FromLong(0);
}


static struct PyMethodDef mk_notifications_funcs[] = {{
        "init",
        init,
        METH_VARARGS,
        "Initialize the library and dependencies"
    }, {
        "start",
        start,
        METH_NOARGS,
        "Start the threads required for the program"
    }, {
        "stop",
        stop,
        METH_NOARGS,
        "Stop the threads running for the program"
    }, {
        "calculate_dominant_color",
        py_calculate_dominant_color,
        METH_VARARGS,
        "Calculate the dominant color in an image"
    }, {
        "flash_keyboard",
        flash_keyboard,
        METH_VARARGS,
        "Flash the keyboard in a given color"
    }, {NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initmk_notifications(void) {
    (void) Py_InitModule("mk_notifications", mk_notifications_funcs);
}
#else  // PY_MAJOR_VERSION >= 3
static struct PyModuleDef mk_notifications_module_def = {
    PyModuleDef_HEAD_INIT,
    "mk_notifications",
    "Library to control MasterKeys keyboard lighting",
    -1,
    mk_notifications_funcs
};
PyMODINIT_FUNC PyInit_mk_notifications(void) {
    return PyModule_Create(&mk_notifications_module_def);
}
#endif
