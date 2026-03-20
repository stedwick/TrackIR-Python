#include "KeyPress.h"

#ifdef _WIN32
#include <conio.h>

bool keyPressed() {
    return _kbhit();
}

void setTerminalMode(bool enable) {
    // No setup needed on Windows
}

#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

static struct termios oldt;

void setTerminalMode(bool enable) {
    struct termios newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

bool keyPressed() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0;
}

#endif
