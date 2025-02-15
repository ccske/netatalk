#ifndef _ATALK_LOGGER_H
#define _ATALK_LOGGER_H 1

/*
 * logger LOG Macro Usage
 * ======================
 *
 * LOG(<logtype>, <loglevel>, "<string>"[, args]);
 *
 *
 * logger API Setup
 * ================
 *
 * Standard interface:
 * -------------------
 *
 *    setuplog(char *confstring)
 *    confstring = "<logtype> <loglevel> [<filename>]"
 *
 * Calling without <filename> configures basic logging to syslog. Specifying <filename>
 * configures extended logging to <filename>.
 *
 * You can later disable logging by calling
 *
 *    unsetuplog(char *confstring)
 *    confstring = "<logtype> [<any_string>]"
 *
 * Calling without <any_string> disables syslog logging, calling with <any_string>
 * disables file logging.
 *
 * <logtype>:
 * you can setup a default with "Default". Any other logtype used in LOG will then
 * use the default if not setup itself. This is probabyl the only thing you may
 * want to use.
 *
 * Example:
 * setuplog("default log_debug /var/log/debug.log");
 * See also libatalk/util/test/logger_test.c
 *
 * "Legacy" interface:
 * -------------------
 *
 * Some netatalk daemons (31.3.2009.: e.g. atalkd, papd) may not be converted to
 * use the new API and might still call
 *
 *    syslog_setup(int loglevel, enum logtypes logtype, int display_options, int facility);
 *
 * directly. These daemons are therefore limited to syslog logging. Also their
 * loglevel can't be changed at runtime.
 *
 *
 * Note:
 * don't get confused by log_init(). It only gets called if your app
 * forgets to setup logging before calling LOG.
 */


#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* logger is used by pam modules */
#ifndef UAM_MODULE_EXPORT
#define UAM_MODULE_EXPORT
#endif

enum loglevels {
    log_none,
    log_severe,
    log_error,
    log_warning,
    log_note,
    log_info,
    log_debug,
    log_debug6,
    log_debug7,
    log_debug8,
    log_debug9,
    log_maxdebug
};

/* this is the enum specifying all availiable logtypes */
enum logtypes {
  logtype_default,
  logtype_logger,
  logtype_cnid,
  logtype_afpd,
  logtype_dsi,
  logtype_uams,
  logtype_fce,
  logtype_ad,
  logtype_sl,
  logtype_end_of_list_marker  /* don't put any logtypes after this */
};


/* Display Option flags. */
/* redefine these so they can don't interfeer with syslog */
/* these can be used in standard logging too */
#define logoption_nsrcinfo    0x04   /* don't log source info */
/* the following do not work anymore, they're only provided in order to not
 * break existing source code */
#define logoption_pid         0x01   /* log the pid with each message */
#define logoption_cons        0x02   /* log on the console if error logging */
#define logoption_ndelay      0x08   /* don't delay open */
#define logoption_perror      0x20   /* log to stderr as well */
#define logoption_nfile       0x40   /* ignore the file that called the log */
#define logoption_nline       0x80   /* ignore the line that called the log*/

/* facility codes */
/* redefine these so they can don't interfeer with syslog */
#define logfacility_user        (1<<3)  /* random user-level messages */
#define logfacility_mail        (2<<3)  /* mail system */
#define logfacility_daemon      (3<<3)  /* system daemons */
#define logfacility_auth        (4<<3)  /* security/authorization messages */
#define logfacility_syslog      (5<<3)  /* messages generated by syslogd */
#define logfacility_lpr         (6<<3)  /* line printer subsystem */
#define logfacility_authpriv    (10<<3) /* security/auth messages (private) */
#define logfacility_ftp         (11<<3) /* ftp daemon */

/* =========================================================================
    Structure definitions
   ========================================================================= */

/* Main log config */
typedef struct {
    bool           inited;                 /* file log config initialized ? */
    bool           syslog_opened;          /* syslog opened ? */
    bool           console;                /* if logging to console from a cli util */
    char           processname[16];
    int            syslog_facility;
    int            syslog_display_options;
} log_config_t;

/* This stores the config and options for one filelog type (e.g. logger, afpd etc.) */
typedef struct {
    bool           set;           /* set individually ? yes: changing default
			                       * doesnt change it. no: it changes it.*/
    bool           syslog;        /* This type logs to syslog */
    int            fd;            /* logfiles fd */
    enum loglevels level;         /* Log Level to put in this file */
    int            display_options;
} logtype_conf_t;


/* =========================================================================
    Global variables
    ========================================================================= */

/* Make config accessible for LOG macro */
extern log_config_t log_config;

extern UAM_MODULE_EXPORT logtype_conf_t type_configs[logtype_end_of_list_marker];

/* =========================================================================
    Global function decarations
   ========================================================================= */

void setuplog(const char *loglevel, const char *logfile);
void set_processname(const char *processname);

/* LOG macro func no.1: log the message to file */
UAM_MODULE_EXPORT  void make_log_entry(enum loglevels loglevel, enum logtypes logtype, const char *file, int line, char *message, ...);

/*
 * How to write a LOG macro:
 * http://c-faq.com/cpp/debugmacs.html
 *
 * We choose the verbose form in favor of the obfuscated ones, its easier
 * to parse for human beings and facilitates expanding the macro for
 * inline checks for debug levels.
 */

#define LOG_MAX log_info

#ifdef NO_DEBUG

#define LOG(log_level, type, ...)                                       \
    do {                                                                \
        if (log_level <= LOG_MAX)                                       \
            if (log_level <= type_configs[type].level)                  \
                make_log_entry((log_level), (type), __FILE__, __LINE__,  __VA_ARGS__); \
    } while(0)

#else  /* ! NO_DEBUG */

#define LOG(log_level, type, ...)               \
    do {                                                                \
        if (log_level <= type_configs[type].level)                      \
            make_log_entry((log_level), (type), __FILE__, __LINE__,  __VA_ARGS__); \
    } while(0)

#endif  /* NO_DEBUG */

#endif /* _ATALK_LOGGER_H */
