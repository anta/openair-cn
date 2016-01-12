/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under 
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*****************************************************************************
  Source    nas_log.c

  Version   0.1

  Date    2012/02/28

  Product   NAS stack

  Subsystem Utilities

  Author    Frederic Maurel

  Description Usefull logging functions

*****************************************************************************/

#include "nas_log.h"
#include <stdio.h>            // stderr, sprintf, fprintf, vfprintf
#include <stdarg.h>           // va_list, va_start, va_end
#include <string.h>           // strlen
#include <sys/time.h>

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/* ANSI escape codes for colored display */
#  define LOG_BLACK "\033[30m"
#  define LOG_RED   "\033[31m"
#  define LOG_GREEN "\033[32m"
#  define LOG_YELLOW  "\033[33m"
#  define LOG_BLUE  "\033[34m"
#  define LOG_MAGENTA "\033[35m"
#  define LOG_CYAN  "\033[36m"
#  define LOG_WHITE "\033[37m"
#  define LOG_END   "\033[0m"
#  define LOG_AUTO  LOG_END

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/* ------------------------
   Internal logging context
   ------------------------
    Internal logging context consists on:
        - The file name and the line number from where the data have been
          logged. These information are gathered into a string that will
          be displayed as a prefix of the logging trace with the format
          filename[line]
        - The severity level filter
        - The indentation level to convey FUNC logging traces
        - The data definition of each logging trace level: name and mask
          (the mask is used against the severity level filter to enable
          or disable specific logging traces)
*/
typedef struct {
#  define LOG_PREFIX_SIZE 118
  char                                    prefix[LOG_PREFIX_SIZE];
  unsigned char                           filter;
  int                                     indent;
  const struct {
    char                                   *name;
    unsigned char                           mask;
    char                                   *color;
  } level[];
} log_context_t;

/*
   Definition of the logging context
*/
static log_context_t                    _log_context = {
  "",                           /* prefix */
  0x00,                         /* filter */
  0,                            /* indent */
  {
   {"DEBUG", NAS_LOG_DEBUG, LOG_GREEN}, /* DEBUG  */
   {"INFO", NAS_LOG_INFO, LOG_AUTO},    /* INFO   */
   {"WARNING", NAS_LOG_WARNING, LOG_BLUE},      /* WARNING  */
   {"ERROR", NAS_LOG_ERROR, LOG_RED},   /* ERROR  */
   {"", NAS_LOG_FUNC, LOG_AUTO},        /* FUNC_IN  */
   {"", NAS_LOG_FUNC, LOG_AUTO},        /* FUNC_OUT */
   }                            /* level[]  */
};

/* Maximum number of bytes into a line of dump logging data */
#  define LOG_DUMP_LINE_SIZE  16

int                                     g_nas_log_start_time_second = 0;

static void nas_log_get_elapsed_time_since_start(struct timeval * const elapsed_time)
{
  // no thread safe but do not matter a lot
  gettimeofday(elapsed_time, NULL);
  // no timersub call for fastest operations
  elapsed_time->tv_sec = elapsed_time->tv_sec - g_nas_log_start_time_second;
}

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:  log_init()                                                **
 **                                                                        **
 ** Description: Initializes internal logging data                         **
 **                                                                        **
 ** Inputs:  filter:  Value of the severity level that will be   **
 **       used as a filter to enable or disable      **
 **       specific logging traces                    **
 **      Others:  None                                       **
 **                                                                        **
 ** Outputs:   Return:  None                                       **
 **      Others:  None                                       **
 **                                                                        **
 ***************************************************************************/
void
nas_log_init (
  char filter)
{
#if MESSAGE_CHART_GENERATOR
  extern int                                     g_msc_start_time_second;
  g_nas_log_start_time_second = g_msc_start_time_second;
#else
    struct timeval                          start_time = {.tv_sec=0, .tv_usec=0};

    gettimeofday(&start_time, NULL);
    g_nas_log_start_time_second = start_time.tv_sec;
#endif
  _log_context.filter = filter;
}

/****************************************************************************
 **                                                                        **
 ** Name:  log_data()                                                **
 **                                                                        **
 ** Description: Defines internal logging data                             **
 **                                                                        **
 ** Inputs:  filename:  Name of the file from where the data have  **
 **         been logged                                **
 **      line:    Number of the line in the file             **
 **      Others:  None                                       **
 **                                                                        **
 ** Outputs:   Return:  None                                       **
 **      Others:  None                                       **
 **                                                                        **
 ***************************************************************************/
