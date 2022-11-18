#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include<sys/wait.h>
#include<signal.h>
#include <fcntl.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

// struct for background process
typedef struct bg_process {
    pid_t pid;            // process id
    struct bg_process *next;  
}bg_process;

//struct for alias
typedef struct alias{
    char aliasName[120];
    char aliasCommand[120];
    struct alias *next;
}alias;

//foreground process can equal = 1 or 0
int fg_process = 0;
//current_fg_process store pid of foreground process
int current_fg_process = 0;

//bg_process head of list is NULL
bg_process *bg_process_first = NULL;
//alias head of list is NULL
alias *alias_first = NULL;


//PART C : I/O Redirection
int redirect(char *args[]) { 
    // if redirection is empty
    if (args[1] == NULL){ 
        return 0;
    }

    //if file is empty
    if(args[2] == NULL){
        fprintf(stderr,"Missing agrument.\n");
    }
    //create child
    pid_t pid = fork();

    if (pid < 0){
        fprintf(stderr,"Fork Failed.\n");
    }

    if (pid == 0) {
        if(strcmp(">>", args[1]) == 0){
            int fd_trunc;
            args[1]=NULL;
            fd_trunc = open(args[2], O_WRONLY | O_APPEND | O_CREAT, 0644);
            dup2(fd_trunc, STDOUT_FILENO);
            close(fd_trunc);

        }else if(strcmp(">", args[1]) == 0){
            int fd_append;
            args[1]=NULL;
            fd_append = open(args[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            dup2(fd_append, STDOUT_FILENO);
            close(fd_append);

        }
    }
}

//remove unalias
void unalias(char *args[]){
    alias *node = alias_first;
    alias *prev = alias_first;
    //if alias first is null and alias->aliasname is equal to given args
    if (node != NULL && strcmp(node->aliasName,args[1])==0) {
        alias_first = node->next; // Changed head
    }
    //if alias first is null and alias->aliasname is not equal to given args
    while (node != NULL && strcmp(node->aliasName,args[1])!=0) {
        //change the nodes
        prev = node;
        node = node->next;
    }
    if(node != NULL){
        prev->next = node->next;
        free(node);
        printf("Alias is removed : %s\n",args[1]);
    }else{
        printf("Cant find alias\n");
    }
    
}

// add alias in list
void insert_alias(char *args[]){
    char command[128] = "";
    size_t command_len;
    // count we have how many args
    int index = 0;
    while(args[index]!=NULL){
        index++;
    }
    command_len = strlen(args[index-2]);
    if(index < 4){ // if arguments are missing
        fprintf(stderr, "Missing Arguments!\n");
    }else if(args[index-3][0] != '"' || args[index-2][command_len-1] != '"'){ // If argument is not between " "
        fprintf(stderr, "Arguments are must in between ' '!\n");
    }
    else{
        for (int i = 1; i < index-1; i++){
            strcat(command,args[i]);
            if(i!=index-1){
                strcat(command," ");
            }
        }
        // if there are aliases in list, add new alias
        if(alias_first != NULL){
            alias *new_alias = (alias*)malloc(sizeof(struct alias));
            strcpy(new_alias->aliasName,args[index-1]);
            strcpy(new_alias->aliasCommand,command);
            new_alias->next=alias_first;
            alias_first = new_alias;
            printf("alias %s created with this : %s\n", alias_first->aliasName, alias_first->aliasCommand);
        }else{// if there is no alias in list, add first alias
            alias_first = (alias*)malloc(sizeof(struct alias));
            strcpy(alias_first->aliasName,args[index-1]);
            strcpy(alias_first->aliasCommand,command);
            alias_first->next = NULL;
            printf("alias %s created with this : %s\n", alias_first->aliasName, alias_first->aliasCommand);
        }
    }
}

//print all alias
void print_alias(char *args[]){
    alias *alias_list = alias_first;
    //if alias is not null
    if(alias_list != NULL){
        // find alias and print it
        while(alias_list != NULL){
            printf("%s : %s\n",alias_list->aliasName,alias_list->aliasCommand);
            alias_list = alias_list->next;
        }
    }else{ // if there is no alias
        printf("There is no alias\n");
    }
}

// signal handler
static void signal_handler(int sigNo){
    if(fg_process > 0){
        printf("\nJob (%d) Stopped by signal\n", current_fg_process);
        kill(current_fg_process,SIGKILL);
        current_fg_process = 0;
        fg_process = 0;
    }else{
        printf("\nNothing to kill\n");
        fflush(NULL);
        printf("myshell :");
    }
    fflush(NULL);
}

char *findPath(char *args[],int background){
    // for check file
    FILE *file;
    char *envPath,*token;
    //get Path
    envPath = getenv("PATH");
    token = strtok(envPath, ":");
    char *checking_file = malloc(strlen(token) + strlen(args[0]));
    while (token != NULL)
    {
        strcpy(checking_file, token);
        strcat(checking_file, "/");
        strcat(checking_file, args[0]);
        if (file = fopen(checking_file, "r"))
        {
            return checking_file;
        }
        token = strtok(NULL, ":");
    }
    fprintf(stderr, "Command is not found\n");
}


void exec_command(char *args[],int background){
    pid_t pid;
    pid=fork();
    struct bg_process *proc_queue = NULL;
    if(pid == -1){
        perror("Fork Error");
    }
    if(pid == 0){
        char *filename = findPath(args,background);
        execv(filename,&args[0]);
        exit(0);
    }
    if(background != 0){
        if(bg_process_first != NULL){
            // If there are some background processes, add new background process
            bg_process *new_process = (bg_process*)malloc(sizeof(bg_process));
            new_process->pid=pid;
            new_process->next = bg_process_first;
            bg_process_first=new_process;
        }else{
            // If there is no background process, add first background process
            bg_process_first = (bg_process*)malloc(sizeof(bg_process));
            bg_process_first->pid = pid;
            bg_process_first->next = NULL;
        }
        printf("[%d] : %d \n", background, pid);
    }
    else{
        fg_process = 1;
        current_fg_process = pid;
        waitpid(pid,NULL,0);
        fg_process = 0;

    }
    
    
}

 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */
void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
            i,      /* loop index for accessing inputBuffer array */
            start,  /* index where beginning of next command parameter is */
            ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    //printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i-1] = '\0';
                    i++;
                    args[ct] = NULL;
                }
        } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

