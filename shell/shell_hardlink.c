#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>



#define MAX_CMD_ARG 	10
#define MAX_CMD_LIST 	10
#define MAX_CMD_GRP	10

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

const char *prompt = "myshell ";

char* cmdgrp[MAX_CMD_GRP];
char* cmdlist[MAX_CMD_LIST];
char* cmdargs[MAX_CMD_ARG];
char cmdline[BUFSIZ];

void fatal(char *str);
void parse_redirect(char* cmd);
void execute_cmd(char *cmdlist);
void execute_cmdline(char* cmdline);
void execute_cmdgrp(char* cmdgrp);
void zombie_handler(int signo);
int makeargv(char *s, const char *delimiters, char** argvp, int MAX_LIST);
void handler_int(int signo);
void handler_quit(int signo);



struct sigaction sig_quit;
struct sigaction sig_int;
struct sigaction act;
static int status;
static int IS_BACKGROUND=0;
pid_t parent_pid;

typedef struct { // 커맨드 구조체
    char* name;
    char* desc;
    int ( *func )( int argc, char* argv[] ); // 함수포인터. 사용할 함수들의 매개변수를 맞춰줌
} COMMAND;

int cmd_cd( int argc, char* argv[] ){ //cd : change directory
    if( argc == 1 )
        chdir( getenv( "HOME" ) );
    else if( argc == 2 ){
        if( chdir( argv[1] ) )
            printf( "No directory\n" );
    }
    else
        printf( "USAGE: cd [dir]\n" );
    
    return TRUE;
}

int cmd_exit( int argc, char* argv[] ){
    printf("!!! Exit !!!\n");
    exit(0);
    
    return TRUE;
}

// ls 명령어 실행 함수 수정
int cmd_ls(int argc, char* argv[]) {
    // 파이프 명령을 처리하기 위한 부분
    if (argc >= 3 && strcmp(argv[1], "|") == 0 && strcmp(argv[2], "grep") == 0) {
        FILE *pipe_output = popen("ls", "r");
        if (pipe_output == NULL) {
            perror("popen");
            return FALSE;
        }

        char buffer[BUFSIZ];
        while (fgets(buffer, BUFSIZ, pipe_output) != NULL) {
            if (strstr(buffer, argv[3]) != NULL) { // 검색어를 포함한 라인만 출력
                printf("%s", buffer);
            }
        }

        pclose(pipe_output);
        return TRUE;
    }
    // 파일 리다이렉션 시 출력 파일명이 주어졌을 때
    if (argc == 3 && strcmp(argv[1], ">") == 0) {
        FILE *output_file = fopen(argv[2], "w");
        if (output_file == NULL) {
            perror("fopen");
            return FALSE;
        }

        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(".")) == NULL) {
            perror("opendir");
            fclose(output_file);
            return FALSE;
        }

        while ((entry = readdir(dir)) != NULL) {
            fprintf(output_file, "%s\n", entry->d_name);
        }

        closedir(dir);
        fclose(output_file);
        return TRUE;
    }

    // 일반적인 ls 명령어 실행
    DIR *dir;
    struct dirent *entry;

    if (argc > 1) {
        fprintf(stderr, "Usage: ls\n");
        return FALSE;
    }

    if ((dir = opendir(".")) == NULL) {
        perror("opendir");
        return FALSE;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return TRUE;
}


int cmd_pwd(int argc, char* argv[]) {
    char cwd[BUFSIZ];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return FALSE;
    }
    return TRUE;
}

int cmd_mkdir(int argc, char* argv[]) {
    if (argc != 2) {
        printf("USAGE: mkdir [directory_name]\n");
        return FALSE;
    }

    if (mkdir(argv[1], 0777) == -1) {
        perror("mkdir");
        return FALSE;
    }

    printf("Directory '%s' created successfully.\n", argv[1]);
    return TRUE;
}

int cmd_rmdir(int argc, char* argv[]) {
    if (argc != 2) {
        printf("USAGE: rmdir [directory_name]\n");
        return FALSE;
    }

    if (rmdir(argv[1]) == -1) {
        perror("rmdir");
        return FALSE;
    }

    printf("Directory '%s' removed successfully.\n", argv[1]);
    return TRUE;
}

int cmd_ln(int argc, char* argv[]) {
    if (argc != 3) {
        printf("USAGE: ln [source_file] [target_file]\n");
        return FALSE;
    }

    if (link(argv[1], argv[2]) == -1) {
        perror("ln");
        return FALSE;
    }

    printf("Linked '%s' to '%s' successfully.\n", argv[1], argv[2]);
    return TRUE;
}

