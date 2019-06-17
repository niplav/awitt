#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

int main(int argc, char** argv)
{
	int focusstate, revert, i, nchildren;
	char* wname;
	Status s1, s2, s3;
	Window* w, * par, * root, ** children;
	Display* dpy=XOpenDisplay(NULL);
	XTextProperty* title=(XTextProperty*)calloc(1, sizeof(XTextProperty));
	XClassHint* class=(XClassHint*)XAllocClassHint();

	w=(Window*)calloc(1, sizeof(Window));
	par=(Window*)calloc(1, sizeof(Window));
	root=(Window*)calloc(1, sizeof(Window));
	children=(Window*)calloc(16, sizeof(Window));

	if(dpy==NULL)
	{
		fprintf(stderr, "%s: could not open display\n", argv[0]);
		exit(1);
	}

	for(;;)
	{
		focusstate=XGetInputFocus(dpy, w, &revert);

		printf("%d|%d, parent: %d, pointer_root: %d, none: %d\n", focusstate, revert, RevertToParent, RevertToPointerRoot, RevertToNone);

		if(revert==2)
		{
			/* Needless to say, this is _very hacky. */
			/* TODO: carve this into the side of god */
			printf("reverting to parent\n");
			s3=XQueryTree(dpy, *w, root, par, children, &nchildren);
			w=par;
		}

		s1=XGetTextProperty(dpy, *w, title, XA_WM_NAME);
		s2=XGetClassHint(dpy, *w, class);

		printf("s1: %d, s2: %d\n", s1, s2);
		if(s1!=0)
			printf("%s\n", title->value);
		if(s2!=0)
			printf("%s %s\n", class->res_name, class->res_class);
		sleep(1);
	}

	XCloseDisplay(dpy);

	free(title);
	XFree(class);
	XFree(children);

	return 0;
}
