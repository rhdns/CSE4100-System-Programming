#include "myshell.h"
#include "csapp.h"

#define MAXLINE 1024
#define MAXARGS 128
#define MAXJOBS 128

/* Global Variables */
pid_t jobs[MAXJOBS]; // Array to store job process IDs
char *job_cmds[MAXJOBS]; // Array to store job command strings
int num_jobs = 0; // Number of active jobs
int isActive[MAXJOBS];
char current_cmdline[MAXLINE]; 
pid_t current_fg_pid;
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void list_jobs();
void bg_job(int job_id);
void fg_job(int job_id);
void kill_job(int job_id);
void init_job();
/* Signal handlers */
void sigint_handler(int signum);
void sigtstp_handler(int signum);
void sigchld_handler(int signum);
void remove_ampersand(char* cmdline);
void addjob(pid_t pid, char* cmdline);
char check_process_status(pid_t pid);
int main() {
    
    struct sigaction sa_int, sa_tstp;

    struct sigaction sa_chld;
    // SIGCHLD handler 
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; 
    sigaction(SIGCHLD, &sa_chld, NULL);

    // SIGINT handler 
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTSTP handler 
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    char cmdline[MAXLINE]; /* Command line */
    while (1) {
        printf("CSE4100-SP-P2> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        eval(cmdline);
    }
}
void remove_ampersand(char *cmdline) {
    int len = strlen(cmdline);
    if (len == 0) return; // 명령행이 비어 있으면 아무 작업 X.

    // 끝에 있는 개행 문자를 제거
    if (cmdline[len - 1] == '\n') {
        cmdline[len - 1] = '\0';
        len--;
    }

    // 뒤로 이동하면서 첫 번째 공백이 아닌 문자 탐새 
    int i = len - 1;
    while (i >= 0 && cmdline[i] == ' ') {
        i--; // 끝에 있는 공백은 건너뛰기
    }
    // &인지 확인
    if (i >= 0 && cmdline[i] == '&') { 
        cmdline[i] = '\0';
    }
}
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    
    
    char *argv[MAXARGS]; /* Argument list execvp() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
  // 파이프가 포함된 명령을 위한 commands 배열
    char *commands[MAXARGS][MAXARGS]; 

    // 파이프 파일 디스크립터 정의
    int pfilediscriptor[MAXARGS - 1][2]; // One less pipe than the number of commands
    strcpy(current_cmdline, cmdline);
    strcpy(buf, cmdline);
    remove_ampersand(cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
        return;   /* Ignore empty lines */
    
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
            if (pipe(pfilediscriptor[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // 각 명령에 대해 자식 프로세스를 생성
        for (int i = 0; i <= pipes_number; i++) {
            pids[i] = fork();
            if (pids[i] == 0) {
                setpgid(0, 0);
                
                // 자식 프로세스
                if (i != 0) {
                     // 첫 번째 명령이 아닌 경우 이전 파이프에서 입력을 리디렉션
                    dup2(pfilediscriptor[i - 1][0], STDIN_FILENO);
                    close(pfilediscriptor[i - 1][0]);  // 이전 파이프의 읽기 끝을 닫음
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
                execvp(commands[i][0], commands[i]);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pids[i] < 0) {
                // Error
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < pipes_number; i++) {
            close(pfilediscriptor[i][0]);
            close(pfilediscriptor[i][1]);
        }

        if (bg) {
            //백그라운드 프로세스 리스트에 추가
            addjob(pids[pipes_number], cmdline);
        } else {
            current_fg_pid = pids[pipes_number];
            sigset_t mask, prev_mask;

            sigemptyset(&mask);
            sigaddset(&mask, SIGCHLD);
            for (int i = 0; i <= pipes_number; i++)
                sigsuspend(&prev_mask); // SIGCHLD를 wait
            
        }
    } else {
        if (!builtin_command(argv)) {
            pid = fork();
         
            if (pid == 0) { //자식프로세스
                setpgid(0, 0);
                execvp(argv[0], argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) { // Error
                perror("fork");
                exit(EXIT_FAILURE);
            } else {
                
                //부모 프로세스
                if (bg) {
                    //백그라운드 프로세스를 list에 추가
                    addjob(pid,cmdline);
                    

                } 
                else {
                current_fg_pid = pid;//현재 프로세스의 pid      
                sigset_t mask, prev_mask;
                sigemptyset(&mask);
                sigaddset(&mask, SIGCHLD);
               // SIGCHLD를 wait
                sigsuspend(&prev_mask);

                }
            }
        }
    }
    current_fg_pid = 0;
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
    if (!strcmp(argv[0], "jobs")) { /* jobs command */
        list_jobs();
        return 1;
    }
    if (!strcmp(argv[0], "bg")) { /* bg command */
        if (argv[1] == NULL) {
            fprintf(stderr, "bg: job ID가 필요합니다.\n");
        } else {
            int job_id = atoi(&argv[1][1]); // Get job ID
            bg_job(job_id);
        }
        return 1;
    }
    if (!strcmp(argv[0], "fg")) { /* fg command */
        if (argv[1] == NULL) {
            fprintf(stderr, "fg: job ID가 필요합니다.\n");
        } else {
            int job_id = atoi(&argv[1][1]); // Get job ID
            fg_job(job_id);
        }
        return 1;
    }
    if (!strcmp(argv[0], "kill")) { /* kill command */
        if (argv[1] == NULL) {
            fprintf(stderr, "kill: job ID가 필요합니다.\n");
        } else {
            int job_id = atoi(&argv[1][1]); // Get job ID
            kill_job(job_id);
        }
        return 1;
    }
    return 0;                     /* Not a builtin command */
}

/* process list */
void list_jobs() 
{
    int status;
    for (int i = 0; i < num_jobs; i++) {
        if (isActive[i]){
        
            pid_t pid = jobs[i];

            char state = check_process_status(pid); // 현재 프로세스 상태를 가져옴.
            //백그라운드에서 실행 중
            if (state=='R')
                    printf("[%d] running %s\n", i + 1, job_cmds[i]);
            //Stopped
            else if(state=='T'){
                printf("[%d] suspended %s\n", i + 1, job_cmds[i]);
            }
            else init_job;
        }  
        
    }
    
}
char check_process_status(pid_t pid) {
    char path[40], line[100], state;
    FILE *fp;

    sprintf(path, "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        return;
    }
    
    if (fscanf(fp, "%*d %*s %c", &state) != 1) {
        fclose(fp);
        return;
    }
    
    fclose(fp);
    return state;
}

/* stopped background process ->  running background process */
void bg_job(int job_id) 
{
    if (job_id <= 0 || job_id > num_jobs) {
        fprintf(stderr, "bg: 잘못된 job ID입니다.\n");
        return;
    }
    
    // SIGCONT를 보냄
    if (kill(jobs[job_id - 1], SIGCONT) == -1) {
        perror("bg");
        return;
    }
    printf("[%d] running %s\n", job_id, job_cmds[job_id - 1]);
}


/* background process -> foreground process */
void fg_job(int job_id) {
    if (job_id <= 0 || job_id > num_jobs) {
        fprintf(stderr, "fg: 잘못된 job ID입니다.\n");
        return;
    }
    printf("[%d] running %s\n", job_id, job_cmds[job_id - 1]);
    // SIGCONT를 보냄
    if (kill(jobs[job_id - 1], SIGCONT) == -1) {
        perror("fg");
        return;
    }

    pid_t pid = jobs[job_id - 1];
    current_fg_pid = pid;

    //Wait하며 상태의 변화를 기다림
    int status;
    while (1) {
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status) || WIFEXITED(status) || WIFSIGNALED(status)) {
            break;
        }
    }
}
//list에 추가
void addjob(pid_t pid, char* cmdline) {
    if (num_jobs >= MAXJOBS) {
        fprintf(stderr, "addjob: 최대 작업 수를 초과했습니다.\n");
        return;
    }
    
    if (strstr(cmdline, "fg") == NULL) {
    jobs[num_jobs] = pid;
    job_cmds[num_jobs] = strdup(cmdline);
    isActive[num_jobs] = 1;
    num_jobs++;
    }
}
//list 초기화
void init_job(){
    for(int i=0;i<num_jobs;i++){
        free(job_cmds[i]);
        job_cmds[i] = NULL;
        isActive[i] = 0;
    }
    num_jobs=0;
}
/* kill process */
void kill_job(int job_id) {
    int isExist;
    if (job_id <= 0 || job_id > num_jobs) {
        fprintf(stderr, "kill: 잘못된 job ID입니다.\n");
        return;
    }

    // 지정된 job ID에 해당하는 프로세스 종료 시킴
    if (kill(jobs[job_id - 1], SIGKILL) == -1) {
        perror("kill");
        return;
    }
    else{
        int isExist=0;
        // 종료 성공 시, 해당 process의 활성 상태를 비활성화로 설정
        isActive[job_id - 1] = 0;         
        free(job_cmds[job_id - 1]);
        job_cmds[job_id - 1] = NULL;
        for(int i=0; i<=num_jobs;i++){
            if(job_cmds[i-1] != NULL)
                isExist++;

        }
        if(isExist==0)
            num_jobs=0;

    }
}

/* SIGINT handler */
void sigint_handler(int signum){

    kill(current_fg_pid, SIGINT);
    current_fg_pid =0;
    
    return;
    
}

/* SIGTSTP handler */
void sigtstp_handler(int signum) {
     sigset_t mask, prev;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK,&mask,&prev);
    if (current_fg_pid != 0) {
        if(kill(current_fg_pid, SIGTSTP)==-1)perror("sigtstp");
        addjob(current_fg_pid, current_cmdline);

        printf("PID: %d, PGID: %d\n", current_fg_pid, getpgid(0));
    }
    sigprocmask(SIG_SETMASK, &prev, NULL);
}
/* SIGCHLD handler */
void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {

        }

    }
}
int parseline(char *buf, char **argv) {
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg = 0;          /* Background job? */

    buf[strlen(buf) - 1] = ' '; /* Replace trailing '\n' with space */
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
            if (*buf == ' ') buf++;  
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

    /*  '&'가 있는지 check */
    if (argc > 0 && strcmp(argv[argc - 1], "&") == 0) {
        bg = 1; 
        argv[--argc] = NULL; // &를 삭제
    } else if (argc > 0 && argv[argc - 1][strlen(argv[argc - 1]) - 1] == '&') {
        bg = 1; 
        argv[argc - 1][strlen(argv[argc - 1]) - 1] = '\0'; // &를 삭제
        if (strlen(argv[argc - 1]) == 0) {  // 
            argc--;  
        }
    }

    return bg;
}