# Custom Lighting Examples
This folder contains a set of examples that make use of either the 
Python library or the native C library to show interactive or otherwise
interesting LED effects on the keyboard. Some examples may require
additional dependencies to run.

## AmbiLight
This example turns the connected keyboard into an ambilight-like device
by showing the average color of the screen (determined using libx11,
so Wayland is currently not supported).

## PhotoViewer
Takes the average of sections of the image to correspond to the color
of a single key, thus showing the image selected on the keyboard in 
a much reduced resolution. As it is pure Python and it loops over a PIL
pixel access object, the current implementation is quite slow.

## Notifications
Mostly an improvement of the AmbiLight example, the notifications
example is a completely rewritten version that runs from Python.
Performance critical functions are implemented in C.

Where AmbiLight only displays the dominant color shown on the screen,
the notifications example will interrupt the AmbiLight color stream and
flash the keyboard in the dominant color of a notification.

Options are available to be set through the Python file
`notifications.py`. The notifications example shows the potential of
RGB keyboards as more than just gimmicks.
