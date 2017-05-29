/* see LICENSE file for copyright and license information */

#include <err.h>
#include <libnotify/notify.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
static void check_batt(char *state_old, char *cflag);
#else
static void check_batt(char *state_old);
#endif
#ifdef BATT_PERC_FILE
#if defined BATT_TIME_REM_EMPTY_FILE &&\
    defined BATT_TIME_REM_CHARGED_FILE
static void get_time(int *time, char *state);
#endif
static void get_perc(int *perc, char *state);
#endif
#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
static void check_crit(int *perc, char *state, char *cflag);
#endif
#ifdef WLAN_LINK_FILE
static void check_wlan(int *link_old);
#endif
static void send_notif(char uflag, char *msg);
static void sighandler(const int signo);

static unsigned short int done;

static void
#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
check_batt(char *state_old, char *cflag)
#else
check_batt(char *state_old)
#endif
{
	FILE *fp;
    char state[12];
	fp = fopen(BATT_STATE_FILE, "r");
	if (fp == NULL) {
		warn("Failed to open file %s", BATT_STATE_FILE);
	}
	fscanf(fp, "%12s", state);
	fclose(fp);

#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
    int percc = 0;
    check_crit(&percc, state, cflag);
#endif 

    if (strcmp(state, state_old)) {
#ifdef BATT_DELAY
        sleep(BATT_DELAY);
#endif

#if defined BATT_TIME_REM_EMPTY_FILE &&\
        defined BATT_TIME_REM_CHARGED_FILE
#define TIME
        int time = 0;
        get_time(&time, state);
#endif
#ifdef BATT_PERC_FILE
#define PERC
        int perc = 0;
        get_perc(&perc, state);
#endif

        char *msg = malloc(sizeof(char) * 100);
        strcpy(msg, "");
        strcat(msg, "Battery ");

        if (!strcmp(state, BATT_STATE_FULL)) {
            strcat(msg, "fully charged");
        } else if (!strcmp(state, BATT_STATE_DISCHARGING)) {
            strcat(msg, "discharging");
        } else if (!strcmp(state, BATT_STATE_CHARGING)) {
            strcat(msg, "charging");
        } else {
            strcat(msg, "status unknown");
        }

        if (strcmp(state, BATT_STATE_FULL)) {
#ifdef PERC
            char *percstr = malloc(sizeof(char) * 6);
            snprintf(percstr, 6, "\n%d%%", perc);
            strcat(msg, percstr);
            free(percstr);
#endif
#ifdef TIME
            char *timestr = malloc(sizeof(char) * 13);
#ifdef PERC
            snprintf(timestr, 13, ", %02d:%02d left", time/60, time%60);
#else
            snprintf(timestr, 13, "\n%02d:%02d left", time/60, time%60);
#endif
            strcat(msg, timestr);
            free(timestr);
#endif
        }
        send_notif(0, msg);
        free(msg);
    }
    strcpy(state_old, state);
}

#if defined BATT_TIME_REM_EMPTY_FILE &&\
    defined BATT_TIME_REM_CHARGED_FILE
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
#endif

#ifdef BATT_PERC_FILE
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
#endif

#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
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

    if(*cflag != 1 && *perc <= BATT_CRITICAL_PERC && strcmp(state, BATT_STATE_CHARGING)) {
        *cflag=1;
        char *critmsg = malloc(sizeof(char) * 40);
        snprintf(critmsg, 40, "Battery low, %d%%\nconnect charger soon", *perc);
        send_notif(1, critmsg);
        free(critmsg);
    } else if (strcmp(state, BATT_STATE_DISCHARGING)) {
        *cflag=0;
    }
}
#endif

#ifdef WLAN_LINK_FILE
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
            send_notif(0, "Lost connection to network\nno connectivity");
        } else if (*link_old == -1) {
            char *linkmsg = malloc(sizeof(char) * 32);
            snprintf(linkmsg, 32, "Connected to network\nlink %d%%", link);
            send_notif(0, linkmsg);
            free(linkmsg);
        }
    }
    *link_old = link;
}
#endif

static void 
send_notif(char uflag, char *msg)
{
    notify_init(msg);
    NotifyNotification *notif = notify_notification_new(NULL, msg, NULL);
    if (uflag) {
        notify_notification_set_urgency(notif, NOTIFY_URGENCY_CRITICAL);
        notify_notification_close(notif, NULL);
    }
    notify_notification_show(notif, NULL);
    g_object_unref(G_OBJECT(notif));
    notify_uninit();
}


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
        printf("sbatt-%s\n", VERSION);
        exit(0);
    } else if (argc == 2 && !strcmp("-d", argv[1])) {
        if (daemon(1, 1) < 0) {
            err(1, "daemon");
        }
    } else if (argc != 1) { 
        printf("usage: snotif [option]\n\
                \noptions:\
                \n\t-d start daemonized\
                \n\t-v print version info and exit\
                \n\t-h print this info and exit");
        exit(0);
    }

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sighandler;
	sigaction(SIGINT,  &act, 0);
	sigaction(SIGTERM, &act, 0);
    
#ifdef BATT_STATE_FILE
    char state_old[12];
#endif
#ifdef WLAN_LINK_FILE
    int link_old = -1;
#endif
#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
    char cflag = 0;
#endif
    while (!done) {
#if defined BATT_STATE_FILE &&\
    defined BATT_STATE_FULL &&\
    defined BATT_STATE_DISCHARGING &&\
    defined BATT_STATE_CHARGING
#if defined BATT_PERC_FILE &&\
    defined BATT_CRITICAL_PERC
        check_batt(state_old, &cflag);
#else
        check_batt(state_old);
#endif
#endif
#ifdef WLAN_LINK_FILE
        check_wlan(&link_old);
#endif
#ifdef INTERVAL
        sleep(INTERVAL);
#endif
    }
    return 0;
}
