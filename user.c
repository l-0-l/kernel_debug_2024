#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int fd;
    unsigned long pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid = strtoul(argv[1], NULL, 10);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    if (ioctl(fd, IOCTL_CHANGE_PID, pid) < 0) {
        perror("Failed to issue ioctl");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("ioctl sent PID %lu to kernel module.\n", pid);

    close(fd);
    return EXIT_SUCCESS;
}
