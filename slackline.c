#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "slackline.h"

struct slackline *
sl_init(void)
{
	struct slackline *sl = malloc(sizeof *sl);

	if (sl == NULL)
		return NULL;

	sl->bufsize = BUFSIZ;
	if ((sl->buf = malloc(sl->bufsize)) == NULL) {
		free(sl);
		return NULL;
	}

	sl_reset(sl);

	return sl;
}

void
sl_free(struct slackline *sl)
{
	free(sl->buf);
	free(sl);
}

void
sl_reset(struct slackline *sl)
{
	sl->buf[0] = '\0';
	sl->len = 0;
	sl->cur = 0;
	sl->esc = ESC_NONE;
}

int
sl_keystroke(struct slackline *sl, int key)
{
	if (sl == NULL || sl->len < sl->cur)
		return -1;

	/* handle escape sequences */
	switch (sl->esc) {
	case ESC_NONE:
		break;
	case ESC:
		sl->esc = key == '[' ? ESC_BRACKET : ESC_NONE;
		return 0;
	case ESC_BRACKET:
		switch (key) {
		case 'A':	/* up    */
		case 'B':	/* down  */
			break;
		case 'C':	/* right */
			if (sl->cur < sl->len)
				sl->cur++;
			break;
		case 'D':	/* left */
			if (sl->cur > 0)
				sl->cur--;
			break;
		case 'H':	/* Home  */
			sl->cur = 0;
			break;
		case 'F':	/* End   */
			sl->cur = sl->len;
			break;
		}
		sl->esc = ESC_NONE;
		return 0;
	}

	/* add character to buffer */
	if (key >= 32 && key < 127) {
		if (sl->cur < sl->len) {
			memmove(sl->buf + sl->cur + 1, sl->buf + sl->cur,
			    sl->len - sl->cur);
			sl->buf[sl->cur++] = key;
		} else {
			sl->buf[sl->cur++] = key;
			sl->buf[sl->cur] = '\0';
		}
		sl->len++;
		return 0;
	}

	/* handle ctl keys */
	switch (key) {
	case 27:	/* Escape */
		sl->esc = ESC;
		break;
	case 127:	/* backspace */
	case 8:		/* backspace */
		if (sl->cur == 0)
			break;
		if (sl->cur < sl->len)
			memmove(sl->buf + sl->cur - 1, sl->buf + sl->cur,
			    sl->len - sl->cur);
		sl->cur--;
		sl->len--;
		sl->buf[sl->len] = '\0';
		break;
	}

	return 0;
}
