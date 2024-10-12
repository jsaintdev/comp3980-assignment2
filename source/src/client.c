#include "client.h"

int main(int argc, char *argv[])
{
    // Program variables
    char   *input_string = NULL;
    char    filter[FILTER_SIZE];
    char    buffer[BUFFER_SIZE];
    int     valid_args;
    int     fd_in;
    int     fd_out;
    ssize_t bytes_written;
    ssize_t bytes_read;
    int     retval = 0;

    // Parse and Handle Command Line Arguments
    parse_arguments(argc, argv, &input_string, filter);

    if(argc > ARGS_NUM)
    {
        usage(argv[0], "Error: Too many arguments");
        retval = -1;
        goto done;
    }

    if(input_string == NULL)
    {
        usage(argv[0], "Error: No user string entered");
        retval = -1;
        goto done;
    }

    valid_args = handle_arguments(input_string, filter);

    if(valid_args == -1)
    {
        usage(argv[0], "Error: Invalid arguments");
        retval = -1;
        goto done;
    }

    // Open input FIFO for writing
    fd_in = open(FIFO_IN, O_WRONLY | O_CLOEXEC);
    if(fd_in == -1)
    {
        perror("Error opening FIFO_IN for writing");
        retval = -1;
        goto done;
    }

    // Open output FIFO for reading
    fd_out = open(FIFO_OUT, O_RDONLY | O_CLOEXEC);
    if(fd_out == -1)
    {
        perror("Error opening FIFO_OUT for reading");
        retval = -1;
        close(fd_in);
        goto done;
    }

    // Send the filter to the server
    bytes_written = write(fd_in, filter, strlen(filter));
    if(bytes_written == -1)
    {
        perror("Error writing filter to FIFO_IN");
        retval = -1;
        goto cleanup;
    }

    // Delimiter
    write(fd_in, "\n", 1);

    // Send the input string to the server
    bytes_written = write(fd_in, input_string, strlen(input_string));
    if(bytes_written == -1)
    {
        perror("Error writing input string to FIFO_IN");
        retval = -1;
        goto cleanup;
    }

    // Read the processed string from the server
    bytes_read = read(fd_out, buffer, BUFFER_SIZE - 1);
    if(bytes_read == -1)
    {
        perror("Error reading from FIFO_OUT");
        retval = -1;
        goto cleanup;
    }
    buffer[bytes_read] = '\0';

    // Print the processed string received from the server
    printf("Processed String from Server: %s\n\n", buffer);

cleanup:
    close(fd_in);
    close(fd_out);

done:
    return retval;
}

static void parse_arguments(int argc, char *argv[], char **input_string, char filter[FILTER_SIZE])
{
    int opt;

    opterr = 0;

    while((opt = getopt(argc, argv, "hs:f:")) != -1)
    {
        switch(opt)
        {
            case 's':
            {
                *input_string = optarg;
                break;
            }
            case 'f':
            {
                strncpy(filter, optarg, FILTER_SIZE - 1);
                filter[FILTER_SIZE - 1] = '\0';
                break;
            }
            case 'h':
            {
                usage(argv[0], NULL);
                break;
            }
            default:
            {
                usage(argv[0], "Invalid argument");
            }
        }
    }
    // printf("Finished parsing arguments: input_string=%s, filter=%s\n", *input_string, filter);
}

static int handle_arguments(const char *input_string, const char *filter)
{
    if(input_string == NULL)
    {
        return -1;
    }

    if(filter == NULL || (strcmp(filter, "upper") != 0 && strcmp(filter, "lower") != 0 && strcmp(filter, "null") != 0))
    {
        return -1;
    }

    return 0;
}

static void usage(const char *program_name, const char *message)
{
    if(message)
    {
        fprintf(stderr, "%s\n", message);
    }

    fprintf(stderr, "Usage: %s -h -s <string> -f [upper lower null]\n", program_name);
    fputs("Options:\n", stderr);
    fputs("  -h\t Display this help message\n", stderr);
    fputs("  -s\t <string>  Input string to be converted\n", stderr);
    fputs("  -f\t <filter>  Choose from: [upper, lower, null]  \n", stderr);
}
