#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

Window find_nicotine_window(Display *display, Window root) {
  Atom net_client_list = XInternAtom(display, "_NET_CLIENT_LIST", False);
  unsigned long *window_ids;
  int window_count;
  Window window = None;

  // Get the list of client windows.
  XGetWindowProperty(display, root, net_client_list, 0, 0, False, AnyPropertyType, &window_count, &window_ids);

  // Iterate through the list of client windows and find the window named "Nicotine+".
  for (int i = 0; i < window_count; i++) {
    Window cur_window = window_ids[i];
    char *wm_name = XGetWMName(display, cur_window);
    if (wm_name && strncmp(wm_name, "Nicotine+", 9) == 0) {
      window = cur_window;
      break;
    }
  }

  // If the window was not found, exit with an error.
  if (window == None) {
    fprintf(stderr, "Could not find window named \"%s\"\n", "Nicotine+");
    exit(1);
  }

  return window;
}

int main(int argc, char *argv[]) {
  Display *display;
  Window root;
  Window window;
  char *window_name;
  XImage *image;
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

  // Get the image for the window.
  image = XGetImage(display, window, 0, 0, width, height, AllPlanes, ZPixmap);
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
      pixel = image->data + (y * image->width + x) * 4;
      unsigned char r = pixel[0];
      unsigned char g = pixel[1];
      unsigned char b = pixel[2];
      pixel[0] = r * t / 255;
      pixel[1] = g * t / 255;
      pixel[2] = b * t / 255;
    }
  }
  fclose(fp);

  // Put the image back on the window.
  XPutImage(display, window, root, image, 0, 0, 0, 0, width, height);

  // Free the image.
  XFree(image);

  // Close the X display.
  XCloseDisplay(display);

  return 0;
}
