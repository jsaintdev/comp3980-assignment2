//
// Created by justinsaint on 09/10/24.
//

#ifndef SERVER_H
#define SERVER_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_IN "./fifo_in"
#define FIFO_OUT "./fifo_out"
#define BUFFER_SIZE 1024
#define FILTER_SIZE 64
#define EXIT_CODE 1

static volatile sig_atomic_t exit_flag = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Function declarations
void *read_string(const void *arg);
void *read_string_wrapper(void *arg);
void  process_string(char *str, const char *filter);
void  setup_signal_handler(void);
void  signal_handler(int signal_number, siginfo_t *info, void *context);

#endif    // SERVER_H
