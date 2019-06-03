/* See LICENSE file for copyright and license details. */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* TODO: isn't this platform independent? */
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
	size_t oldlen=0, len;
	char* oldtitle=NULL, * newtitle;
	time_t spent=0;
	xcb_window_t wid=0;
	xcb_get_property_reply_t* reply;
	xcb_get_property_cookie_t cookie;

	init_xcb(&conn);

	for(;;)
	{
		wid=focus_window();
		cookie=xcb_get_property(conn, 0, wid, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 32L);
		reply=xcb_get_property_reply(conn, cookie, NULL);

		if(reply!=NULL)
		{
			len=xcb_get_property_value_length(reply);
			newtitle=(char*)xcb_get_property_value(reply);

			/*
				On the occasion that oldtitle has content
				and newtitle is empty OR newtitle is
				now not empty and oldtitle is empty OR
				newtitle and oldtitle are both not empty
				and have different content
			*/

			if(!oldlen||(!len&&oldlen!=1)||(len&&oldlen==1)||
			  (len&&oldlen!=1&&strncmp(newtitle, oldtitle, MIN(len+1, oldlen))))
			{
				if(oldtitle!=NULL)
					printf("%ld:%ld:%s\n", time(NULL), spent, oldtitle);

				free(oldtitle);
				oldtitle=calloc(len+1, sizeof(char));
				strncpy(oldtitle, newtitle, len);
				oldtitle[len]='\0';
				oldlen=len+1;
				spent=0;
			}
		}

		free(reply);
		spent++;
		sleep(1);
	}

	free(oldtitle);

	kill_xcb(&conn);
	return 0;
}
