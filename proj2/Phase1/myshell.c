#include "myshell.h"

#define MAXLINE 1024
#define MAXARGS 128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        /* Read */
        printf("CSE4100-SP-P2> ");                   
        fgets(cmdline, MAXLINE, stdin); 
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    } 
}

/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* execve()를 위한 목록 */
    char buf[MAXLINE];  
    int bg;              /* 백그라운드 실행 */
    pid_t pid;           /* 프로세스 ID */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
        return;   /* 빈 줄 무시 */
    
    if (!builtin_command(argv)) {
        pid = fork();
        if (pid == 0) { // 자식 프로세스
            char full_path[MAXLINE];
            snprintf(full_path, sizeof(full_path), "/bin/%s", argv[0]); // 명령어의 전체 경로 설정
            char *envp[] = { NULL }; // 환경 변수 설정
            if (execve(full_path, argv, envp) < 0) {//실행
                printf("%s: 명령을 찾을 수 없습니다.\n", argv[0]);
                exit(0);
            }
        } else if (pid < 0) { // 오류
            perror("fork");
            exit(EXIT_FAILURE);
        } else { // 부모 프로세스
            if (!bg) {
                int status;
                waitpid(pid, &status, 0);//wait 처리
            } else {
                printf("[%d] %s", pid, cmdline);
            }
        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit")) /* quit command */
        exit(0);
    if (!strcmp(argv[0], "exit")) /* quit command */
        exit(0);   
    if (!strcmp(argv[0], "cd")) { /* cd command */
        if (argv[1] == NULL) {
            fprintf(stderr,"error");
        } else {
            if (chdir(argv[1]) != 0) {
                perror("myshell");
            }
        }
        return 1;
    }
    return 0;                     /* Not a builtin command */
}

/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}