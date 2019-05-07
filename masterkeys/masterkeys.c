/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
 *
 * Python wrapper around libmk
 *
 * Exposes a Python interface to libmk, allowing the use of lists
 * to control MasterKeys RGB keyboards. For python interface
 * documentation, please check __init__.py.
*/
#include "../libmk/libmk.h"
#include <stdlib.h>
#include <Python.h>


#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyInt_Check PyLong_Check
    #define PyInt_AsLong PyLong_AsLong
#endif


static PyObject* masterkeys_init(PyObject* self, PyObject* args) {
    /** Initialize the library upon import and register libmk_exit
     *
     * libmk requires initialization to initialize a libusb session. This
     * action is performed on import of the library. Upon (clean) exit
     * of the Python interpreter, the libmk_exit function should be
     * called to free these resources. This functions is therefore
     * registered to be executed at clean interpreter exit.
    */
    bool r = libmk_init();
    if (!r)
        return NULL;
    // Register libmk_exit with cast to void (as Py_AtExit takes only void)
    Py_AtExit((void*) libmk_exit);
    return PyBool_FromLong((long int) r);
}


static PyObject* masterkeys_detect_devices(PyObject* self, PyObject* args) {
    /** Return a tuple of connected MasterKeys devices
     *
     * Python usually runs with simple user permissions, but to access
     * (and identify) HID devices and unload kernel drivers, usually
     * root privileges are required.
    */
    LibMK_Model* models;
    int r = libmk_detect_devices(&models);
    if (r < 0)
        return Py_None;
    PyObject * tuple = PyTuple_New(r);
    PyObject * elem;
    for (short i=0; i < r; i++) {
        elem = PyInt_FromLong(models[i]);
        int s = PyTuple_SetItem(tuple, i, elem);
        if (s != 0) {
            // raise RuntimeError("Failed to set tuple item")
            PyErr_SetString(PyExc_RuntimeError, "Failed to set tuple item");
            return NULL;
        }
    }
    free(models);  // Models list is allocated in libmk_detect_devices
    return tuple;
}


