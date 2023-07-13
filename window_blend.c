#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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
  window_name = "Nicotine+";
  window = XFindWindow(display, root, window_name);
  if (window == NULL) {
    fprintf(stderr, "Could not find window named \"%s\"\n", window_name);
    return 1;
  }

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
