#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
extern "C" {
#include <sys/ioctl.h>
#include <unistd.h>
}

#define TERM_COLOR_RESET   "\033[0m"      // reset current terminal color
#define TERM_CLEAR_SCREEN  "\033[2J"      // clear the screen
#define TERM_GOTO_TOP_LEFT "\033[H"       // place cursor at top-left
#define TERM_ERASE_LINE    "\033[K"       // erase from current position to end
#define TERM_CLEAR_LINE    "\033[2K"      // clear the entire line

inline winsize linux_terminal_winsize()
{
    struct winsize w;
    int rc = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (rc)
        throw std::runtime_error(strerror(errno));
    return w;
}
