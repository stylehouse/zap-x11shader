#!/usr/bin/env python3
import Xlib.display
import numpy as np
from PIL import Image

image_path = "boreo_desert.jpg"

def create_window_with_image():
    # Connect to the X server
    display = Xlib.display.Display()

    # Create a new window
    screen = display.screen()
    window = screen.root.create_window(
        0, 0, 800, 600, 0, screen.root_depth, Xlib.X.InputOutput, Xlib.X.CopyFromParent,
        background_pixel=screen.white_pixel, event_mask=Xlib.X.ExposureMask | Xlib.X.ButtonPressMask
    )
    window.map()

    # Get the window's visual
    visual = screen.root_visual

    # Load and resize the image
    image = Image.open(image_path)
    image = image.resize((800, 600))

    # Convert the image to RGBA mode
    image = image.convert("RGBA")

    # Create an XImage from the image data
    image_data = np.array(image)
    image_data_bytes = image_data.tobytes()
    image_width, image_height = image.size
    ximage = Xlib.ext.image_to_pil.ImageWrapper(
        image_width, image_height, image_data_bytes,
        format='rgba', bits_per_pixel=32, byte_order=display.byte_order,
        bitmap_unit=32, bitmap_bit_order=display.byte_order
    )

    # Set the window attributes for transparency
    window.change_attributes(backing_store=Xlib.X.NotUseful, save_under=False)
    window.change_property(
        display.intern_atom('_NET_WM_WINDOW_OPACITY'), Xlib.X.XA_CARDINAL, 32, [0xFFFFFFFF]
    )

    # Set the compositing hint to enable blending
    window.change_property(
        display.intern_atom('_NET_WM_WINDOW_TYPE'), Xlib.X.XA_ATOM, 32,
        [display.intern_atom('_NET_WM_WINDOW_TYPE_NORMAL')]
    )

    # Set the visual for the window
    window.change_attributes(visual=visual)

    # Create a graphics context for drawing
    gc = window.create_gc(
        foreground=screen.black_pixel, background=screen.white_pixel,
        graphics_exposures=False, line_width=1
    )

    # Draw the blended image on the window
    window.put_image(gc, 0, 0, ximage)

    # Flush the changes to the server
    display.flush()

    # Enter the event loop
    while True:
        event = display.next_event()
        if event.type == Xlib.X.ButtonPress:
            break

    # Clean up and close the display connection
    display.close()

create_window_with_image()