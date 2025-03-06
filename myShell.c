#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

//1
// פונקציה שמציגה "Welcome" 
void Welcome() {
    printf("        ______\n");
    printf("       /|_||_\\.__\n");
    printf("      (   _    _ _\\\n");
    printf("      =-(_)--(_)-'\n");
    printf("\n");
    printf("        הזיזע\n");
}

//2
// פונקציה שמציגה את המיקום הנוכחי במערכת 
void getLocation() {
    char cwd[1024];
    char *username = getenv("USER");  // For Windows, use getenv("USERNAME")
    char hostname[1024];
    
    gethostname(hostname, sizeof(hostname));

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\033[1;34m");  // Set text color to blue
        printf("User: %s, Hostname: %s\n", username ? username : "Unknown", hostname);
        printf("Current Directory: %s\n", cwd);
        printf("\033[0m");  // Reset text color
    } else {
        perror("getcwd() error");
    }
}

//3
// פונקציה לפיצול מחרוזת לפרמטרים
char** splitArgument(char *str) {
    int size = 0;
    char **args = malloc(64 * sizeof(char*));
    char *token;

    token = strtok(str, " ");
    while (token != NULL) {
        args[size] = token;
        size++;
        token = strtok(NULL, " ");
    }
    args[size] = NULL;  // Null-terminate the array of arguments
    return args;
}

//4
// פונקציה ליציאה מהתוכנית
void logout(char *str) {
    // מחפש את המילה "exit" ומוודא שהיא מופיעה במחרוזת
    if (strstr(str, "exit") != NULL) {
        printf("logout...\n");
        exit(0);  // יציאה מהתוכנית
    } else {
        printf("Unknown command. You need to enter 'exit' to logout.\n");
    }
}

//5
// פונקציה לשינוי ספרייה
void cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: expected argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd failed");
        }
    }
}

//6
// פונקציה להעתקת קבצים
void cp(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "cp: expected source and destination arguments\n");
        return;
    }

    FILE *source = fopen(args[1], "rb");
    if (source == NULL) {
        perror("Error opening source file");
        return;
    }

    FILE *dest = fopen(args[2], "wb");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(source);
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, 1024, source)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }

    fclose(source);
    fclose(dest);
    printf("File copied successfully\n");
}

//7
//פונקציה מחיקת קבצים 
void delete(char **arguments) {
    if (arguments[1] == NULL) {
        fprintf(stderr, "-myShell: delete: missing argument\n");
        return;
    }
    if (unlink(arguments[1]) != 0) {
        perror("-myShell: delete failed");
    }
}

