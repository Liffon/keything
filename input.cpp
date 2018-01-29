#include <linux/uinput.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* these below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

void send_key(int fd, int code)
{
    emit(fd, EV_KEY, code, 1);
    emit(fd, EV_KEY, code, 0);
}

int main() {
    uinput_user_dev uud = {};

    int fd = open("/dev/input/event3", O_RDONLY | O_NONBLOCK);
    printf("fd is %d\n", fd);

    /*
     * Enable the device that is about to be created
     * to pass key events, in this case the space key.
     */


    /*
     * Sleep for a bit to wait for userspace to notice the new device
     */

    sleep(1);

    puts("Please type something.");
    printf("> ");
    char buffer[100];
    memset(buffer, 0, sizeof(buffer));

    /* Key press, report the event, send key release, and report again */

    /*
     * Wait for userspace to read the events before destroying the device
     */
    sleep(2);

    printf("Grabbing!\n");
    ioctl(fd, EVIOCGRAB, 1);

    sleep(2);

    printf("Ungrabbing!\n");
    ioctl(fd, EVIOCGRAB, 0);

    sleep(2);
    printf("Good job. How did it go?\n");

    close(fd);

    return 0;
}
