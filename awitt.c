#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <getopt.h>
#include <time.h>
#include <xcb/xcb.h>

#include "arg.h"

static xcb_connection_t *conn;
static xcb_screen_t *scr;
static uint32_t mask = XCB_EVENT_MASK_NO_EVENT
                     | XCB_EVENT_MASK_FOCUS_CHANGE
		     | XCB_EVENT_MASK_ENTER_WINDOW
                     ;

void init_xcb(xcb_connection_t **);
void kill_xcb(xcb_connection_t **);
void get_screen(xcb_connection_t*, xcb_screen_t**);
int get_windows(xcb_connection_t*, xcb_window_t, xcb_window_t**);
void usage(char*);
void list_events(void);
void handle_events(void);
void register_events(xcb_window_t, uint32_t);

void
init_xcb(xcb_connection_t **con)
{
	*con = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(*con))
		errx(1, "unable connect to the X server");
}

void
kill_xcb(xcb_connection_t **con)
{
	if (*con)
		xcb_disconnect(*con);
}

void
get_screen(xcb_connection_t *con, xcb_screen_t **scr)
{
	*scr = xcb_setup_roots_iterator(xcb_get_setup(con)).data;
	if (*scr == NULL)
		errx(1, "unable to retrieve screen informations");
}

int
get_windows(xcb_connection_t *con, xcb_window_t w, xcb_window_t **l)
{
	int childnum = 0;
	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(con, w);
	r = xcb_query_tree_reply(con, c, NULL);
	if (r == NULL)
		errx(1, "0x%08x: no such window", w);

	*l = xcb_query_tree_children(r);

	childnum = r->children_len;
	free(r);

	return childnum;
}

void
usage(char *name)
{
	fprintf(stderr, "usage: %s [-l] [-m <mask>]\n", name);
	exit(1);
}

void
register_events(xcb_window_t w, uint32_t m)
{
	uint32_t val[] = { m };

	xcb_change_window_attributes(conn, w, XCB_CW_EVENT_MASK, val);
	xcb_flush(conn);
}

void
handle_events(void)
{
	int i, wn;
	xcb_window_t *wc, wid = 0;
	xcb_generic_event_t *e;
	xcb_create_notify_event_t *ec;
	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *reply;
	xcb_atom_t property=XCB_ATOM_WM_NAME;
	xcb_atom_t type=XCB_ATOM_STRING;

	/*
	 * We need to get notifed of window creations, no matter what, because
	 * we need to register the event mask on all newly created windows
	 */
	register_events(scr->root, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

	if (mask & XCB_EVENT_MASK_BUTTON_PRESS) {
		xcb_grab_button(conn, 0, scr->root,
		                XCB_EVENT_MASK_BUTTON_PRESS |
		                XCB_EVENT_MASK_BUTTON_RELEASE,
		                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
		                scr->root, XCB_NONE, 1, XCB_NONE);
	}

	/* register the events on all mapped windows */
	wn = get_windows(conn, scr->root, &wc);
	for (i=0; i<wn; i++)
		register_events(wc[i], mask);

	xcb_flush(conn);

	for(;;) {
		e = xcb_wait_for_event(conn);

		switch (e->response_type & ~0x80)
		{
			/* Find a way to fix focusing */
			/*
			case XCB_FOCUS_IN:
				wid = ((xcb_focus_in_event_t*)e)->event;
				break;
			*/
			case XCB_ENTER_NOTIFY:
				wid=((xcb_enter_notify_event_t*)e)->event;
				break;
			case XCB_LEAVE_NOTIFY:
				wid=((xcb_leave_notify_event_t*)e)->event;
				break;
			default:
				wid=0x0;
				break;
		}

		if (wid > 0) {
			cookie=xcb_get_property(conn, 0, wid, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 32);
			if((reply=xcb_get_property_reply(conn, cookie, NULL)))
			{
				int len=xcb_get_property_value_length(reply);
				if(len!=0)
					printf("%ld:%.*s\n", time(NULL), len, (char*)xcb_get_property_value(reply));
				free(reply);
			}
			fflush(stdout);
		}

		free(e);
	}
}

int
main (int argc, char **argv)
{
	init_xcb(&conn);
	get_screen(conn, &scr);

	handle_events();

	kill_xcb(&conn);

	return 0;
}
