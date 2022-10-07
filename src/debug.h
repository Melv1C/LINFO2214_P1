//
// Created by melvyn on 7/10/22.
//

#ifndef PROJET1_DEBUG_H
#define PROJET1_DEBUG_H

#endif //PROJET1_DEBUG_H

#include <errno.h>
#ifdef _COLOR
/* Want more/other colors? See https://stackoverflow.com/a/3219471 and
 * https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#else
#define ANSI_COLOR_BRIGHT_RED
#define ANSI_COLOR_CYAN
#define ANSI_COLOR_RESET
#endif

#define _LOG(color, prefix, msg, ...)\
    do {\
        fprintf(stderr, color prefix msg ANSI_COLOR_RESET "\n", ##__VA_ARGS__);\
    } while (0)

#define ERROR(msg, ...) _LOG(ANSI_COLOR_BRIGHT_RED, "[ERROR] ", msg, ##__VA_ARGS__)

#ifdef _DEBUG
#define DEBUG(msg, ...) _LOG(ANSI_COLOR_CYAN, "[DEBUG] ", msg, ##__VA_ARGS__)
#else
#define DEBUG(msg, ...)
#endif
