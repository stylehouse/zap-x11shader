#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <jpeglib.h>

// < maybe more doable in wayland


// Function to load the JPEG image using libjpeg
unsigned char* loadJPEG(const char* filename, int* width, int* height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* infile;
    JSAMPARRAY buffer;
    int row_stride;

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Error opening JPEG file: %s\n", filename);
        return NULL;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    int numChannels = cinfo.num_components;

    unsigned char* imageData = (unsigned char*)malloc((*width) * (*height) * numChannels);
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, *width * numChannels, 1);
    row_stride = (*width) * numChannels;

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(imageData + (cinfo.output_scanline - 1) * row_stride, buffer[0], row_stride);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return imageData;
}


unsigned char onechannel(unsigned char src, unsigned char dst) {
    int brightness = src - 200;
    if (brightness > 0) {
        src = 0x00;
    }
    return src;
}
// Function to alpha blend two pixels
unsigned long somekindaBlend(unsigned long sourcePixel, unsigned long destPixel, unsigned char alpha) {
    unsigned char sourceRed = (sourcePixel & 0x00FF0000) >> 16;
    unsigned char sourceGreen = (sourcePixel & 0x0000FF00) >> 8;
    unsigned char sourceBlue = sourcePixel & 0x000000FF;

    unsigned char destRed = (destPixel & 0x00FF0000) >> 16;
    unsigned char destGreen = (destPixel & 0x0000FF00) >> 8;
    unsigned char destBlue = destPixel & 0x000000FF;

    unsigned char blendedRed = (sourceRed * alpha + destRed * (255 - alpha)) / 255;
    unsigned char blendedGreen = (sourceGreen * alpha + destGreen * (255 - alpha)) / 255;
    unsigned char blendedBlue = (sourceBlue * alpha + destBlue * (255 - alpha)) / 255;

    return (blendedRed << 16) | (blendedGreen << 8) | blendedBlue;
}

const char* testPixel = "\xaa\x55\xaa\x00";
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
    int screen;
    char *window_name;
    int width, height;
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


    // Load the "boreo_desert.jpg" image
    int jpgWidth, jpgHeight;
    unsigned char* desertRaw = loadJPEG("boreo_desert.jpg", &jpgWidth, &jpgHeight);
    if (desertRaw == NULL) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }
    // Create an XImage object for "boreo_desert.jpg" image
    XImage *desert = XCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen),
                         ZPixmap, 0, (char*)desertRaw, jpgWidth, jpgHeight, 32, 0);
    if (desert == NULL) {
        fprintf(stderr, "Failed to XCreateImage\n");
        return 1;
    }




    // < loop would be:

    // Create an XImage structure to hold the pixel data.
    XImage *image = XGetImage(display, window, 0, 0, width, height, AllPlanes, ZPixmap);
    fprintf(stdout, "post-XGetImage!\n");

    if (image == NULL) {
        fprintf(stderr, "Failed to XGetImage for window\n");
        return 1;
    }
    fprintf(stdout, " Jpeg geo: x=%d  y=%d\n", jpgWidth, jpgHeight);
    
    // Perform alpha blending with the XImage
    for (unsigned int y = 0; y < height; y++) {
        // fprintf(stdout, "iter y=%d\n",y);
        for (unsigned int x = 0; x < width; x++) {
            // if (y == 595) fprintf(stdout, "    iter x=%d\n",x);
            if (y == 595 && x == 1340) {
                fprintf(stdout, "    to x=%d  y=%d\n",x % jpgWidth, y % jpgHeight);
            }

            // < segfaults:
            //unsigned long sourcePixel = XGetPixel(desert, x % jpgWidth, y % jpgHeight);
            // just use black:
            unsigned long sourcePixel = *((unsigned long*)testPixel);
            
            unsigned long destPixel = XGetPixel(image, x, y);
            unsigned char alpha = 255;  // Set your desired alpha value

            unsigned long blendedPixel = somekindaBlend(destPixel, sourcePixel, alpha);
            XPutPixel(image, x, y, blendedPixel);
        }
    }

    // Put the modified image onto the window.
    XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);

    // Free resources and close the X display.
    XFreeGC(display, gc);
    XDestroyImage(image);
    XCloseDisplay(display);

    return 0;
}
