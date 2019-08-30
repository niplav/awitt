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

void focus_window_info(Display* d, Winformation* wi)
{
	int focusstate, revert;
	unsigned int nchildren;

	Status s=0;

	Window* w=(Window*)calloc(1, sizeof(Window));
	Window* root=(Window*)calloc(1, sizeof(Window));
	Window** children=(Window**)calloc(16, sizeof(Window));

	XTextProperty* title=(XTextProperty*)calloc(1, sizeof(XTextProperty));
	XClassHint* class=(XClassHint*)XAllocClassHint();

	focusstate=XGetInputFocus(d, w, &revert);

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
		Window* par=(Window*)calloc(1, sizeof(Window));
		s=XQueryTree(d, *w, root, par, children, &nchildren);
		free(w);
		w=par;
	}
	if(s==BadWindow)
		w=NULL;

	wi->class[0]='\0';
	wi->title[0]='\0';
	wi->name[0]='\0';

	if(w==NULL)
		goto general_free;

	/*
		"If it was able to read and store the data in the XTextProperty
		structure, XGetTextProperty returns a nonzero status; otherwise, it
		returns a zero status."

		–XGetTextProperty(1)

		Isn't this contrary to the Anna Karenina principle?
	*/

	s=XGetTextProperty(d, *w, title, XA_WM_NAME);

	if(s==0)
		goto title_free;

	strncpy(wi->title, (char*)title->value, title->nitems);
	wi->title[title->nitems]='\0';

	s=XGetClassHint(d, *w, class);

	if(s==0)
		goto class_free;

	/* I don't find any information about whether res_name and
	res_class are null-terminated. I'll just assume it, because it
	works at the moment, but if segfault, look there. */

	strncpy(wi->class, class->res_class, BUFSIZ-1);
	strncpy(wi->name, class->res_name, BUFSIZ-1);

class_free:

	XFree(class->res_class);
	XFree(class->res_name);
	XFree(class);

title_free:

	free(title);

general_free:

	XFree(children);
	free(w);
	free(root);
}

int main(int argc, char** argv)
{
	size_t oldtlen=0, oldclen=0, tlen, clen;
	char * oldtitle=NULL, * oldclass=NULL;

	time_t spent=0;
	Winformation w;
	Display* dpy;

	w.name=(char*)calloc(BUFSIZ, sizeof(char));
	w.class=(char*)calloc(BUFSIZ, sizeof(char));
	w.title=(char*)calloc(BUFSIZ, sizeof(char));

	dpy=XOpenDisplay(NULL);

	if(dpy==NULL)
	{
		fprintf(stderr, "%s: could not open display\n", argv[0]);
		exit(1);
	}

	for(;;)
	{
		sleep(1);

		focus_window_info(dpy, &w);
		tlen=strlen(w.title);
		clen=strlen(w.class);

		/*
			On the occasion that oldtitle has content
			and newtitle is empty OR newtitle is
			now not empty and oldtitle is empty OR
			newtitle and oldtitle are both not empty
			and have different content.
			Same for the class.
		*/

		if(!oldtlen||(!tlen&&oldtlen!=1)||(tlen&&oldtlen==1)||
		  (tlen&&oldtlen!=1&&strncmp(w.title, oldtitle, MIN(tlen, oldtlen-1)))||
		   !oldclen||(!clen&&oldclen!=1)||(clen&&oldclen==1)||
		  (clen&&oldclen!=1&&strncmp(w.class, oldclass, MIN(clen, oldclen-1))))
		{
			if(oldtitle!=NULL&&(oldtlen>1||oldclen>1))
			{
				printf("%ld:%ld:%s:%s\n", time(NULL), spent, oldclass, oldtitle);
				fflush(stdout);
			}

			free(oldtitle);
			oldtitle=calloc(tlen+1, sizeof(char));
			strncpy(oldtitle, w.title, tlen);
			oldtitle[tlen]='\0';
			oldtlen=tlen+1;

			free(oldclass);
			oldclass=calloc(clen+1, sizeof(char));
			strncpy(oldclass, w.class, clen);
			oldclass[clen]='\0';
			oldclen=clen+1;

			spent=0;
		}

		spent++;
	}

	XCloseDisplay(dpy);

	free(w.name);
	free(w.class);
	free(w.title);
	free(oldtitle);
	free(oldclass);

	return 0;
}
