#include <stdio.h>
#include <string.h>

#define MAX_TASKS 100

typedef struct {
    int active;
    int done;
    char text[256];
} Task;

void addTask(void) {
    char input[256];
    FILE *file;

    file = fopen("text.txt", "a");
    if (!file) {
        printf("File Error!!!\n");
        return;
    }
    
    printf("Add a TASK: ");
    fgets(input, sizeof(input), stdin);

    input[strcspn(input, "\n")] = 0;

    // avoid blank tasks
    if (strlen(input) == 0) {
        printf("Empty task : Ignored...\n");
        return;
    }

    fprintf(file, "1|0|%s\n", input);
    fclose(file);

    printf("Task added!\n");
}

int loadTasks(Task tasks[]) {
    FILE *file = fopen("text.txt", "r");
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

void viewTasks(void) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int displayIndex = 1; // the task number to diplay

    if (count == 0) {
        printf("No tasks Found...\n");
        return;
    }

    for (int i = 0;i < count; i++) {
        if (!tasks[i].active) continue;

        printf("%d. [%c] %s\n",
            displayIndex++,
            tasks[i].done ? 'x' : ' ',
            tasks[i].text);
    }
}

void deleteTask(int userIndex) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int visible = 0;

    for (int i = 0; i < count; i++) {
        if (!tasks[i].active) continue;

        visible++;
        if (visible == userIndex) {
            tasks[i].active = 0;
            break;
        }
    }

    FILE *file = fopen("text.txt", "w");
    if (!file) return;

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%d|%s\n",
                tasks[i].active,
                tasks[i].done,
                tasks[i].text);
    }
    fclose(file);

    printf("Task soft-deleted!\n");
}


void completeTask(int userIndex) {
    Task tasks[MAX_TASKS];
    int count = loadTasks(tasks);
    int visible = 0;

    for (int i = 0; i < count; i++) {
        if (!tasks[i].active) continue;

        visible++;
        if (visible == userIndex) {
            tasks[i].done = 1;
            break;
        }
    }

    FILE *file = fopen("text.txt", "w");
    if (!file) return;

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%d|%s\n",
                tasks[i].active,
                tasks[i].done,
                tasks[i].text);
    }
    fclose(file);

    printf("Task marked as completed!\n");
}



int main(void) {
    char command[32];

    printf("TODO CLI app made by GasSpace\n");
    printf(
        "A : Add task\n"
        "V : View tasks\n"
        "C <num> : Complete task\n"
        "D <num> : Delete task\n"
        "Q : Quit\n"
    );

    while (1) {
        printf("> ");

        if (!fgets(command, sizeof(command), stdin))
            break;

        // remove trailing newline
        command[strcspn(command, "\n")] = 0;

        if (command[0] == 'q' || command[0] == 'Q') {
            printf("Exiting program...\n");
            break;
        }
        else if (command[0] == 'a' || command[0] == 'A') {
            addTask();
        }
        else if (command[0] == 'v' || command[0] == 'V') {
            viewTasks();
        }
        else if (command[0] == 'c' || command[0] == 'C') {
            int n;
            if (sscanf(command + 1, "%d", &n) == 1)
                completeTask(n);
            else
                printf("Usage: c <number>\n");
        }
        else if (command[0] == 'd' || command[0] == 'D') {
            int n;
            if (sscanf(command + 1, "%d", &n) == 1)
                deleteTask(n);
            else
                printf("Usage: d <number>\n");
        }
        else {
            printf("Invalid command\n");
        }
    }

    return 0;
}

