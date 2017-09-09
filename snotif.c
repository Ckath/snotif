/* see LICENSE file for copyright and license information */

#include <err.h>
#include <libnotify/notify.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
typedef enum { NORMAL, IMPORTANT } urgency;

/* config parse logic */
#ifdef BATT_PERC_FILE
#define BATT_PERC_SET 1
#else
#define BATT_PERC_FILE ""
#define BATT_PERC_SET 0
#endif

#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
#define BATT_CRIT_SET 1
#else
#define BATT_PERC_FILE ""
#define BATT_CRIT_SET 0
#endif

#if defined BATT_TIME_REM_EMPTY_FILE &&\
    defined BATT_TIME_REM_CHARGED_FILE
#define BATT_TIME_SET 1
#else
#define BATT_TIME_REM_EMPTY_FILE ""
#define BATT_TIME_REM_CHARGED_FILE ""
#define BATT_TIME_SET 0
#endif

#ifdef WLAN_LINK_FILE
#define WLAN_LINK_SET 1
#else
#define WLAN_LINK_FILE ""
#define WLAN_LINK_SET 0
#endif

#ifdef BATT_DELAY
#define BATT_DELAY_SET 1
#else
#define BATT_DELAY 0
#define BATT_DELAY_SET 0
#endif

#ifdef INTERVAL
#define INTERVAL_SET 1
#else
#define INTERVAL 0
#define INTERVAL_SET 0
#endif

static void check_batt(char *state_old, char *cflag);
static void get_time(int *time, char *state);
static void get_perc(int *perc, char *state);
static void check_crit(int *perc, char *state, char *cflag);
static void check_wlan(int *link_old);
static void send_notif(urgency notif_urgency, char *title, char *body);
static void sighandler(const int signo);

static unsigned short int done;

/* main battery check function,
 * if so configured only checks for state,
 * will gather more info if it's configured.
 * includes the critical check as well 
 * only if state changes notification message is prepared and sent */
static void
check_batt(char *state_old, char *cflag)
{
	FILE *fp;
    char state[12];

	fp = fopen(BATT_STATE_FILE, "r");
	if (fp == NULL) {
		warn("Failed to open file %s", BATT_STATE_FILE);
	}
	fscanf(fp, "%11s", state);
	fclose(fp);

    if (BATT_CRIT_SET) {
        int percc = 0;
        check_crit(&percc, state, cflag);
    }

    if (strcmp(state, state_old)) {
        if (BATT_DELAY_SET) {
            sleep(BATT_DELAY);
        } 
        
        int time = 0;
        int perc = 0;
        if (BATT_TIME_SET) {
            get_time(&time, state);
        } if (BATT_PERC_SET) {
            get_perc(&perc, state);
        }

        char title[23];
        char body[78];
        strcpy(title, "");
        strcpy(body, "");
        strcat(title, "Battery ");

        if (!strcmp(state, BATT_STATE_FULL)) {
            strcat(title, "fully charged");
        } else if (!strcmp(state, BATT_STATE_DISCHARGING)) {
            strcat(title, "discharging");
        } else if (!strcmp(state, BATT_STATE_CHARGING)) {
            strcat(title, "charging");
        } else {
            strcat(title, "status unknown");
        }

        if (strcmp(state, BATT_STATE_FULL)) {
            if (BATT_PERC_SET) {
                char percstr[6];
                sprintf(percstr, "%d%%", perc);
                strcat(body, percstr);
            }
            if (BATT_TIME_SET) {
                char timestr[21];
                if (BATT_PERC_SET) {
                    sprintf(timestr, ", %02d:%02d left", time/60, time%60);
                } else {
                    sprintf(timestr, "%02d:%02d left", time/60, time%60);
                }
                strcat(body, timestr);
            }
        }
        send_notif(NORMAL, title, body);
    }
    strcpy(state_old, state);
}

/* read time value from file, only if (dis)charging
 * write this value in the variable */
