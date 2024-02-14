#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct Winformation
{
	char* title;
	char* class;
	char* name;
} Winformation;

void focus_window_info(Display* display, Winformation* window_info)
{
	int focusstate, revert;
	unsigned int nchildren;

	Status title_status=0;

	Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);

	Window* window=(Window*)calloc(1, sizeof(Window));
	Window* root=(Window*)calloc(1, sizeof(Window));
	Window* children=NULL;

	XTextProperty* title=(XTextProperty*)calloc(1, sizeof(XTextProperty));
	XClassHint* class=(XClassHint*)XAllocClassHint();

	if(window==NULL||root==NULL||title==NULL||class==NULL)
	{
		fprintf(stderr, "could not allocate memory, exiting\n");
		exit(2);
	}

	focusstate=XGetInputFocus(display, window, &revert);

	if(focusstate==BadValue||focusstate==BadWindow||revert==None)
		window=NULL;
	/*
		TODO: What if this means that we should go up the tree
		to find the focussed window and not just one step up
		to the parent? Investigate.
	*/
	if(revert==RevertToParent&&window!=NULL)
	{
		Window* par=(Window*)calloc(1, sizeof(Window));

		if(par==NULL)
		{
			fprintf(stderr, "could not allocate memory, exiting\n");
			exit(2);
		}

		title_status=XQueryTree(display, *window, root, par, &children, &nchildren);

		if(children!=NULL)
			XFree(children);

		free(window);
		window=par;
	}
	if(title_status==BadWindow)
		window=NULL;

	window_info->class[0]='\0';
	window_info->title[0]='\0';
	window_info->name[0]='\0';

	if(window==NULL)
		goto general_free;

	/*
		"If it was able to read and store the data in the XTextProperty
		structure, XGetTextProperty returns a nonzero status; otherwise, it
		returns a zero status."

		–XGetTextProperty(1)

		Isn't this contrary to the Anna Karenina principle?
	*/

	title_status=XGetTextProperty(display, *window, title, net_wm_name);

	if(title_status==0||title->value==NULL)
		goto general_free;

	strncpy(window_info->title, (char*)title->value, title->nitems);
	window_info->title[title->nitems]='\0';

	title_status=XGetClassHint(display, *window, class);

	if(title_status==0)
		goto class_free;

	/* I couldn't find any information about whether res_name and
	res_class are null-terminated. I'll just assume it, because it
	works at the moment, but if segfault, look there. */

	strncpy(window_info->class, class->res_class, BUFSIZ-1);
	strncpy(window_info->name, class->res_name, BUFSIZ-1);

class_free:
	XFree(class->res_class);
	XFree(class->res_name);

general_free:

	XFree(class);
	free(title);
	free(window);
	free(root);
}

int main(int argc, char** argv)
{
	size_t oldtlen=0, oldclen=0, tlen, clen;
	char * oldtitle=NULL, * oldclass=NULL;

	time_t spent=0;
	Winformation window;
	Display* dpy;

	window.name=(char*)calloc(BUFSIZ, sizeof(char));
	window.class=(char*)calloc(BUFSIZ, sizeof(char));
	window.title=(char*)calloc(BUFSIZ, sizeof(char));

	dpy=XOpenDisplay(NULL);

	if(dpy==NULL)
	{
		fprintf(stderr, "%s: could not open display\n", argv[0]);
		exit(1);
	}

	for(;;)
	{
		sleep(1);

		focus_window_info(dpy, &window);
		tlen=strlen(window.title);
		clen=strlen(window.class);

		/*
			On the occasion that oldtitle has content and
			newtitle is empty OR newtitle is now not empty
			and oldtitle is empty OR newtitle and oldtitle
			are both not empty and have different content.
			Same for the class.
		*/

		if(!oldtlen||(!tlen&&oldtlen!=1)||(tlen&&oldtlen==1)||
		  (tlen&&oldtlen!=1&&strncmp(window.title, oldtitle, MIN(tlen, oldtlen-1)))||
		   !oldclen||(!clen&&oldclen!=1)||(clen&&oldclen==1)||
		  (clen&&oldclen!=1&&strncmp(window.class, oldclass, MIN(clen, oldclen-1))))
		{
			if(oldtitle!=NULL&&(oldtlen>1||oldclen>1))
			{
				printf("%ld:%ld:%s:%s\n", time(NULL), spent, oldclass, oldtitle);
				fflush(stdout);
			}

			free(oldtitle);
			oldtitle=calloc(tlen+1, sizeof(char));
			strncpy(oldtitle, window.title, tlen);
			oldtitle[tlen]='\0';
			oldtlen=tlen+1;

			free(oldclass);
			oldclass=calloc(clen+1, sizeof(char));
			strncpy(oldclass, window.class, clen);
			oldclass[clen]='\0';
			oldclen=clen+1;

			spent=0;
		}

		spent++;
	}

	XCloseDisplay(dpy);

	free(window.name);
	free(window.class);
	free(window.title);
	free(oldtitle);
	free(oldclass);

	return 0;
}
