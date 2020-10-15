#include "ncurses_display.h"

#include <curses.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "system.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, ++row, 2, ("Kernel: " + system.Kernel()).c_str());
  mvwprintw(window, ++row, 2, "CPU: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.Cpu().Utilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.MemoryInfo().Utilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, size_t n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{18};
  int const ram_column{27};
  int const time_column{36};
  int const command_column{47};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (size_t i = 0; i < n; ++i) {
    mvwprintw(window, ++row, pid_column, to_string(processes[i].Pid()).c_str());
    mvwprintw(window, row, user_column,
              processes[i].User().substr(0, 8).c_str());
    float cpu = processes[i].CpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column,
              processes[i].Command().substr(0, window->_maxx - 46).c_str());
  }
}

void NCursesDisplay::Display(System& system, size_t n) {
  initscr();              // start ncurses
  noecho();               // do not print input values
  keypad(stdscr, TRUE);   // enable keys (getch())
  nodelay(stdscr, TRUE);  // getch() becomes non-blocking
  cbreak();               // terminate ncurses on ctrl + c
  start_color();          // enable color

  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW* process_window;

  bool quit = false;
  size_t previous_n = n;

  while (!quit) {
    system.Refresh();
    size_t num_processes = system.Processes().size();
    size_t processes_lines = std::min(num_processes, n);

    process_window =
        newwin(3 + processes_lines, x_max - 1, system_window->_maxy + 1, 0);

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, processes_lines);
    wrefresh(system_window);
    wrefresh(process_window);

    // Clear lines below process window, when 'n' decreases
    if (previous_n > processes_lines) {
      for (size_t offset = n; offset < previous_n; ++offset) {
        move(9 + 3 + offset, 0);
        clrtoeol();
      }
    }

    move(0, 0);  // Keep cursor here
    refresh();
    previous_n = n;

    // Several inputs can be processed between refreshes
    SleepAndCheckInput(system,
                       /* Number of processes */ n,
                       /* milliseconds */ 250,
                       /* number of sleeps */ 4, quit);
  }
  endwin();
}

void NCursesDisplay::SleepAndCheckInput(System& system, size_t& n,
                                        int millisecondsPerSleep,
                                        int numberOfSleeps, bool& quit) {
  int ch;
  quit = false;

  for (int sleep = 0; sleep < numberOfSleeps; ++sleep) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(millisecondsPerSleep));

    ch = getch();
    if (ch == KEY_UP) {
      system.ToggleProcessOrderByCpu();
    } else if (ch == KEY_DOWN) {
      system.ToggleProcessOrderByMemory();
    } else if (ch == '+') {
      // Increase number of processes displayed (upper limit: # processes)
      n = (n < system.Processes().size()) ? (n + 1) : n;
    } else if (ch == '-') {
      // Decrease number of processes displayed (lower limit: 1)
      if (n > 1) {
        --n;
      }
    } else if (ch == 'q') {
      quit = true;
      break;
    }
  }
}