//8
void mypipe(char **argv1, char **argv2) {
    int fd[2];
    pid_t pid1, pid2;

    if (pipe(fd) == -1) {
        perror("pipe failed");
        return;
    }

    if ((pid1 = fork()) == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(argv1[0], argv1);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    if ((pid2 = fork()) == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(argv2[0], argv2);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

//9
//פונקציה העברת של הרקובץ ממקור ליעד 
void move(char **args) {
    char *arguments[4] = {NULL}; // מערך לשמירת עד 4 ארגומנטים
    int i;

    // איסוף הארגומנטים (עד 4)
    for (i = 0; i < 4 && args[i + 1] != NULL; i++) {
        arguments[i] = args[i + 1];
    }

    // בדיקה אם יש לפחות שני ארגומנטים (מקור ויעד)
    if (arguments[0] == NULL || arguments[1] == NULL) {
        fprintf(stderr, "Usage: move <source> <destination>\n");
        return;
    }

    // ביצוע העברת הקובץ/תיקייה
    if (rename(arguments[0], arguments[1]) != 0) {
        perror("move failed");
    }
}

//10
//פונקציה מוסיפה לקובץ את המחרוזת שהתקבלה 
void echoppend(char **args) {
    if (args[1] == NULL || args[2] == NULL || args[3] == NULL || strcmp(args[2], ">>") != 0) {
        fprintf(stderr, "Usage: echo <string> >> <file>\n");
        return;
    }

    printf("DEBUG: Writing to file: %s\n", args[3]);

    // שילוב כל הטקסט עד >>
    char message[1024] = "";
    int i = 1;
    while (args[i] != NULL && strcmp(args[i], ">>") != 0) {
        strcat(message, args[i]);
        strcat(message, " ");
        i++;
    }
    message[strlen(message) - 1] = '\0';

    // פתיחת הקובץ להוספה
    FILE *file = fopen(args[i + 1], "a");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // כתיבה לקובץ
    fprintf(file, "%s\n", message);
    fclose(file);
    printf("DEBUG: Successfully written to file.\n");
}


//11
//פונקציה משנה התוכן לקובץ ל המחרוזת שהתקבלה 
void echowrite(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: echo <string> > <file>\n");
        return;
    }

    // חיפוש סימן `>`
    int i = 1;
    while (args[i] != NULL && strcmp(args[i], ">") != 0) {
        i++;
    }

    // אם אין `>` או שאין שם קובץ אחריו - שגיאה
    if (args[i] == NULL || args[i + 1] == NULL) {
        fprintf(stderr, "Usage: echo <string> > <file>\n");
        return;
    }

    // איחוד כל המילים למחרוזת אחת עד `>`
    char message[1024] = "";
    for (int j = 1; j < i; j++) {
        strcat(message, args[j]);
        strcat(message, " ");  // שמירת הרווחים
    }
    message[strlen(message) - 1] = '\0';  // הסרת הרווח האחרון

    // פתיחת הקובץ לכתיבה (ימחוק תוכן קודם)
    FILE *file = fopen(args[i + 1], "w");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // כתיבת המחרוזת לקובץ
    fprintf(file, "%s\n", message);
    fclose(file);
    printf("DEBUG: Successfully written to %s\n", args[i + 1]);
}


//12
//פונקציה מדפיסה תוכן הקובץ 
void _read(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: read <file>\n");
        return;
    }

    FILE *file = fopen(args[1], "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[1024]; // חוצץ לקריאה
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}
 
//13
//פונקציה מדפים כמה מילים וכמה שורות 
void wordCount(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        fprintf(stderr, "Usage: wc <-l | -w> <file>\n");
        return;
    }

    char *option = args[1];
    char *filename = args[2];

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    int lines = 0, words = 0, inWord = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++; // ספירת שורות
        if (isspace(ch)) {
            inWord = 0;
        } else if (!inWord) {
            inWord = 1;
            words++; // ספירת מילים
        }
    }

    fclose(file);

    if (strcmp(option, "-l") == 0) {
        printf("Lines: %d\n", lines);
    } else if (strcmp(option, "-w") == 0) {
        printf("Words: %d\n", words);
    } else {
        fprintf(stderr, "Invalid option. Use -l for line count or -w for word count.\n");
    }
}

//---------------------------------------------------------------------------------------

// פונקציה שמציגה תפריט למשתמש
void displayMenu() {
    printf("\n--- MyShell Menu ---\n");
    printf("1. Welcome\n");
    printf("2. getLocation\n");
    printf("3. splitArgument\n");
    printf("4. logout\n");
    printf("5. cd\n");
    printf("6. cp\n");
    printf("7. delete\n");
    printf("8. mypipe\n");
    printf("9. move\n");
    printf("10. echoppend\n");
    printf("11. echowrite\n");
    printf("12. _read\n");
    printf("13. wordCount\n");
    printf("-------------------\n");
    printf("Select an option: ");
}

// פונקציה ראשית שמאפשרת למשתמש לבחור מספר ולהריץ פונקציה
int main() {
    char command[1024];
    char **args;
    int option;
    while (1) {
        displayMenu();
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        option = atoi(command);

        switch(option) {
            case 1:
                Welcome();
                break;
            case 2:
                getLocation();
                break;
            case 3:
                printf("Enter string to split: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                for (int i = 0; args[i] != NULL; i++) {
                    printf("Arg %d: %s\n", i+1, args[i]);
                }
                free(args);
                break;
            case 4:
                printf("Enter 'exit' to logout: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                logout(command);
                break;
            case 5:
                printf("Enter directory to change: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                cd(args);
                free(args);
                break;
            case 6:
                printf("Enter source and destination files: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                cp(args);
                free(args);
                break;
            case 7:
                printf("Enter filename to delete: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                delete(args);
                free(args);
                break;
            case 8:
                printf("Enter two commands for piping: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                mypipe(args, args + 1);
                free(args);
                break;
            case 9:
                printf("Enter source and destination: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                move(args);
                free(args);
                break;
            case 10:
                printf("Enter text and filename: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                echoppend(args);
                free(args);
                break;
            case 11:
                printf("Enter text and filename: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                echowrite(args);
                free(args);
                break;
            case 12:
                printf("Enter filename to read: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                _read(args);
                free(args);
                break;
            case 13:
                printf("Enter wc option (-l or -w) and filename: ");
                fgets(command, sizeof(command), stdin);
                command[strcspn(command, "\n")] = '\0';
                args = splitArgument(command);
                wordCount(args);
                free(args);
                break;
            default:
                printf("Unknown option: %d\n", option);
                break;
        }
    }
    return 0;
}
