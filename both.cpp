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

    sleep(1);

    int in_device = open("/dev/input/event3", O_RDONLY);
    int out_device = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (in_device < 0) {
        puts("Could not open input device. Does it exist? Do you need to sudo?");
        _exit(1);
    }
    if (out_device < 0) {
        puts("Could not open output device. I don't know why!");
        _exit(1);
    }

    /*
     * Enable the device that is about to be created
     * to pass key events.
     */
    ioctl(out_device, UI_SET_EVBIT, EV_KEY);
    for(int key = KEY_ESC; key <= KEY_MICMUTE; key++)
    {
        ioctl(out_device, UI_SET_KEYBIT, key);
    }

    /* Actually create the device */

    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "fake keyboard thing");
    write(out_device, &uud, sizeof(uud));
    ioctl(out_device, UI_DEV_CREATE);

    /*
     * Sleep for a bit to wait for userspace to notice the new device
     */

    send_key(out_device, KEY_SPACE);

    puts("Starting soon ...");

    sleep(5);

    puts("Please try typing.");
    printf("> ");
    char buffer[100];
    memset(buffer, 0, sizeof(buffer));

    printf("Grabbing!\n");
    ioctl(in_device, EVIOCGRAB, 1);

    for (int num_events = 0; num_events < 100; num_events++)
    {
        input_event event;
        int rd = read(in_device, &event, sizeof(input_event));
        if (event.type == EV_KEY) {
            printf("Key event! Code is %d, value is %d\n", event.code, event.value);
        } else {
            printf("Read %d %d %d\n", event.type, event.code, event.value);
        }

        if (event.code != KEY_F) {
            write(out_device, &event, sizeof(event));
        }
    }

    printf("Ungrabbing!\n");
    ioctl(in_device, EVIOCGRAB, 0);

    sleep(2);
    printf("Good job. How did it go?\n");

    close(in_device);

    return 0;
}
