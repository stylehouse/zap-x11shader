#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>

// Function to blend two RGB pixel values based on opacity
unsigned long blendPixel(unsigned long srcPixel, unsigned long destPixel, double opacity) {
    unsigned long srcRed = (srcPixel & 0xFF0000) >> 16;
    unsigned long srcGreen = (srcPixel & 0xFF00) >> 8;
    unsigned long srcBlue = srcPixel & 0xFF;

    unsigned long destRed = (destPixel & 0xFF0000) >> 16;
    unsigned long destGreen = (destPixel & 0xFF00) >> 8;
    unsigned long destBlue = destPixel & 0xFF;

    unsigned long blendedRed = (srcRed * opacity) + (destRed * (1 - opacity));
    unsigned long blendedGreen = (srcGreen * opacity) + (destGreen * (1 - opacity));
    unsigned long blendedBlue = (srcBlue * opacity) + (destBlue * (1 - opacity));

    return (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
}

int main() {
    Display *display;
    Window root, window;
    XWindowAttributes root_attrs, window_attrs;
    XImage *image;
    XEvent event;
    XFontStruct *font_info;
    GC gc;

    // Open the display
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open display\n");
        return 1;
    }

    // Get the root window
    root = DefaultRootWindow(display);

    // Create a new window
    window = XCreateSimpleWindow(display, root, 0, 0, 800, 600, 0, 0, WhitePixel(display, 0));

    // Map the window to the screen
    XMapWindow(display, window);

    // Wait for the window to be mapped
    XSelectInput(display, window, ExposureMask);
    XWindowEvent(display, window, ExposureMask, &event);

    // Create the graphics context
    gc = XCreateGC(display, window, 0, NULL);






    // Set the font for drawing text
    char *font_name = "-*-helvetica-*-r-*-*-48-*-*-*-*-*-*-*";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "Unable to load font\n");
        return 1;
    }
    XSetFont(display, gc, font_info->fid);

    // Draw text on the window
    const char *text = "Hello";
    int text_length = strlen(text);
    int text_width = XTextWidth(font_info, text, text_length);
    int text_height = font_info->ascent + font_info->descent;
    int text_x = (window_attrs.width - text_width) / 2;
    int text_y = (window_attrs.height - text_height) / 2 + font_info->ascent;
    XDrawString(display, window, gc, text_x+ 80, text_y+ 80, text, text_length);

    // Load the image
    image = XGetImage(display, window, 0, 0, 800, 600, AllPlanes, ZPixmap);
    if (!image) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }

    // Set the opacity (alpha channel) for each pixel in the image
    double opacity = 0.4;  // Set the desired opacity level (0.0 to 1.0)
    for (int x = 0; x < image->width; x++) {
        for (int y = 0; y < image->height; y++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned long blendedPixel = blendPixel(pixel, BlackPixel(display, 0), opacity);
            XPutPixel(image, x, y, blendedPixel);
        }
    }







    // Get the window attributes
    XGetWindowAttributes(display, window, &window_attrs);
    XGetWindowAttributes(display, root, &root_attrs);

    // Put the blended image on the window
    XPutImage(display, window, DefaultGC(display, 0), image, 0, 0, 0, 0,
              window_attrs.width, window_attrs.height);

    // Flush the changes to the server
    XFlush(display);

    // Event loop
    while (1) {
        XNextEvent(display, &event);
        if (event.type == ButtonPress)
            break;
    }

    // Clean up and close the display connection
    XFreeFontInfo(NULL, font_info, 1);
    XDestroyImage(image);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