void
log_data (
  const char *filename,
  int line)
{
  int                                     len = strlen (filename) + 4+1+6+1+2 + 1;      // 4+1+6+1:"%04ld:%06ld|" , 2:[], 1:\0
  struct timeval                          elapsed_time = {.tv_sec = 0, .tv_usec = 0};

  nas_log_get_elapsed_time_since_start(&elapsed_time);

  if (line > 9999)
    len += 5;
  else if (line > 999)
    len += 4;
  else if (line > 99)
    len += 3;
  else if (line > 9)
    len += 2;
  else
    len += 1;

  if (len > LOG_PREFIX_SIZE) {
    snprintf (_log_context.prefix, LOG_PREFIX_SIZE , "%04ld:%06ld|%s:%d", elapsed_time.tv_sec, elapsed_time.tv_usec, &filename[len - LOG_PREFIX_SIZE], line);
  } else {
    snprintf (_log_context.prefix, LOG_PREFIX_SIZE , "%04ld:%06ld|%s:%d", elapsed_time.tv_sec, elapsed_time.tv_usec, filename, line);
  }
}

/****************************************************************************
 **                                                                        **
 ** Name:  log_trace()                                               **
 **                                                                        **
 ** Description: Displays logging data                                     **
 **                                                                        **
 ** Inputs:  severity:  Severity level of the logging data         **
 **      data:    Formated logging data to display           **
 **      Others:  None                                       **
 **                                                                        **
 ** Outputs:   Return:  None                                       **
 **      Others:  None                                       **
 **                                                                        **
 ***************************************************************************/
void
log_trace (
  log_severity_t severity,
  const char *data,
  ...)
{
  int                                     i;

  /*
   * Sanity check
   */
  if (severity > LOG_SEVERITY_MAX)
    return;

  /*
   * Display only authorized logging traces
   */
  if (_log_context.level[severity].mask & _log_context.filter) {
    va_list                                 argp;

    /*
     * First, display internal logging data (logging trace prefix: file
     * name and line number from where the data have been logged) and
     * the severity level.
     */
    fprintf (stderr, "%s%-120.118s%-10s", _log_context.level[severity].color, _log_context.prefix, _log_context.level[severity].name);
    {
      /*
       * Next, perform indentation for FUNC logging trace
       */
      if (severity == FUNC_OUT) {
        _log_context.indent--;
      }

      for (i = 0; i < _log_context.indent; i++) {
        fprintf (stderr, "  ");
      }

      if (severity == FUNC_IN) {
        _log_context.indent++;
      }
    }
    /*
     * Finally, display logging data
     */
    va_start (argp, data);
    vfprintf (stderr, data, argp);
    /*
     * Terminate with line feed character
     */
    fprintf (stderr, "%s\n", LOG_END);
    va_end (argp);
  }
}

/****************************************************************************
 **                                                                        **
 ** Name:  log_dump()                                                **
 **                                                                        **
 ** Description: Dump logging data                                         **
 **                                                                        **
 ** Inputs:  data:    Logging data to dump                       **
 **      len:   Number of bytes to be dumped               **
 **      Others:  None                                       **
 **                                                                        **
 ** Outputs:   Return:  None                                       **
 **      Others:  None                                       **
 **                                                                        **
 ***************************************************************************/
void
log_dump (
  const char *data,
  int len)
{
  int                                     i;

  /*
   * Display only authorized logging traces
   */
  if ((len > 0) && (NAS_LOG_HEX & _log_context.filter)) {
    int                                     bytes = 0;

    fprintf (stderr, "\n\t");

    for (i = 0; i < len; i++) {
      fprintf (stderr, "%.2hx ", (const unsigned char)data[i]);

      /*
       * Add new line when the number of displayed bytes exceeds
       * * * * the line's size
       */
      if (++bytes > (LOG_DUMP_LINE_SIZE - 1)) {
        bytes = 0;
        fprintf (stderr, "\n\t");
      }
    }

    if (bytes % LOG_DUMP_LINE_SIZE) {
      fprintf (stderr, "\n");
    }

    fprintf (stderr, "\n");
    fflush (stderr);
  }
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/
