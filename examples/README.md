# Custom Lighting Examples
This folder contains a set of examples that make use of either the 
Python library or the native C library to show interactive or otherwise
interesting LED effects on the keyboard. Some examples may require
additional dependencies to run.

## AmbiLight
This example turns the connected keyboard into an ambilight-like device
by showing the average color of the screen (determined using libx11,
so Wayland is currently not supported).

This example also functions as a stress test of sorts for the library.
By continuously updating the color of all keys of the keyboard (using 
`set_all_led_color` to avoid the flickering effect of the 
`set_full_led_color` function).

**Disclaimer**: Running this example for a long period of time may cause
the chip inside the keyboard to heat up and reduce lighting performance.
So far, the inability of disabling LED control without power cycling the
keyboard and the failure to update the keys of the first packet have 
been observed.

## PhotoViewer
Takes the average of sections of the image to correspond to the color
of a single key, thus showing the image selected on the keyboard in 
a much reduced resolution. As it is pure Python and it loops over a PIL
pixel access object, the current implementation is quite slow.
