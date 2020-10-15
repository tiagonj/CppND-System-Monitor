#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

#include "process.h"
#include "system.h"

namespace NCursesDisplay {
void Display(System& system, size_t n = 10);

void SleepAndCheckInput(System& system, size_t& n, int millisecondsPerSleep,
                        int numberOfSleeps, bool& quit);

void DisplaySystem(System& system, WINDOW* window);

void DisplayProcesses(std::vector<Process>& processes, WINDOW* window,
                      size_t n);

std::string ProgressBar(float percent);
};  // namespace NCursesDisplay

#endif