#include <stdio.h>

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

    fprintf(file, "%s", input);
    fclose(file);

    printf("Task added!\n");
}

void viewTasks(void) {
    FILE *file;
    char line[256];

    file = fopen("text.txt", "r");
    if (!file) {
        printf("No tasks found!!!\n");
        return;
    }

    printf("\n-----Tasks-----\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}

int main(void) {
    int action; // The action taken by the User

    printf("TODO CLI app made by GasSpace\n");
    printf("A : Add a new task\nV : View tasks\nQ : Quit this program\n");

    while (1) {
        printf("> ");

        action = getchar();

        // skip newline
        if (action == '\n') continue;

        // clear the buffer
        while (getchar() != '\n');

        if (action == 'q') {
            printf("Exiting program...\n");
            break;
        }
        else if (action == 'a') {
            addTask();
        }
        else if (action == 'v') {
            viewTasks();
        }
    }

    return 0;
}

