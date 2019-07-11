#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

int main(int argc, char** argv)
{
	int focusstate, revert, i, nchildren;
	size_t oldtlen=0, oldclen=0, tlen, clen;
	char* wname, * oldtitle=NULL, * newtitle, * oldclass=NULL, * newclass;
	time_t spent=0;
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

		if(focusstate==BadValue||focusstate==BadWindow)
			fprintf(stderr, "XGetInputFocus returned bad value, exiting\n");
		if(revert==None)
			continue;
		if(revert==RevertToParent)
		{
			s3=XQueryTree(dpy, *w, root, par, children, &nchildren);
			w=par;
		}

		s1=XGetTextProperty(dpy, *w, title, XA_WM_NAME);
		s2=XGetClassHint(dpy, *w, class);

		/*
			"If it was able to read and store the data in the XTextProperty
			structure, XGetTextProperty returns a nonzero status; otherwise, it
			returns a zero status."

			–XGetTextProperty(1)

			Isn't this contrary to the Anna Karenina principle?
		*/

		if(focusstate!=BadValue&&focusstate!=BadWindow&&s1!=0&&s2!=0)
		{
			printf("s1: %d, s2: %d\n", s1, s2);
			if(s1!=0)
				printf("%s\n", title->value);
			if(s2!=0)
				printf("%s %s\n", class->res_name, class->res_class);
		}
		sleep(1);
	}

	XCloseDisplay(dpy);

	free(title);
	XFree(class);
	XFree(children);

	return 0;
}
