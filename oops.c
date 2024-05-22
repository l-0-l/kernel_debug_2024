#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "common.h"

int main(int argc, char *argv[]) {
    int fd;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    if (ioctl(fd, IOCTL_CAUSE_OOPS, 0) < 0) {
        perror("Failed to issue ioctl");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("ioctl sent oops to kernel module.\n");

    close(fd);
    return EXIT_SUCCESS;
}
