#include <linux/uinput.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>

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

static volatile bool has_received_interruption_signal = false;

void signal_handler(int signal) {
    assert(signal == SIGINT);
    has_received_interruption_signal = true;
}

int main() {
    uinput_user_dev uud = {};

    //TODO(liffon): figure out how to handle this properly
    // Right now we just fail if SUDO_UID is not set
    // or if it is set to 0
    char *sudo_uid_str = getenv("SUDO_UID");
    if (!sudo_uid_str) {
        puts("I would prefer it if you ran this through sudo.");
        puts("(You have to have root privileges for it to work anyway.)");
        exit(1);
    }
    uid_t sudo_uid = atoi(sudo_uid_str);
    //printf("SUDO_UID = %d\n", sudo_uid);
    if (!sudo_uid) {
        puts("Hey, SUDO_UID should not be 0!");
        exit(1);
    }

    usleep(100000);

    int in_device = open("/dev/input/event3", O_RDONLY);
    if (in_device < 0) {
        puts("Could not open input device. Does it exist? Do you need to sudo?");
        exit(1);
    }


    int out_device = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (out_device < 0) {
        puts("Could not open output device. I don't know why!");
        exit(1);
    }

    puts("Opened devices. Don't need to be root now.");
    if (setuid(sudo_uid)) {
        puts("Failed to deescalate privileges!");
        exit(1);
    }

    /*
     * Enable the device that is about to be created
     * to send lots of different key events.
     */
    ioctl(out_device, UI_SET_EVBIT, EV_KEY);
    for(int key = KEY_ESC; key <= KEY_MICMUTE; key++)
    {
        ioctl(out_device, UI_SET_KEYBIT, key);
    }

    /* Actually create the device */

    //TODO(liffon): Make up a name for this
    snprintf(uud.name, UINPUT_MAX_NAME_SIZE, "Keything Virtual Input Device");
    write(out_device, &uud, sizeof(uud));
    ioctl(out_device, UI_DEV_CREATE);

    send_key(out_device, KEY_SPACE);

    char buffer[100];
    memset(buffer, 0, sizeof(buffer));

    /*
     * EVIOCGRAB makes sure no one else can read from the device
     */
    puts("Grabbing!");
    if(ioctl(in_device, EVIOCGRAB, 1)) {
        puts("Failed to grab input device! Is another instance running?");
        exit(1);
    }

    bool in_super_mode = false;
    bool in_super_delete_mode = false;
    bool in_super_select_mode = false;

    signal(SIGINT, signal_handler);

    while (!has_received_interruption_signal)
    {
        input_event event;
        int rd = read(in_device, &event, sizeof(input_event));
        //if (event.type == EV_KEY) {
            //printf("Key event! Code is %d, value is %d\n", event.code, event.value);
        //} else {
            //printf("Read %d %d %d\n", event.type, event.code, event.value);
        //}

        if (event.type == EV_KEY) {            
            if (event.code == KEY_CAPSLOCK) {
                in_super_mode = event.value;
                continue;
            }

            if (in_super_mode) {
                if (event.code == KEY_ESC) {
                    break;
                }

                if (event.code == KEY_D) {
                    in_super_delete_mode = event.value;
                    continue;
                } else if (event.code == KEY_F) {
                    in_super_select_mode = event.value;
                    continue;
                }
                input_event new_event = event;
                if (event.code == KEY_I) {
                    new_event.code = KEY_UP;
                    write(out_device, &new_event, sizeof(new_event));
                } else if (event.code == KEY_J) {
                    new_event.code = KEY_LEFT;
                    write(out_device, &new_event, sizeof(new_event));
                } else if (event.code == KEY_K) {
                    new_event.code = KEY_DOWN;
                    write(out_device, &new_event, sizeof(new_event));
                } else if (event.code == KEY_L) {
                    new_event.code = KEY_RIGHT;
                    write(out_device, &new_event, sizeof(new_event));
                } else if (event.code == KEY_H) {
                    new_event.code = KEY_HOME;
                    write(out_device, &new_event, sizeof(new_event));
                } else if (event.code == KEY_SEMICOLON) {
                    new_event.code = KEY_END;
                    write(out_device, &new_event, sizeof(new_event));
                } else {
                    // Other keys should do nothing in super mode!
                }
            } else {
                write(out_device, &event, sizeof(event));
            }
        } else {
            write(out_device, &event, sizeof(event));
        }
    }

    puts("\nUngrabbing!");
    ioctl(in_device, EVIOCGRAB, 0);

    close(in_device);

    return 0;
}
