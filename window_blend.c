#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// < maybe more doable in wayland
//   XGetImage() bails: X Error of failed request:  BadMatch (invalid parameter attributes)



Window find_nicotine_window(Display *display, Window root) {
    Window *children;
    Window parent;
    unsigned int num_children;
    int i;

    Atom net_client_list;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    Window *wm_name_list;

    net_client_list = XInternAtom(display, "_NET_CLIENT_LIST", False);
    XGetWindowProperty(display, root, net_client_list, 0, ~0, False, AnyPropertyType, &actual_type, &actual_format,
                       &nitems, &bytes_after, (unsigned char **)&wm_name_list);

    Atom wm_name_atom = XInternAtom(display, "WM_NAME", False);

    for (i = 0; i < nitems; i++) {
        XTextProperty wm_name_prop;
        XGetTextProperty(display, wm_name_list[i], &wm_name_prop, wm_name_atom);

        if (wm_name_prop.value && strncmp((char *)wm_name_prop.value, "Nicotine+", 9) == 0) {
            return wm_name_list[i];
        }
    }

    for (i = 0; i < nitems; i++) {
        if (XQueryTree(display, wm_name_list[i], &root, &parent, &children, &num_children)) {
            Window nicotine_window = find_nicotine_window(display, wm_name_list[i]);
            if (nicotine_window != None) {
                XFree(children);
                XFree(wm_name_list);
                return nicotine_window;
            }
            XFree(children);
        }
    }

    XFree(wm_name_list);
    return None;
}
int main(int argc, char *argv[]) {
    Display *display;
    Window root;
    Window window;
    char *window_name;
    int width, height;
    int x, y;
    unsigned char *pixel;
    unsigned char t;

    // Open the X display.
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    // Get the root window.
    root = RootWindow(display, DefaultScreen(display));

    // Find the window named "Nicotine+".
    window = find_nicotine_window(display, root);
    fprintf(stdout, "found nico\n");

    // Get the window attributes to obtain the width and height.
    XWindowAttributes window_attr;
    XGetWindowAttributes(display, window, &window_attr);
    width = window_attr.width;
    height = window_attr.height;

    // Create a graphics context (GC) for the window.
    GC gc = XCreateGC(display, window, 0, NULL);

    // Create an XImage structure to hold the pixel data.
    fprintf(stdout, "pre-XGetImage: %d x %d\n", width, height);
    XImage *image = XGetImage(display, window, 0, 0, width, height, AllPlanes, ZPixmap);
    fprintf(stdout, "post-XGetImage!\n");

    if (image == NULL) {
        fprintf(stderr, "Failed to get image for window\n");
        return 1;
    }

    // Blend the image with the "boreo_desert.jpg" image.
    FILE *fp = fopen("boreo_desert.jpg", "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open \"boreo_desert.jpg\"\n");
        return 1;
    }
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            t = fgetc(fp);
            pixel = image->data + (y * image->bytes_per_line + x * image->bits_per_pixel / 8);
            unsigned char r = pixel[0];
            unsigned char g = pixel[1];
            unsigned char b = pixel[2];
            pixel[0] = r * t / 255;
            pixel[1] = g * t / 255;
            pixel[2] = b * t / 255;
        }
    }
    fclose(fp);

    // Put the modified image onto the window.
    XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);

    // Free resources and close the X display.
    XFreeGC(display, gc);
    XDestroyImage(image);
    XCloseDisplay(display);

    return 0;
}
