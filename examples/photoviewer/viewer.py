"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2018-2019 RedFantom
"""
import masterkeys as mk
from os import path
from PIL import Image
import sys


def print_progress(done, total):
    """Print a progress bar using #"""
    filled, percent = int(60 * done / total), done * 100 / total
    bar = "[{}]".format("#" * filled + " " * (60 - filled))
    sys.stdout.write("\r{} {:02.1f}%\r".format(bar, percent))
    sys.stdout.flush()


if __name__ == '__main__':
    """
    Open a file from a path given on input and process that to create a 
    grid of key colors to set on the keyboard.
    """
    # Open the file
    file = input("File: ")
    if not path.exists(file):
        print("That file does not exist.")
        exit(-1)
    img = Image.open(file)

    # Process the image
    w, h = img.size
    pixels = img.load()
    layout = mk.build_layout_list()
    w_p_c, h_p_r = (w // mk.MAX_COLS), (h // mk.MAX_ROWS)
    done, total = 0, mk.MAX_ROWS * mk.MAX_COLS
    for r in range(mk.MAX_ROWS):
        for c in range(mk.MAX_COLS):
            sums = [0, 0, 0]
            xrange = list(range(w_p_c * c, w_p_c * (c + 1)))
            yrange = list(range(h_p_r * r, h_p_r * (r + 1)))
            for x in xrange:
                for y in yrange:
                    for i in range(3):
                        sums[i] += pixels[x, y][i]
            color = (v // (w_p_c * h_p_r) for v in sums)
            color = tuple(map(int, color))
            layout[r][c] = color

            done += 1
            print_progress(done, total)
    print()

    # Update the color of the keyboard
    devices = mk.detect_devices()
    if len(devices) == 0:
        print("No devices connected.")
        exit(-2)
    mk.set_device(devices[0])
    mk.enable_control()
    mk.set_all_led_color(layout)
    mk.disable_control()