static PyObject* masterkeys_set_device(PyObject* self, PyObject* args) {
    /** Set the device to control with the library
     *
     * The Python library only supports the control of a single device
     * using the global device handle of libmk.
    */
    LibMK_Model model;
    if (!PyArg_ParseTuple(args, "i", &model))
        return NULL;
    int r = libmk_set_device(model, NULL);
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_enable_control(PyObject* self, PyObject* args) {
    /** Enable control of the set control device */
    int r = libmk_enable_control(NULL);  // NULL -> global DeviceHandle
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_disable_control(PyObject* self, PyObject* args) {
    /** Disable control of the set control device */
    int r = libmk_disable_control(NULL);  // NULL -> global DeviceHandle
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_set_effect(PyObject* self, PyObject* args) {
    /** Set the effect of the keyboard to one of the built-ins */
    LibMK_Effect effect;
    if (!PyArg_ParseTuple(args, "i", &effect))
        return NULL;
    int r = libmk_set_effect(NULL, effect);
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_set_full_color(PyObject* self, PyObject* args) {
    /** Set the color of all LEDs on the keyboard
     *
     * Takes three Python integer values as arguments *tuple(r, g, b)
    */
    int r, g, b;
    if (!PyArg_ParseTuple(args, "iii", &r, &g, &b))
        return NULL;
    int result = libmk_set_full_color(NULL, r, g, b);
    return PyInt_FromLong(result);
}


static PyObject* masterkeys_set_all_led_color(PyObject* self, PyObject* args) {
    /** Set the color of all the LEDs on the keyboard individually
     *
     * Allocates a layout matrix of color values to set the color of all
     * the LEDs on the control device. LEDs not supported on a specific
     * keyboard are ignored. The argument should be given as a list of
     * lists of tuples [row][column][index].
    */
    PyObject* list, *sub, *tuple, *item;
    unsigned char value;
    // Parse the list of lists of tuples
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
        return NULL;
    // Check the size of the list (it must be a full list)
    if (PyList_Size(list) != LIBMK_MAX_ROWS) {
        // raise ValueError("Invalid number of list elements")
        PyErr_SetString(PyExc_ValueError, "Invalid number of list elements");
        return NULL;
    }
    // Populate layout color buffer
    unsigned char layout[LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3];
    for (unsigned char r=0; r < LIBMK_MAX_ROWS; r++) {
        // Assert that the item in the list is also a list
        sub = PyList_GetItem(list, r);
        if (!PyList_Check(sub)) {
            // raise TypeError("Invalid sub-type in list")
            PyErr_SetString(PyExc_TypeError, "Invalid sub-type in list");
            return NULL;
        } else if (PyList_Size(sub) != LIBMK_MAX_COLS) {
            // raise ValueError("Invalid number of sub-list elements")
            PyErr_SetString(
                PyExc_ValueError, "Invalid number of sub-list elements");
            return NULL;
        }
        for (unsigned char c=0; c < LIBMK_MAX_COLS; c++) {
            // Get sub-list item and assert that it is a tuple,3
            tuple = PyList_GetItem(sub, c);
            if (!PyTuple_Check(tuple)) {
                // raise TypeError("Invalid type of sub-list element")
                PyErr_SetString(
                    PyExc_TypeError, "Invalid type of sub-list element");
                return NULL;
            } else if (PyTuple_Size(tuple) != 3) {
                // raise ValueError("Invalid number of tuple elements")
                PyErr_SetString(
                    PyExc_ValueError, "Invalid number of tuple elements");
                return NULL;
            }
            for (unsigned char i=0; i < 3; i++) {
                // Read a value from the sub-list tuple-element
                item = PyTuple_GetItem(tuple, i);
                if (!PyInt_Check(item)) {
                    // raise TypeError("Invalid tuple element type")
                    PyErr_SetString(
                        PyExc_TypeError, "Invalid tuple element type");
                    return NULL;
                }
                // Update buffer with specified value
                value = (unsigned char) PyInt_AsLong(item);
                layout[r][c][i] = value;
            }
        }
    }
    // Perform the keyboard LED update
    int code = libmk_set_all_led_color(NULL, (unsigned char*) layout);
    return PyInt_FromLong(code);
}


static PyObject* masterkeys_set_ind_led_color(PyObject* self, PyObject* args) {
    /** Set the color of a single LED on the keyboard */
    int row, col, r, g, b;
    if (!PyArg_ParseTuple(args, "iiiii", &row, &col, &r, &g, &b))
        return NULL;
    int result = libmk_set_single_led(NULL, row, col, r, g, b);
    return PyInt_FromLong(result);
}


static PyObject* masterkeys_set_effect_details(PyObject* self, PyObject* args) {
    /** Set the an effect with additional arguments */
    unsigned char effect, direction, speed, amount;
    PyObject* foreground;
    PyObject* background;
    int r = PyArg_ParseTuple(
        args, "iiiiO!O!",
        &effect, &direction, &speed, &amount,
        &PyTuple_Type, &foreground, &PyTuple_Type, &background);
    if (!r) return NULL;
    LibMK_Effect_Details* effect_struct =
        (LibMK_Effect_Details*) malloc(sizeof(LibMK_Effect_Details));
    effect_struct->effect = (LibMK_Effect) effect;
    effect_struct->direction = direction;
    effect_struct->speed = speed;
    effect_struct->amount = amount;
    PyObject* iterim;
    for (unsigned char i=0; i < 3; i++) {
        iterim = PyTuple_GetItem(foreground, i);
        if (iterim == NULL)
            return NULL;
        effect_struct->foreground[i] = (unsigned char) PyLong_AsLong(iterim);
        iterim = PyTuple_GetItem(background, i);
        if (iterim == NULL)
            return NULL;
        effect_struct->background[i] = (unsigned char) PyLong_AsLong(iterim);
    }
    r = libmk_set_effect_details(NULL, effect_struct);
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_get_device_ident(PyObject* self, PyObject* args) {
    /** Return the bDevice value for the controlled keyboard */
    return PyInt_FromLong(libmk_get_device_ident(NULL));
}


static PyObject* masterkeys_get_active_profile(PyObject* self, PyObject* args) {
    /** Return the active profile on the keyboard */
    char profile;
    int r = libmk_get_active_profile(NULL, &profile);
    if (r != LIBMK_SUCCESS)
        return NULL;
    return PyInt_FromLong(profile);
}


static PyObject* masterkeys_set_active_profile(PyObject* self, PyObject* args) {
    /** Set the active profile on the keyboard */
    long profile;
    if (!PyArg_ParseTuple(args, "i", &profile))
        return NULL;
    int r = libmk_set_active_profile(NULL, profile);
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_save_profile(PyObject* self, PyObject* args) {
    /** Save changes made to the active profile */
    int r = libmk_save_profile(NULL);
    return PyInt_FromLong(r);
}


static PyObject* masterkeys_set_control_mode(PyObject* self, PyObject* args) {
    /** Set the control mode on the keyboard */
    LibMK_ControlMode mode;
    if (!PyArg_ParseTuple(args, "i", &mode))
        return NULL;
    return PyInt_FromLong(libmk_set_control_mode(NULL, mode));
}


static struct PyMethodDef masterkeys_funcs[] = {
    {
        "detect_devices",
        masterkeys_detect_devices,
        METH_NOARGS,
        "Return a tuple of connected device models"
    }, {
        "set_device",
        masterkeys_set_device,
        METH_VARARGS,
        "Set the device to control with the library"
    }, {
        "enable_control",
        masterkeys_enable_control,
        METH_NOARGS,
        "Enable control of the RGB LEDs on the controlled device"
    }, {
        "disable_control",
        masterkeys_disable_control,
        METH_NOARGS,
        "Disable control of the RGB LEDs on the controlled device"
    }, {
        "set_effect",
        masterkeys_set_effect,
        METH_VARARGS,
        "Set the LED lighting effect of the controlled device"
    }, {
        "set_all_led_color",
        masterkeys_set_all_led_color,
        METH_VARARGS,
        "Set the color of all the LEDs on the controlled device "
            "individually",
    }, {
        "set_full_led_color",
        masterkeys_set_full_color,
        METH_VARARGS,
        "Set the color of all the LEDs on the controlled device to a "
            "single color"
    }, {
       "set_ind_led_color",
       masterkeys_set_ind_led_color,
       METH_VARARGS,
       "Set the color of a single LED on the controlled device"
    }, {
        "set_effect_details",
        masterkeys_set_effect_details,
        METH_VARARGS,
        "Set the effect on the keyboard with specific arguments"
    }, {
        "get_device_ident",
        masterkeys_get_device_ident,
        METH_NOARGS,
        "Return the bDevice USB descriptor value"
    }, {
        "get_active_profile",
        masterkeys_get_active_profile,
        METH_VARARGS,
        "Return the number of the active profile"
    }, {
        "set_active_profile",
        masterkeys_set_active_profile,
        METH_VARARGS,
        "Set the active profile on the keyboard"
    }, {
       "save_profile",
       masterkeys_save_profile,
       METH_VARARGS,
       "Save the changes made to the active profile"
    }, {
        "set_control_mode",
        masterkeys_set_control_mode,
        METH_VARARGS,
        "Set the control mode of the keyboard"
    }, {NULL, NULL, 0, NULL}
};


#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initmasterkeys(void) {
    masterkeys_init(NULL, NULL);
    (void) Py_InitModule("masterkeys", masterkeys_funcs);
}
#else  // PY_MAJOR_VERSION >= 3
static struct PyModuleDef masterkeys_module_def = {
    PyModuleDef_HEAD_INIT,
    "masterkeys",
    "Library to control MasterKeys keyboard lighting",
    -1,
    masterkeys_funcs
};
PyMODINIT_FUNC PyInit_masterkeys(void) {
    masterkeys_init(NULL, NULL);
    return PyModule_Create(&masterkeys_module_def);
}
#endif
