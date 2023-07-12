#!/usr/bin/env python3
import Xlib.display
import numpy as np
import cv2
import re
import pprint
def dd(data,depth=7):
    pp = pprint.PrettyPrinter(depth=depth)
    pp.pprint(data)

# Alters the pixels directly on the screen for an existing X window#
#  blending it with an image via multiply.
def alter_window_pixels(display, window, image):
    win = window.get_geometry()
    image_width, image_height = image.shape[:2]

    filtered_image = np.zeros((win.width, win.height, 3), dtype=np.uint8)
    raw_data = window.get_image(0, 0, win.width, win.height, Xlib.X.ZPixmap, 0xffffffff).data
    for x in range(win.width):
        for y in range(win.height):
            pixel_offset = (y * win.width + x) * 4
            pixel = raw_data[pixel_offset:pixel_offset + 3][::-1]  # BGR to RGB
            filtered_image[x, y] = np.multiply(pixel, image[x % image_width, y % image_height])

    window.put_image(0, 0, filtered_image.flatten().tobytes(), win.width, win.height, Xlib.X.ZPixmap, 32)

def find_nicotine_window(d,root):
    window_ids = root.get_full_property(d.intern_atom('_NET_CLIENT_LIST'), Xlib.X.AnyPropertyType).value
    nicotine_window = None
    for window_id in window_ids:
        window = d.create_resource_object('window', window_id)
        wm_name = window.get_wm_name()
        dd(wm_name)
        if re.search(r'^Nicotine\+',wm_name):
            nicotine_window = window
            break
    if not nicotine_window:
       exit(8)
    return nicotine_window

def doit():
    d = Xlib.display.Display()
    root = d.screen().root
    window = find_nicotine_window(d,root)
    image = cv2.imread("boreo_desert.jpg")

    alter_window_pixels(d, window, image)

if __name__ == "__main__":
    doit()