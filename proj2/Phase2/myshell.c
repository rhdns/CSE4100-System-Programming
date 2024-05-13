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
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
      // 파이프가 포함된 명령을 위한 commands 배열
    char *commands[MAXARGS][MAXARGS]; 

    // 파이프 파일 디스크립터 정의
    int pfilediscriptor[MAXARGS - 1][2]; 

    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
        return;   /* 빈 줄 무시 */
    
    // 명령에 파이프가 포함되어 있는지 확인
    int include_pipe = 0;
    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0) {
            include_pipe = 1;
            break;
        }
    }

    if (include_pipe) {
        // 파이프를 포함하는 경우 처리
        int pipes_number = 0; // 파이프의 개수
        for (int i = 0; argv[i] != NULL; i++) {
            if (strcmp(argv[i], "|") == 0) {
                pipes_number++;
            }
        }

        // 자식 프로세스 ID를 저장할 배열 생성
        pid_t pids[pipes_number + 1];

        int i = 0, cmd_index = 0, arg_index = 0;
        while (argv[i] != NULL) {
            if (strcmp(argv[i], "|") == 0) {
                commands[cmd_index][arg_index] = NULL; // 현재 명령을 널로 종료
                cmd_index++; // 다음 명령으로 이동
                arg_index = 0; // 다음 명령을 위해 인수 인덱스 초기화
            } else {
                commands[cmd_index][arg_index] = argv[i];
                arg_index++;
            }
            i++;
        }
        commands[cmd_index][arg_index] = NULL; // 마지막 명령을 널로 종료

        // 파이프 생성
        for (int i = 0; i < pipes_number; i++) {
            if (pipe(pfilediscriptor[i]) == -1) { // 파이프 생성
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // 각 명령에 대해 자식 프로세스를 생성
        for (int i = 0; i <= pipes_number; i++) {
            pids[i] = fork();
            if (pids[i] == 0) {
                // 자식 프로세스
                if (i != 0) {
                    // 첫 번째 명령이 아닌 경우 이전 파이프에서 입력을 리디렉션
                    dup2(pfilediscriptor[i - 1][0], STDIN_FILENO);
                    close(pfilediscriptor[i - 1][0]); // 이전 파이프의 읽기 끝을 닫음
                }
                if (i != pipes_number) {
                    // 마지막 명령이 아닌 경우 출력을 현재 파이프로 리디렉션
                    dup2(pfilediscriptor[i][1], STDOUT_FILENO);
                    close(pfilediscriptor[i][1]); // 현재 파이프의 쓰기 끝을 닫음
                }

                // 모든 파이프 디스크립터를 닫음
                for (int j = 0; j < pipes_number; j++) {
                    close(pfilediscriptor[j][0]);
                    close(pfilediscriptor[j][1]);
                }

                // 명령 실행
                char *envp[] = { NULL }; // 환경 변수 설정
                char full_path[100]; // 전체 경로를 저장할 배열
                snprintf(full_path, sizeof(full_path), "/bin/%s", commands[i][0]); // 명령어의 전체 경로 설정
                execve(full_path, commands[i], envp);
                perror("execve");
                exit(EXIT_FAILURE);
            } else if (pids[i] < 0) {
                // 오류
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }

        // 부모 프로세스에서 모든 파이프 디스크립터를 닫음
        for (int i = 0; i < pipes_number; i++) {
            close(pfilediscriptor[i][0]);
            close(pfilediscriptor[i][1]);
        }

        // 모든 자식 프로세스가 종료될 때까지 기다림
        for (int i = 0; i <= pipes_number; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }else {
        // 파이프가 포함되지 않은 경우 명령을 일반적으로 실행함
        if (!builtin_command(argv)) {
            pid = fork();
            if (pid == 0) { // 자식프로세스
                char full_path[100]; // 전체 경로를 저장할 배열
                snprintf(full_path, sizeof(full_path), "/bin/%s", argv[0]); // 명령어의 전체 경로 설정
                char *envp[] = { NULL };
                if (execve(full_path, argv, envp) < 0){
                    printf("%s: Command not found.\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
            } else if (pid < 0) { // Error
                perror("fork");
                exit(EXIT_FAILURE);
            } else { // 부모 프로세스
                if (!bg) {
                    int status;
                    waitpid(pid, &status, 0);
                } else {
                    printf("[%d] %s", pid, cmdline);
                }
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
            fprintf(stderr, "myshell: 경로를 지정하세요.\n");
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
    while (*buf) {
        // '"'가 포함된 명령 실행을 위함
        if (*buf == '"') {
            buf++;
            delim = strchr(buf, '"');
            if (delim == NULL) {
                fprintf(stderr, "Syntax error: Unterminated quotation mark.\n");
                return -1;
            }
            argv[argc++] = buf;
            *delim = '\0';
            buf = delim + 1;
        } else {
            delim = strchr(buf, ' ');
            if (delim == NULL) {
                argv[argc++] = buf;
                break;
            }
            *delim = '\0';
            argv[argc++] = buf;
            buf = delim + 1;
        }
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