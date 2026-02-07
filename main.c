#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_TASKS 100
#define FILE_NAME "text.txt"

typedef struct {
    int active;
    int done;
    char text[256];
} Task;

/* ---------- GLOBAL UI STATE ---------- */

static char status_msg[256] = "";

/* ---------- UTILS ---------- */

int center_x(int w, int len) {
    return (w - len) / 2;
}

void set_status(const char *msg) {
    strncpy(status_msg, msg, sizeof(status_msg) - 1);
}

static volatile sig_atomic_t resized = 0;

void handle_resize(int sig) {
    (void)sig;
    resized = 1;
}


/* ---------- FILE ---------- */

int loadTasks(Task tasks[]) {
    FILE *file = fopen(FILE_NAME, "r");
    int count = 0;

    if (!file) return 0;

    while (count < MAX_TASKS &&
           fscanf(file, "%d|%d|%255[^\n]\n",
                  &tasks[count].active,
                  &tasks[count].done,
                  tasks[count].text) == 3) {
        count++;
    }

    fclose(file);
    return count;
}

void saveTasks(Task tasks[], int count) {
    FILE *file = fopen(FILE_NAME, "w");
    if (!file) return;

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%d|%s\n",
                tasks[i].active,
                tasks[i].done,
                tasks[i].text);
    }

    fclose(file);
}

/* ---------- DRAW ---------- */

void draw_base_ui(void) {
    int h = LINES;
    int w = COLS;

    /* clear only the lines we own */
    for (int i = 0; i < 6; i++) {
        move(i, 0);
        clrtoeol();
    }
    for (int i = h - 4; i < h; i++) {
        move(i, 0);
        clrtoeol();
    }

    /* title */
    const char *title = "TODO · GasSpace";
    mvprintw(1, center_x(w, strlen(title)), "%s", title);

    /* command help (kept compact & centered) */
    const char *line1 = "A Add   V View   E <n> Edit   C <n> Complete   D <n> Delete";
    const char *line2 = "R Raw   X Clear   Q Quit";

    mvprintw(3, center_x(w, strlen(line1)), "%s", line1);
    mvprintw(4, center_x(w, strlen(line2)), "%s", line2);

    /* separator */
    mvhline(5, 0, ACS_HLINE, w);

    /* footer separator */
    mvhline(h - 4, 0, ACS_HLINE, w);

    /* status (clipped, calm) */
    mvprintw(h - 3, 2, "Status:");
    mvprintw(h - 3, 10, "%.*s", w - 12, status_msg);

    /* prompt */
    mvprintw(h - 2, 2, "> ");

    refresh();
}


void clear_content(void) {
    int h;
    getmaxyx(stdscr, h, h);
    for (int i = 6; i < h - 4; i++) {
        move(i, 0);
        clrtoeol();
    }
}

/* ---------- TASK OPS ---------- */

void viewTasksUI(void) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int h = LINES;
    int w = COLS;
    int row = 7;
    int idx = 1;

    (void)w; /* unused but kept for symmetry */

    clear_content();

    if (count == 0) {
        mvprintw(row, 4, "No tasks found");
        set_status("Nothing to display");
        return;
    }

    for (int i = 0; i < count && row < h - 4; i++) {
        if (!tasks[i].active) continue;

        mvprintw(row++, 4,
                 "%d. [%c] %s",
                 idx++,
                 tasks[i].done ? 'x' : ' ',
                 tasks[i].text);
    }
    set_status("Viewing Tasks");
}


void addTaskUI(void) {
    char input[256];

    clear_content();

    mvprintw(7, 4, "Add a task:");
    mvprintw(9, 4, "> ");

    move(9, 6);
    clrtoeol();

    echo();
    getnstr(input, 255);
    noecho();

    if (strlen(input) == 0) {
        viewTasksUI();
        set_status("Empty task ignored");
        return;
    }

    FILE *file = fopen(FILE_NAME, "a");
    if (!file) {
        set_status("File error while adding task");
        return;
    }

    fprintf(file, "1|0|%s\n", input);
    fclose(file);

    viewTasksUI();
    set_status("Task added");
}


