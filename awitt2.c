#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

Window* get_focus_window(Display* d)
{
	int focusstate, revert;
	unsigned int nchildren;

	Status s=0;

	Window* w=(Window*)calloc(1, sizeof(Window));
	Window* par=(Window*)calloc(1, sizeof(Window));
	Window* root=(Window*)calloc(1, sizeof(Window));
	Window** children=(Window**)calloc(16, sizeof(Window));

	focusstate=XGetInputFocus(d, w, &revert);

	printf("%d|%d, parent: %d, pointer_root: %d, none: %d\n", focusstate, revert, RevertToParent, RevertToPointerRoot, RevertToNone);

	if(focusstate==BadValue||focusstate==BadWindow)
	{
		fprintf(stderr, "could not find focussed window\n");
		w=NULL;
	}
	if(revert==None)
	{
		fprintf(stderr, "could not find focussed window\n");
		w=NULL;
	}
	if(revert==RevertToParent&&w!=NULL)
	{
		s=XQueryTree(d, *w, root, par, children, &nchildren);
		w=par;
	}

	if(s==BadWindow)
	{
		w=NULL;
	}

	free(par);
	free(root);
	XFree(children);

	return w;
}

int main(int argc, char** argv)
{
	int i;
	size_t oldtlen=0, oldclen=0, tlen, clen;
	char* wname, * oldtitle=NULL, * newtitle, * oldclass=NULL, * newclass;

	time_t spent=0;
	Status s1, s2;
	Window *w=(Window*)calloc(1, sizeof(Window));
	Display* dpy=XOpenDisplay(NULL);
	XTextProperty* title=(XTextProperty*)calloc(1, sizeof(XTextProperty));
	XClassHint* class=(XClassHint*)XAllocClassHint();

	if(dpy==NULL)
	{
		fprintf(stderr, "%s: could not open display\n", argv[0]);
		exit(1);
	}

	for(;;)
	{
		sleep(1);

		w=get_focus_window(dpy);

		if(w==NULL)
			continue;

		s1=XGetTextProperty(dpy, *w, title, XA_WM_NAME);
		s2=XGetClassHint(dpy, *w, class);

		if(s1==0||s2==0)
		{
			free(w);
			continue;
		}

		/*
			"If it was able to read and store the data in the XTextProperty
			structure, XGetTextProperty returns a nonzero status; otherwise, it
			returns a zero status."

			–XGetTextProperty(1)

			Isn't this contrary to the Anna Karenina principle?
		*/

		newtitle=title->value;

		printf("s1: %d, s2: %d\n", s1, s2);
		printf("%s\n", title->value);
		printf("%s | %s\n", class->res_name, class->res_class);

		free(w);
	}

	XCloseDisplay(dpy);

	free(title);
	XFree(class);

	return 0;
}
