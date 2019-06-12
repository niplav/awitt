/* See LICENSE file for copyright and license details. */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* TODO: isn't sleep() platform independent? */
#include <xcb/xcb.h>

#define MIN(a, b)	((a) < (b) ? (a) : (b))

static xcb_connection_t *conn;

void init_xcb(xcb_connection_t **);
void kill_xcb(xcb_connection_t **);
static xcb_window_t focus_window(void);

void init_xcb(xcb_connection_t **con)
{
	*con = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(*con))
		errx(1, "unable connect to the X server");
}

void kill_xcb(xcb_connection_t **con)
{
	if (*con)
		xcb_disconnect(*con);
}

static xcb_window_t focus_window(void)
{
	xcb_window_t w = 0;
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		errx(1, "xcb_get_input_focus");

	w = r->focus;
	free(r);

	if (w == XCB_NONE || w == XCB_INPUT_FOCUS_POINTER_ROOT)
		errx(1, "focus not set");

	return w;
}

int
main(int argc, char **argv)
{
	size_t oldtlen=0, oldclen=0, tlen, clen;
	char* oldtitle=NULL, * newtitle, * oldclass=NULL, * newclass;
	time_t spent=0;
	xcb_window_t wid=0;
	xcb_get_property_reply_t* nreply, * creply;
	xcb_get_property_cookie_t ncookie, ccookie;

	init_xcb(&conn);

	for(;;)
	{
		wid=focus_window();

		ncookie=xcb_get_property(conn, 0, wid, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 256L);
		nreply=xcb_get_property_reply(conn, ncookie, NULL);

		ccookie=xcb_get_property(conn, 0, wid, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 0, 256L);
		creply=xcb_get_property_reply(conn, ccookie, NULL);

		if(nreply!=NULL&&creply!=NULL)
		{
			tlen=xcb_get_property_value_length(nreply);
			newtitle=(char*)xcb_get_property_value(nreply);

			clen=xcb_get_property_value_length(creply);
			newclass=(char*)xcb_get_property_value(creply);

			/*
				For some reason inexplicable to me right now,
				clen is not reset. This is a hack around that.
			*/

			clen=strnlen(newclass, clen);

			/*
				On the occasion that oldtitle has content
				and newtitle is empty OR newtitle is
				now not empty and oldtitle is empty OR
				newtitle and oldtitle are both not empty
				and have different content.
				Same for the class.
			*/

			if(!oldtlen||(!tlen&&oldtlen!=1)||(tlen&&oldtlen==1)||
			  (tlen&&oldtlen!=1&&strncmp(newtitle, oldtitle, MIN(tlen, oldtlen-1)))||
			   !oldclen||(!clen&&oldclen!=1)||(clen&&oldclen==1)||
			  (clen&&oldclen!=1&&strncmp(newclass, oldclass, MIN(clen, oldclen-1))))
			{
				if(oldtitle!=NULL&&(oldtlen>1||oldclen>1))
				{
					printf("%ld:%ld:%s:%s\n", time(NULL), spent, oldclass, oldtitle);
					fflush(stdout);
				}

				free(oldtitle);
				oldtitle=calloc(tlen+1, sizeof(char));
				strncpy(oldtitle, newtitle, tlen);
				oldtitle[tlen]='\0';
				oldtlen=tlen+1;

				free(oldclass);
				oldclass=calloc(clen+1, sizeof(char));
				strncpy(oldclass, newclass, clen);
				oldclass[clen]='\0';
				oldclen=clen+1;

				spent=0;
			}
		}

		free(nreply);
		free(creply);
		spent++;
		sleep(1);
	}

	free(oldtitle);
	free(oldclass);

	kill_xcb(&conn);
	return 0;
}