void completeTaskUI(int userIndex) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int visible = 0, found = 0;

    for (int i = 0; i < count; i++) {
        if (!tasks[i].active) continue;
        visible++;

        if (visible == userIndex) {
            tasks[i].done = 1;
            found = 1;
            break;
        }
    }

    if (!found) {
        set_status("Invalid task index");
        return;
    }

    saveTasks(tasks, count);
    viewTasksUI();
    set_status("Task marked completed");
}

void editTaskUI(int userIndex) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int visible = 0;
    int found = -1;

    /* locate task by visible index */
    for (int i = 0; i < count; i++) {
        if (!tasks[i].active) continue;
        if (++visible == userIndex) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        set_status("Invalid task index");
        return;
    }

    clear_content();

    mvprintw(7, 4, "Old : %s", tasks[found].text);
    mvprintw(9, 4, "New : ");

    move(9, 10);
    clrtoeol();

    char buf[256];
    echo();
    getnstr(buf, 255);

    if (strlen(buf) == 0) {
        set_status("Edit cancelled");
        return;
    }

    strncpy(tasks[found].text, buf, sizeof(tasks[found].text) - 1);
    tasks[found].text[sizeof(tasks[found].text) - 1] = '\0';

    saveTasks(tasks, count);
    viewTasksUI();
    set_status("Task updated");
}


void deleteTaskUI(int userIndex) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int visible = 0, found = 0;

    for (int i = 0; i < count; i++) {
        if (!tasks[i].active) continue;
        visible++;

        if (visible == userIndex) {
            tasks[i].active = 0;
            found = 1;
            break;
        }
    }

    if (!found) {
        set_status("Invalid task index");
        return;
    }

    saveTasks(tasks, count);
    viewTasksUI();
    set_status("Task deleted");
}

void viewTasksRawUI(void) {
    FILE *file = fopen(FILE_NAME, "r");
    char line[256];
    int h, w, row = 7;
    getmaxyx(stdscr, h, w);

    clear_content();

    if (!file) {
        mvprintw(row, center_x(w, 12), "No task file");
        set_status("Raw view failed");
        return;
    }

    while (fgets(line, sizeof(line), file) && row < h - 4) {
        mvprintw(row++, 2, "%s", line);
    }

    fclose(file);
    set_status("Raw file view");
}

void clearTaskDataUI(void) {
    clear_content();
    mvprintw(7, 2, "WARNING: This will delete all tasks!");
    mvprintw(9, 2, "Confirm (y/n): ");

    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        FILE *file = fopen(FILE_NAME, "w");
        if (file) fclose(file);
        viewTasksUI();
        set_status("All task data cleared");
    } else {
        viewTasksUI();
        set_status("Clear data cancelled");
    }
}

/* ---------- MAIN ---------- */

int main(void) {
    char cmd[32];

    initscr();
    signal(SIGWINCH, handle_resize);
    cbreak();
    // noecho();
    keypad(stdscr, TRUE);

    viewTasksUI();
    set_status("Ready");
    draw_base_ui();

    while (1) {
        if (resized) {
            resized = 0;

            endwin();          // reset ncurses
            refresh();
            clear();

            draw_base_ui();    // redraw everything cleanly
        }

        move(LINES - 2, 4);
        clrtoeol();
        getnstr(cmd, 31);

        if (cmd[0] == 'q' || cmd[0] == 'Q')
            break;
        else if (cmd[0] == 'a' || cmd[0] == 'A')
            addTaskUI();
        else if (cmd[0] == 'v' || cmd[0] == 'V')
            viewTasksUI();
        else if (cmd[0] == 'c' || cmd[0] == 'C')
            completeTaskUI(atoi(cmd + 1));
        else if (cmd[0] == 'e' || cmd[0] == 'E')
            editTaskUI(atoi(cmd + 1));
        else if (cmd[0] == 'd' || cmd[0] == 'D')
            deleteTaskUI(atoi(cmd + 1));
        else if (cmd[0] == 'r' || cmd[0] == 'R')
            viewTasksRawUI();
        else if (cmd[0] == 'x' || cmd[0] == 'X')
            clearTaskDataUI();
        else
            set_status("Unknown command");

        draw_base_ui();
    }

    endwin();
    return 0;
}
