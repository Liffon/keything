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

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    int version;
    int rc = ioctl(fd, UI_GET_VERSION, &version);

    // if (rc == 0 && version >= 5) {
    //     /* use UI_DEV_SETUP */
    //     puts("This system supports uinput 5! Use that instead, right?");
    //     return 0;
    // }

    /*
     * Enable the device that is about to be created
     * to pass key events, in this case the space key.
     */
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    for(int key = KEY_ESC; key < KEY_HANGEUL; key++)
    {
        ioctl(fd, UI_SET_KEYBIT, key);
    }

    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "uinput example (old interface)");
    write(fd, &uud, sizeof(uud));

    ioctl(fd, UI_DEV_CREATE);

    /*
     * Sleep for a bit to wait for userspace to notice the new device
     */

    sleep(1);

    puts("Please type something.");
    printf("> ");
    char buffer[100];
    memset(buffer, 0, sizeof(buffer));

    /* Key press, report the event, send key release, and report again */
    send_key(fd, KEY_H);
    send_key(fd, KEY_E);
    send_key(fd, KEY_L);
    send_key(fd, KEY_L);
    send_key(fd, KEY_O);
    send_key(fd, KEY_SPACE);
    send_key(fd, KEY_W);
    send_key(fd, KEY_O);
    send_key(fd, KEY_R);
    send_key(fd, KEY_L);
    send_key(fd, KEY_D);
    send_key(fd, KEY_ENTER);
    emit(fd, EV_SYN, SYN_REPORT, 0);

    fgets(buffer, sizeof(buffer), stdin);
    /*
     * Wait for userspace to read the events before destroying the device
     */
    sleep(1);

    printf("You typed `%s`\n", buffer);

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    return 0;
}
