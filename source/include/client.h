//
// Created by justinsaint on 09/10/24.
//

#ifndef CLIENT_H
#define CLIENT_H

#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FIFO_IN "./fifo_in"
#define FIFO_OUT "./fifo_out"
#define BUFFER_SIZE 1024
#define FILTER_SIZE 64
#define ARGS_NUM 5

static void parse_arguments(int argc, char *argv[], char **input_string, char filter[FILTER_SIZE]);
static int  handle_arguments(const char *binary_name, const char *input_string, const char *filter);
static void usage(const char *program_name, const char *message);

#endif    // CLIENT_H
