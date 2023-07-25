# zap-x11shader
supervisor running|monitoring commands and decking your field of windows with glorious colour and texture

# description
To shade an X window, possibly blending an image with it, while leaving the gui program in the window perfectly functional, for aesthetics.

Results may be quite hectic:

https://github.com/stylehouse/zap/assets/20439374/0800f95b-b321-46e8-a03d-103786e080b6

# not yet doing the trick

Did something spastic at c011995d70bf7ec56fac2

Probably not feeding data properly somewhere.

Lots of researching docs for the AI to be the C maestro.

https://www.x.org/releases/current/doc/libX11/libX11/libX11.html

Just so thin on examples out there.

# somehow?

Ideally it should do this on the gpu. Methods so far have involved placing pixels back on the window.

# in wayland?

Wayland is a newer display server protocol than X11.

# in go?

go is a newer dostuff ecosystem than C.

prompt: write a Go script to open a jpg and blend its pixels onto an X window whose title is /^Nicotine/

everything was lost due to the opaqueness of git submodules, and `go get` wanting to be in one..?

https://pkg.go.dev/github.com/eankeen/xgbutil/xgraphics#hdr-A_quick_example


