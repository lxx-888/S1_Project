/*
 * KeyEventContext.c
 *
 *  Created on: Feb 16, 2020
 *      Author: guoxs
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <linux/input.h>

#include "KeyEventContext.h"

#define DEV_INPUT_NODE		"/dev/input/event0"

static int sWakeFds[2] = { -1, -1 };
static pthread_t thread = 0;
static on_key_event_callback s_callback;
static int last_code = 0;
static bool last_code_lpress = false;

static void* _event_Loop(void *user) {
	int kbd = open(DEV_INPUT_NODE, O_RDONLY);
	if (kbd < 0) {
		printf("open %s fail!\n", DEV_INPUT_NODE);
		return NULL;
	}

	fd_set fdset;
	FD_ZERO(&fdset);

	while (1) {
		FD_SET(kbd, &fdset);
		FD_SET(sWakeFds[0], &fdset);
		if (select(FD_SETSIZE, &fdset, NULL, NULL, NULL) > 0) {
			if (FD_ISSET(kbd, &fdset)) {
				FD_CLR(kbd, &fdset);
				struct input_event event;
				if (read(kbd, &event, sizeof(event)) == sizeof(event) &&
					event.type == EV_KEY &&
					s_callback) {
					// printf("event code: %d, value: %d\n", event.code, event.value);
					// 和 set_key_event_callback 最好加一下锁，再操作
					if (event.value == UI_KEY_PRESS) {
						last_code = event.code;
						last_code_lpress = false;
					} else if (event.value == UI_KEY_LPRESS) {
						last_code = event.code;
						last_code_lpress = true;
					} else if (event.value == UI_KEY_RELEASE) {
						if (event.code == last_code &&
							last_code_lpress)
							event.value = UI_KEY_LRELEASE;
					}
					s_callback(event.code, event.value);
				}
			}

			if (FD_ISSET(sWakeFds[0], &fdset)) {
				printf("wait_event wake up ...\n");
				close(sWakeFds[0]);
				sWakeFds[0] = -1;
				break;
			}
		}
	}

	close(kbd);
	printf("_event_Loop exit...\n");
	return NULL;
}

int start_key_event_ctx() {
	if (pipe(sWakeFds) < 0) {
		printf("Create pipe error!\n");
		return -1;
	}

	int ret = pthread_create(&thread, NULL, _event_Loop, NULL);

	if (ret || !thread) {
		printf("pthread_create error : %s\n", strerror(errno));
		return -1;
	} else {
		pthread_setname_np(thread, "keypad");
	}

	return 0;
}

int stop_key_event_ctx() {
	if (sWakeFds[1] >= 0) {
		printf("Want to wake up wait_event...\n");

		ssize_t nWrite;
		do {
			nWrite = write(sWakeFds[1], "W", 1);
		} while ((nWrite == -1) && (errno == EINTR));

		pthread_join(thread, NULL);

		close(sWakeFds[1]);
		sWakeFds[1] = -1;
	}

	return 0;
}

void set_key_event_callback(on_key_event_callback cb) {
	s_callback = cb;
}