int cmd_cp(int argc, char* argv[]) {
    if (argc != 3) {
        printf("USAGE: cp [source_file] [target_file]\n");
        return FALSE;
    }

    FILE *source, *target;
    char ch;

    source = fopen(argv[1], "r");
    if (source == NULL) {
        perror("cp");
        return FALSE;
    }

    target = fopen(argv[2], "w");
    if (target == NULL) {
        fclose(source);
        perror("cp");
        return FALSE;
    }

    while ((ch = fgetc(source)) != EOF) {
        fputc(ch, target);
    }

    printf("File '%s' copied to '%s' successfully.\n", argv[1], argv[2]);

    fclose(source);
    fclose(target);
    return TRUE;
}

int cmd_rm(int argc, char* argv[]) {
    if (argc != 2) {
        printf("USAGE: rm [file]\n");
        return FALSE;
    }

    if (remove(argv[1]) == -1) {
        perror("rm");
        return FALSE;
    }

    printf("File '%s' removed successfully.\n", argv[1]);
    return TRUE;
}

int cmd_mv(int argc, char* argv[]) {
    if (argc != 3) {
        printf("USAGE: mv [source_file] [target_file]\n");
        return FALSE;
    }

    if (rename(argv[1], argv[2]) == -1) {
        perror("mv");
        return FALSE;
    }

    printf("File '%s' moved to '%s' successfully.\n", argv[1], argv[2]);
    return TRUE;
}

// cat 명령어 실행 함수 수정
int cmd_cat(int argc, char* argv[]) {
    
    // 파일 리다이렉션 시 입력 파일명이 주어졌을 때
    if (argc == 3 && strcmp(argv[1], "<") == 0) {
        FILE *input_file = fopen(argv[2], "r");
        if (input_file == NULL) {
            perror("fopen");
            return FALSE;
        }

        char ch;
        while ((ch = fgetc(input_file)) != EOF) {
            putchar(ch);
        }

        fclose(input_file);
        return TRUE;
    }
    

    // 일반적인 cat 명령어 실행
    if (argc != 2) {
        printf("USAGE: cat [file]\n");
        return FALSE;
    }

    FILE *file;
    char ch;

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("cat");
        return FALSE;
    }

    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);
    return TRUE;
}

int cmd_grep(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: grep <pattern> <file>\n");
        return EXIT_FAILURE;
    }

    char *pattern = argv[1];
    char *filename = argv[2];
    char line[BUFSIZ];

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
        }
    }

    fclose(file);
    return EXIT_SUCCESS;
}




static COMMAND builtin_cmds[] =
{
    { "cd", "change directory", cmd_cd },
    { "exit", "exit this shell", cmd_exit },
    { "quit", "quit this shell", cmd_exit },
    { "ls", "list directory contents", cmd_ls },
    { "pwd", "print working directory", cmd_pwd },
    { "mkdir", "make directory", cmd_mkdir },
    { "rmdir", "remove directory", cmd_rmdir },
    { "ln", "create a link", cmd_ln },
    { "cp", "copy file", cmd_cp },
    { "rm", "remove file", cmd_rm },
    { "mv", "move file", cmd_mv },
    { "cat", "concatenate files and print on the standard output", cmd_cat },
    { "grep", "grep", cmd_grep}
};

void handler_int(int signo){
    printf("\nCTRL + c 입력!! -> SIGINT 실행\n");
    pid_t ppid = getppid();
    kill(ppid, SIGINT);
}

void handler_quit(int signo){
    printf("\nCTRL + z 입력!! -> SIGQUIT 실행\n");
    pid_t ppid = getppid();
    kill(ppid, SIGQUIT);
    raise(SIGQUIT);

}


int main(int argc, char**argv)
{


    int i;
	sigset_t set;
    parent_pid = getppid();
    
	
    
    struct sigaction sig_int;
    sig_int.sa_handler = handler_int; // SIGINT에 대한 핸들러
    sigfillset(&(sig_int.sa_mask));
    sigaction(SIGINT, &sig_int, NULL);
    sig_int.sa_flags = SA_RESTART;

    struct sigaction sig_quit;
    sig_quit.sa_handler = handler_quit; // SIGQUIT에 대한 핸들러
    sigfillset(&(sig_quit.sa_mask));
    sigaction(SIGTSTP, &sig_quit, NULL);
    sig_quit.sa_flags = SA_RESTART;


	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	act.sa_handler = zombie_handler;
	sigaction(SIGCHLD, &act, 0);
    
    

    
	
    while (1) {
        char cwd[BUFSIZ]; // 현재 작업 디렉토리를 저장할 버퍼
        fputs(prompt, stdout);
        if (getcwd(cwd, sizeof(cwd)) != NULL) { // 현재 작업 디렉토리를 가져옴
            printf("[%s] $ ", cwd);
        } else {
            perror("getcwd() error");
            return EXIT_FAILURE;
        }
        fgets(cmdline, BUFSIZ, stdin);
        cmdline[strlen(cmdline) - 1] = '\0';
        execute_cmdline(cmdline);
    }
    return 0;
}