static void
get_time(int *time, char *state)
{
    if (!strcmp(state, BATT_STATE_DISCHARGING)) {
        FILE *fp;
        fp = fopen(BATT_TIME_REM_EMPTY_FILE, "r");
        if (fp == NULL) {
            warn("Failed to open file %s", BATT_TIME_REM_EMPTY_FILE);
        }
        fscanf(fp, "%d", time);
        fclose(fp);
    } else if (!strcmp(state, BATT_STATE_CHARGING)) {
        FILE *fp;
        fp = fopen(BATT_TIME_REM_CHARGED_FILE, "r");
        if (fp == NULL) {
            warn("Failed to open file %s", BATT_TIME_REM_CHARGED_FILE);
        }
        fscanf(fp, "%d", time);
        fclose(fp);
    }
}

/* get percentage value from file
 * write this value in the variable */
static void
get_perc(int *perc, char *state)
{
	FILE *fp;
    fp = fopen(BATT_PERC_FILE, "r");
    if (fp == NULL) {
        warn("Failed to open file %s", BATT_PERC_FILE);
    }
    fscanf(fp, "%d", perc);
    fclose(fp);
}

/* check if percentage is below defined critical value
 * send warning notification, set cflag to stop spam
 * this flag is reset once you start charging */
static void
check_crit(int *perc, char *state, char *cflag)
{
	FILE *fp;
    fp = fopen(BATT_PERC_FILE, "r");
    if (fp == NULL) {
        warn("Failed to open file %s", BATT_PERC_FILE);
    }
    fscanf(fp, "%d", perc);
    fclose(fp);

    if (!(*cflag) && *perc > 0 && 
            *perc <= BATT_CRITICAL_PERC && 
            strcmp(state, BATT_STATE_CHARGING)) {
        *cflag = 1;
        char critmsg[20];
        sprintf(critmsg, "Battery low, %d%%", *perc);
        send_notif(IMPORTANT, critmsg, "connect charger soon");
    } else if (strcmp(state, BATT_STATE_DISCHARGING)) {
        *cflag = 0;
    }
}

/* check the link from file
 * if link is unknown network assumed to be unreachable
 * send warning notification */
static void
check_wlan(int *link_old)
{
    FILE *fp;
    int link = -1;
    fp = fopen(WLAN_LINK_FILE, "r");
    if (fp == NULL) {
        warn("Failed to open file %s", WLAN_LINK_FILE);
    }
	fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n\
            %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*d\n\
            %*s %*d %d", &link);
	fclose(fp);

    if (link != *link_old) {
        if (link == -1) {
            send_notif(NORMAL, "Lost connection to network", "no connectivity");
        } else if (*link_old == -1) {
            char linkmsg[12];
            sprintf(linkmsg, "link %d%%", link);
            send_notif(NORMAL, "Connected to network", linkmsg);
        }
    }
    *link_old = link;
}

/* simple libnotify wrapper function to easily send notifications */
static void 
send_notif(urgency notif_urgency, char *title, char *body)
{
    notify_init(title);
    NotifyNotification *notif = notify_notification_new(title, body, NULL);
    if (notif_urgency == IMPORTANT) {
        notify_notification_set_urgency(notif, NOTIFY_URGENCY_CRITICAL);
        notify_notification_close(notif, NULL);
    }
    notify_notification_show(notif, NULL);
    g_object_unref(G_OBJECT(notif));
    notify_uninit();
}

/* properly quit when asked to */
static void
sighandler(const int signo)
{
	if (signo == SIGTERM || signo == SIGINT) {
		done = 1;
	}
}

int
main(int argc, char *argv[]) 
{
    if (argc == 2 && !strcmp("-v", argv[1])) {
        printf("snotif-%s\n", VERSION);
        exit(0);
    } else if (argc == 2 && !strcmp("-d", argv[1])) {
        if (daemon(1, 1) < 0) {
            err(1, "daemon");
        }
    } else if (argc != 1) { 
        fprintf(stderr, "usage: snotif [option]\n"
                "options:\n"
                "  -d start daemonized\n"
                "  -v print version info and exit\n"
                "  -h print this info and exit\n");
        exit(1);
    }

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;
	sigaction(SIGINT,  &act, 0);
	sigaction(SIGTERM, &act, 0);
    
    char state_old[12];
    int link_old = -1;
    char cflag = 0;

    while (!done) {
        check_batt(state_old, &cflag);
        if (WLAN_LINK_SET) {
            check_wlan(&link_old);
        }

        if (INTERVAL_SET) {
            sleep(INTERVAL);
        }
    }
    return 0;
}
