#include "server.h"

// static volatile sig_atomic_t exit_flag = 0;

// Structure to hold file descriptors for the server
typedef struct
{
    int fd_in;
    int fd_out;
} server_control;

int main(void)
{
    // Program Variables
    server_control controller;
    sigset_t       block_set;
    int            fd_in;
    int            fd_out;
    int            result;
    pthread_t      thread;
    int            retval = 0;

    setup_signal_handler();

    // Block SIGINT Temporarily for server setup
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    if(sigprocmask(SIG_BLOCK, &block_set, NULL) < 0)
    {
        perror("Failed to block SIGINT");
        retval = -1;
        goto done;
    }

    // Create two FIFOS
    if(mkfifo(FIFO_IN, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1)
    {
        if(errno != EEXIST)
        {
            perror("Error creating FIFO_IN");
            retval = -2;
            goto done;
        }
    }

    if(mkfifo(FIFO_OUT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1)
    {
        if(errno != EEXIST)
        {
            perror("Error creating FIFO_OUT");
            retval = -3;
            goto done;
        }
    }

    // Open the read FIFO
    fd_in = open(FIFO_IN, O_RDONLY | O_CLOEXEC);
    if(fd_in == -1)
    {
        perror("Error opening FIFO_IN");
        retval = -4;
        goto done;
    }

    // Open the write FIFO
    fd_out = open(FIFO_OUT, O_WRONLY | O_CLOEXEC);
    if(fd_out == -1)
    {
        perror("Error opening FIFO_OUT");
        // retval = -5;

        close(fd_in);
        goto done;
    }

    printf("\nServer started...\n");

    // Set FIFO args for thread
    controller.fd_in  = fd_in;
    controller.fd_out = fd_out;

    // Unblock SIGINT after setup
    if(sigprocmask(SIG_UNBLOCK, &block_set, NULL) < 0)
    {
        perror("Error while unblocking SIGINT");
        // retval = -6;
        goto cleanup;
    }

    // Main server loop
    while(!exit_flag)
    {
        // Create a new thread
        result = pthread_create(&thread, NULL, read_string_wrapper, (void *)&controller);
        if(result != 0)
        {
            perror("Error creating thread");
            // retval = -7;
            goto cleanup;
        }

        pthread_join(thread, NULL);
    }

    printf("Server shutting down...\n");

cleanup:
    // Close File Descriptors
    close(fd_in);
    close(fd_out);
    unlink(FIFO_IN);
    unlink(FIFO_OUT);

done:
    return retval;
}

void *read_string(const void *arg)
{
    const server_control *argument = (const server_control *)arg;
    int                   fd_in    = argument->fd_in;
    int                   fd_out   = argument->fd_out;

    char    buffer[BUFFER_SIZE];
    char    filter[FILTER_SIZE];
    ssize_t bytes_read;
    ssize_t bytes_wrote;
    int     i = 0;

    // Read the filter
    while((bytes_read = read(fd_in, &buffer[i], 1)) > 0)
    {
        // Keep Reading until the delimiter
        if(buffer[i] == '\n')
        {
            buffer[i] = '\0';
            strcpy(filter, buffer);
            break;
        }
        i++;
        if(i >= FILTER_SIZE - 1)
        {
            buffer[i] = '\0';
            break;
        }
    }
    if(bytes_read == -1)
    {
        perror("Error reading filter from FIFO");
        return NULL;
    }

    // Read the string
    bytes_read = read(fd_in, buffer, sizeof(buffer) - 1);
    if(bytes_read == -1)
    {
        perror("Error reading string from FIFO");
        return NULL;
    }
    buffer[bytes_read] = '\0';

    // Apply the filter onto the string
    process_string(buffer, filter);

    // Write the processed sentence back to the output FIFO
    bytes_wrote = write(fd_out, buffer, strlen(buffer));
    if(bytes_wrote == -1)
    {
        if(errno == EPIPE)
        {
            printf("Cannot write to FIFO_OUT.\n");
        }
        else
        {
            perror("Error writing to FIFO_OUT");
        }
    }

    return NULL;
}

void *read_string_wrapper(void *arg)
{
    return read_string((const void *)arg);
}

void process_string(char *str, const char *filter)
{
    if(strcmp(filter, "upper") == 0)
    {
        for(int i = 0; str[i]; i++)
        {
            str[i] = (char)toupper((unsigned char)str[i]);
        }
    }
    else if(strcmp(filter, "lower") == 0)
    {
        for(int i = 0; str[i]; i++)
        {
            str[i] = (char)tolower((unsigned char)str[i]);
        }
    }
}

void setup_signal_handler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
    sa.sa_sigaction = signal_handler;
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
}

void signal_handler(int signal_number, siginfo_t *info, void *context)
{
    (void)info;
    (void)context;

    if(signal_number == SIGINT)
    {
        printf("Received SIGINT, setting exit_flag.\n");
        exit_flag = EXIT_CODE;
    }
}
