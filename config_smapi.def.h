/* see LICENCE file for copyright and license information */

/* INTERVAL: interval to check in seconds
   BATT_STATE_FILE: file containing the state "charging" "discharging" etc
   BATT_STATE_(FULL/DISCHARGING/CHARGING): strings to match state from file to
   BATT_PERC_FILE: file containing the remaining battery percentage
   BATT_TIME_REM_(EMPTY/CHARGED)_FILE: file containing time remaining till fully empty/charged
   BATT_CRITICAL_PERC: percentage at which a low battery warning message will be shown 
   BATT_DELAY: optional extra delay before sending, useful since some info isnt available right away
   WLAN_LINK_FILE: file containing wireless info including the link percentage */

/* features left undefined will not be compiled */

/* general */
#define INTERVAL                   2

/* required settings battery */
#define BATT_STATE_FILE            "/sys/devices/platform/smapi/BAT0/state"
#define BATT_STATE_FULL            "idle"
#define BATT_STATE_DISCHARGING     "discharging"
#define BATT_STATE_CHARGING        "charging"

/* optional settings battery */
#define BATT_PERC_FILE             "/sys/devices/platform/smapi/BAT0/remaining_percent"
#define BATT_TIME_REM_EMPTY_FILE   "/sys/devices/platform/smapi/BAT0/remaining_running_time_now"
#define BATT_TIME_REM_CHARGED_FILE "/sys/devices/platform/smapi/BAT0/remaining_charging_time"
#define BATT_CRITICAL_PERC         25
#define BATT_DELAY                 5

/* required settings wlan */
#define WLAN_LINK_FILE             "/proc/net/wireless"
