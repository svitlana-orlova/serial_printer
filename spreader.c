/*
 * The program to communicate with serial ports.
 * It is based on libserialport, so it has to be installed
 * on the system to compile.
 *
 * For debian-like systems it could be acomplished by
 *
 * $ apt install libserialport-dev
 *
 * The usage is guided by the program itself.
 * enjoy
 */

#define _POSIX_C_SOURCE 2018L
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libserialport.h>

enum {
    DEFAULT_BRATE = 19200
};

static int check(enum sp_return);
static void read_port_task(struct sp_port *port);
static void usage(void);

int main(int ac, char **av)
{
    struct sp_port *port = NULL;
    char *filename = NULL;
    int speed = 0;
    int opt = 0;
    FILE * fd = NULL;

    if (ac < 2) {
        usage();
    }

    while ((opt = getopt(ac, av, "s:")) != -1) {
        switch (opt) {
            case 's':
            {
                errno = 0;
                speed = strtol(optarg, NULL, 10);
                if (errno) speed = 0;
            }
                break;
            default:
                usage();
                break;
        }
    }

    if (optind < ac) {
        filename = av[optind];
    }

    fd = fopen(filename, "r");

    if (fd) {
        fclose(fd);
    } else {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (!speed) {
        fprintf(stdout, "Using default speed: %d\n", DEFAULT_BRATE);
        speed = DEFAULT_BRATE;
    }

    printf("Looking for port: %s\n", filename);
    check(sp_get_port_by_name(filename, &port));

    printf("Opening port: %s\n", filename);
    check(sp_open(port, SP_MODE_READ_WRITE));

    printf("Setting port to %d 8N1, no flow control.\n",
            speed);
    check(sp_set_baudrate(port, speed));
    check(sp_set_bits(port, 8));
    check(sp_set_parity(port, SP_PARITY_NONE));
    check(sp_set_stopbits(port, 1));
    check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));

    read_port_task(port);

    check(sp_close(port));
    sp_free_port(port);

    return EXIT_SUCCESS;
}

static void usage(void)
{
    fprintf(stdout, "Usage: spreader </dev/ttyUSB0>\n");
    exit(EXIT_FAILURE);
}

static void read_port_task(struct sp_port *port)
{
    enum { BUFSIZE = 32 };
    unsigned timeout = 60;
    char buf[BUFSIZE];

    while (1) {
        int const res =
            check(sp_blocking_read(port, buf, BUFSIZE, timeout));
        fwrite(buf, 1, res, stdout);
    }
}

/* Helper function for error handling. */
static int check(enum sp_return result)
{
    /* For this example we'll just exit on any error by calling abort(). */
    char *error_message;

    switch (result) {
        case SP_ERR_ARG:
            printf("Error: Invalid argument.\n");
            abort();
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            printf("Error: Failed: %s\n", error_message);
            sp_free_error_message(error_message);
            abort();
        case SP_ERR_SUPP:
            printf("Error: Not supported.\n");
            abort();
        case SP_ERR_MEM:
            printf("Error: Couldn't allocate memory.\n");
            abort();
        case SP_OK:
        default:
            return result;
    }
}