void zombie_handler(int signo)
{
    pid_t pid ;
    int stat ;
    
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated normaly\n", pid) ;
}

void fatal(char *str)
{
	perror(str);
	exit(1);
}

int makeargv(char *s, const char *delimiters, char** argvp, int MAX_LIST)
{
	int i = 0;
	int numtokens = 0;
	char *snew = NULL;
    
	if( (s==NULL) || (delimiters==NULL) )
	{
		return -1;
	}
    
	snew = s+strspn(s, delimiters);
	
	argvp[numtokens]=strtok(snew, delimiters);
	
	if( argvp[numtokens] !=NULL)
		for(numtokens=1; (argvp[numtokens]=strtok(NULL, delimiters)) != NULL; numtokens++)
		{
			if(numtokens == (MAX_LIST-1)) return -1;
		}
    
	if( numtokens > MAX_LIST) return -1;
    
	return numtokens;
}

void parse_redirect(char* cmd)
{
	char *arg;
	int cmdlen = strlen(cmd);
	int fd, i;
	
	for(i = cmdlen-1;i >= 0;i--)
	{
		switch(cmd[i])
		{
			case '<':
				arg = strtok(&cmd[i+1], " \t");
				if( (fd = open(arg, O_RDONLY | O_CREAT, 0644)) < 0)
					fatal("file open error");
				dup2(fd, STDIN_FILENO);
				close(fd);
				cmd[i] = '\0';
				break;
			case '>':
				arg = strtok(&cmd[i+1], " \t");
                if( (fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
					fatal("file open error");
                dup2(fd, STDOUT_FILENO);
                close(fd);
                cmd[i] = '\0';
				break;
			default:break;
		}
	}
    
}

int parse_background(char *cmd)
{
	int i;
    
    for(i=0; i < strlen(cmd); i++)
        if(cmd[i] == '&')
        {
            cmd[i] = ' ';
            return 1;
        }
			
    
	return 0;
}

void execute_cmd(char *cmdlist)
{
    parse_redirect(cmdlist);
    
    if(makeargv(cmdlist, " \t", cmdargs, MAX_CMD_ARG) <= 0)
		fatal("makeargv_cmdargs error");
        
	
    execvp(cmdargs[0], cmdargs);
    fatal("exec error");

}

void execute_cmdgrp(char *cmdgrp)
{
	int i=0;
	int count = 0;
	int pfd[2];
    sigset_t set;
    
	setpgid(0,0);
 	if(!IS_BACKGROUND)
        tcsetpgrp(STDIN_FILENO, getpid());
    
    sigfillset(&set);
    sigprocmask(SIG_UNBLOCK,&set,NULL);
    
    if((count = makeargv(cmdgrp, "|", cmdlist, MAX_CMD_LIST)) <= 0)
        fatal("makeargv_cmdgrp error");
    
	for(i=0; i<count-1; i++)
    {
		pipe(pfd);
		switch(fork())
		{
			case -1: fatal("fork error");
            case  0: close(pfd[0]);
                dup2(pfd[1], STDOUT_FILENO);
                execute_cmd(cmdlist[i]);
            default: close(pfd[1]);
                dup2(pfd[0], STDIN_FILENO);
		}
	}
	execute_cmd(cmdlist[i]);
    
}

void execute_cmdline(char* cmdline)
{
    int count = 0;
    int i=0, j=0, pid;
    char* cmdvector[MAX_CMD_ARG];
    char cmdgrptemp[BUFSIZ];
    int numtokens = 0;
    
    count = makeargv(cmdline, ";", cmdgrp, MAX_CMD_GRP);
    
    for(i=0; i<count; ++i)
    {
        memcpy(cmdgrptemp, cmdgrp[i], strlen(cmdgrp[i]) + 1);
        numtokens = makeargv(cmdgrp[i], " \t", cmdvector, MAX_CMD_GRP);
        
        for( j = 0; j < sizeof( builtin_cmds ) / sizeof( COMMAND ); j++ ){            if( strcmp( builtin_cmds[j].name, cmdvector[0] ) == 0 ){
                builtin_cmds[j].func( numtokens , cmdvector );
                return;
            }
        }
        
		IS_BACKGROUND = parse_background(cmdgrptemp);
        
        switch(pid=fork())
        {
            case -1:
                fatal("fork error");
            case  0:
                execute_cmdgrp(cmdgrptemp);
            default:
                if(IS_BACKGROUND){
                    printf("[%d] Background process started.\n", pid);
                    break;
                }
                 
                waitpid(pid, NULL, 0);
                tcsetpgrp(STDIN_FILENO, getpgid(0));
                fflush(stdout);
        }
    }
    
}