//  for (i = 0; i <= ct; i++)
//      printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */
 
int main(void)
{              
            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */
            // Signal
            struct sigaction act;
            act.sa_handler = signal_handler;
            act.sa_flags = SA_RESTART;
            // Set up signal handler for ^Z signal
            if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGTSTP, &act, NULL) == -1)) {
                fprintf(stderr, "Failed to set SIGTSTP handler\n");
                return 1;
            }
            while (1)   
            {
                background = 0;
                printf("myshell :");
                fflush(stdout);
                /*setup() calls exit() when Control-D is entered */
                setup(inputBuffer, args, &background);
                if(args[0]==NULL){
                    continue;
                }else if(strcmp(args[0],"exit")==0){
                    printf("See You\n");
                    exit(0);
                }else if(strcmp(args[0],"clr")==0){
                    system("clear");
                }else if(strcmp(args[0],"alias")==0){
                    if(args[1] != NULL){
                        if(strcmp(args[1],"-l")==0){
                            print_alias(args);
                        }else{
                            insert_alias(args);
                        }
                    }
                }else if(strcmp(args[0],"unalias")==0){
                    unalias(args);
                }else if(redirect(args)){ /*checks for any redirect command*/
                    
                }else{
                    exec_command(args,background);
                }
                /* the steps are:*/
                /*(1) fork a child process using fork()/
                /(2) the child process will invoke execv()/
                /(3) if background == 0, the parent will wait*/
                        
            }
}
