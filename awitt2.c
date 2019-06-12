#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

int main(int argc, char** argv)
{
	int focusstate, revert;
	char* wname;
	Window* w=(Window*)calloc(1, sizeof(Window));
	Display* dpy=XOpenDisplay(NULL);

	if(dpy==NULL)
	{
		fprintf(stderr, "%s: could not open display\n", argv[0]);
		exit(1);
	}

	for(;;)
	{
		focusstate=XGetInputFocus(dpy, w, &revert);
		printf("%d|%d, parent: %d, pointer_root: %d, none: %d\n", focusstate, revert, RevertToParent, RevertToPointerRoot, RevertToNone);
		XFetchName(dpy, *w, &wname);
		printf("%s\n", wname);
		sleep(1);
	}

	XCloseDisplay(dpy);

	return 0;
}
