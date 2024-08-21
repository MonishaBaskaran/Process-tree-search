#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#define MAX 1024

//Struct to get process information
typedef struct {
    int pid;
    int ppid;
    char state;
} ProcessInfo;

// Written function logic to get process information from the proc
int get_process_info(int pid, ProcessInfo *info) {
    char path[MAX];
    char buf[MAX];
    FILE *fp;

    //Open the stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (!fp) return -1;

    if (fgets(buf, sizeof(buf), fp)) {
        sscanf(buf, "%d %*s %c %d", &info->pid, &info->state, &info->ppid); //Read the pid, parent pid, state
    }
    fclose(fp);
 
    return 0;
}

// Here I have wrriten logic to check if a process is a descendant of a given root processs
int is_descendant(int root_pid, int process_pid) {
    ProcessInfo info;
    while (process_pid != 1) {
        if (get_process_info(process_pid, &info) == -1) return 0;
        if (info.ppid == root_pid) return 1;
        process_pid = info.ppid;
    }
    return 0;
}

// Function to list descendants of a process and perform actions based on the signal and list_defunct_only flag
void list_descendants(int pid, int signal, int list_defunct_only) {
    DIR *proc = opendir("/proc");
    struct dirent *entry;

    if (!proc) {
        perror("Error opening /proc directory");
        return;
    }

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) { //check if its a directory
            ProcessInfo info;
            int proc_pid = atoi(entry->d_name);
            if (get_process_info(proc_pid, &info) == 0 && is_descendant(pid, proc_pid)) {
                if (list_defunct_only) {
                    if (info.state == 'Z') { //Check if its a zombie process
                        printf("Defunct Descendant PID: %d\n", info.pid);
                    }
                } else {
                    if (signal > 0) { //Checks if any signal is passed as argument
                        kill(proc_pid, signal);
                    } else {
                        printf("Descendant PID: %d\n", info.pid);
                    }
                }
            }
        }
    }
    closedir(proc);
}

// written function logic to list immediate descendants of a process if found else print not found
void list_immediate_descendants(int pid) {
    DIR *proc = opendir("/proc");
    struct dirent *entry;
    int find=0;
    
    if (!proc) return;

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) { //check if its a directory
            ProcessInfo info;
            int proc_pid = atoi(entry->d_name);
            if (get_process_info(proc_pid, &info) == 0 && info.ppid == pid) { //check if current process parent id is same as given process id
                find = 1;
                printf("Immediate Descendant PID: %d\n", info.pid);
            }
        }
    }
    
    closedir(proc);
    if (find!=1) {
                printf("There were no immediate descendants found.\n");
            }
}
 
// Here I have wrriten code to list non-direct descendants of a process if found else print not found
void list_non_direct_descendants(int pid) {
    DIR *proc = opendir("/proc");
    struct dirent *entry;
    int find=0;
    if (!proc) {
        perror("Error opening /proc directory");
        return;
    }

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && isdigit(*entry->d_name)) { //check if its a directory
            pid_t proc_pid = atoi(entry->d_name);
            ProcessInfo info;
            if (get_process_info(proc_pid, &info) == 0 && info.ppid != pid && is_descendant(pid, proc_pid)) { //check if current process parent id is not same as given process id but it is a descendant process
                printf("Non direct Descendants:%d\n", proc_pid);
                find = 1;
            }
        }
    }

    closedir(proc);
    if (find!=1) {
                    printf("There were no non-direct descendants found.\n");
                }
}

// Function code to list grandchildren of a process if found else print not found
void list_grandchildren(pid_t process_id) {
    DIR *proc = opendir("/proc");
    struct dirent *entry;
    int find=0;
    if (!proc) {
        perror("Failed to open /proc directory");
        return;
    }

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            ProcessInfo child_info;
            pid_t child_pid = atoi(entry->d_name);
            if (get_process_info(child_pid, &child_info) == 0 && child_info.ppid == process_id) {
                // Found an immediate child, now look for its children
                DIR *proc2 = opendir("/proc");
                struct dirent *entry2;
                if (!proc2) {
                    perror("Failed to open /proc directory");
                    closedir(proc);
                    return;
                }

                while ((entry2 = readdir(proc2)) != NULL) {
                    if (entry2->d_type == DT_DIR && atoi(entry2->d_name) > 0) {
                        ProcessInfo grandchild_info;
                        pid_t grandchild_pid = atoi(entry2->d_name);
                        if (get_process_info(grandchild_pid, &grandchild_info) == 0 && grandchild_info.ppid == child_pid) {
                            printf("Grandchild PID: %d\n", grandchild_pid);
                            find=1;
                        }
                    }
                }
                closedir(proc2);
            }
        }
    }
    closedir(proc);
    if (find!=1) {
                        printf("There were no grandchildren found.\n");
                    }
}

// Function logic to list siblings of a process if found else print not found
void list_siblings(int pid, int list_defunct_only) {
    ProcessInfo target_info;
    get_process_info(pid, &target_info);
    DIR *proc = opendir("/proc");
    struct dirent *entry;
    int find=0;

    if (!proc) return;

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            ProcessInfo info;
            int proc_pid = atoi(entry->d_name);
            if (get_process_info(proc_pid, &info) == 0 && info.ppid == target_info.ppid && info.pid != pid) { //checks if the current process ppid is same as the given process ppid
                find=1;
                if (list_defunct_only) {
                    if (info.state == 'Z') { //check if the current process state is zombie
                        printf("Defunct Sibling PID: %d\n", info.pid);
                    }
                } else {
                    printf("Sibling PID: %d\n", info.pid);
                }
            }
        }
    }
    closedir(proc);
    if (find!=1) {
                        printf("There were no siblings found.\n");
                    }
}

//Function to Kill the parents of all zombie process that are the descendants of the given process id
void kill_zombieParent(int root_pid) {
    DIR *proc = opendir("/proc");
    struct dirent *entry;

    if (!proc) {
        perror("Error opening /proc directory");
        return;
    }

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            ProcessInfo info;
            int proc_pid = atoi(entry->d_name);
            if (get_process_info(proc_pid, &info) == 0 && is_descendant(root_pid, proc_pid)) {
                if (info.state == 'Z') {
                    // Check if parent exists
                    ProcessInfo parent_info;
                    if (get_process_info(info.ppid, &parent_info) == 0) {
                        // Kill the parent of the zombie process
                        kill(parent_info.pid, SIGKILL);
                        printf("Killed parent PID: %d of zombie PID: %d\n", parent_info.pid, info.pid);
                    }
                }
            }
        }
    }
    closedir(proc);
}

// Here I have written function to get the process status whether it is defunct or not, orphan or not
void get_status(int pid)
{
    ProcessInfo info;
    char is_zombie_orphan;
    if (get_process_info(pid, &info) == 0) {
        is_zombie_orphan = info.state;
        if(is_zombie_orphan =='Z' && info.ppid != 1)
        {
            printf("Defunct");
        }
        else if(is_zombie_orphan != 'Z' && info.ppid != 1 )
        {
            printf("Not Defunct");
        }
        else if(info.ppid == 1 && is_zombie_orphan != 'Z') 
        {
            printf("Orphan");
        }
        else if(info.ppid != 1 && is_zombie_orphan != 'Z')
        {
            printf("Not Orphan");
        }
        else
        {
            printf("NONE");
        }


    } else {
        printf("Failed to get process info for PID: %d\n", pid);
    }
}

/* Main program starts here */
int main(int argc, char *argv[]) {
    
    int root_pid;
    int process_pid;
    char *option;
    
    /*  Check if number of arguments are provided correctly */ 
    if (argc < 3) {
        fprintf(stderr, "./%s [option] [root_process] [processs_id]\n", argv[0]);
        return 1;
    }
    else if( argc == 3) //if option argument is not provided, then this else if block will be executed
    {
         root_pid = atoi(argv[1]); //root process id
         process_pid = atoi(argv[2]); //process id
         
         //Checks if the given process id is descendant of given processs
         if (!is_descendant( root_pid, process_pid)) {
            printf("Process %d does not belong to the process tree positioned at %d\n", process_pid, root_pid);
            return 0;
            }
         else 
            {
                ProcessInfo info;
                    if (get_process_info(process_pid, &info) == 0) {
                        printf("Process ID: %d, Parent PID: %d\n", info.pid, info.ppid);
                    }
            }
        
    }
    else if( argc == 4) //if option argument is provided, then this else if block will be executed 
    {
         option = argv[1];
         root_pid = atoi(argv[2]);
         process_pid = atoi(argv[3]);
        
         if (!is_descendant( root_pid, process_pid)) {
            printf("Process %d does not belong to the process tree positioned at %d\n", process_pid, root_pid);
            return 0;
         }
         else {

            if (strcmp(option, "-dx") == 0) {            
                    list_descendants(root_pid, SIGKILL, 0);    //kills all descendants using SIGKILL          
            }
            else if(strcmp(option, "-dt") == 0) {
                    list_descendants(root_pid, SIGSTOP, 0);    //Sending SIGSTOP signals to descendants 
            }
            else if(strcmp(option, "-dc") == 0) {
                    list_descendants(root_pid, SIGCONT, 0);     //Sending SIGCONT signals to descendants
            }
            else if(strcmp(option, "-rp") == 0) {
                    kill(process_pid, SIGKILL);                 //root_process kills process_id 
            }
            else if(strcmp(option, "-dd") == 0) {
                    list_immediate_descendants(process_pid);    //prints all the direct descendants of process pid if found
            }
            else if(strcmp(option, "-sb") == 0) {
                    list_siblings(process_pid, 0);              //prints all the siblings of process pid if found 
            }
            else if(strcmp(option, "-gc") == 0) {
                    list_grandchildren(process_pid);            //prints all the grandchildren of process pid if found
            }
            else if(strcmp(option, "-nd") == 0) {
                    list_non_direct_descendants(process_pid);       //lprints all the non direct descendants of process pid if found
            }
            else if(strcmp(option, "-sz") == 0) {
                    get_status(process_pid);                    //prints the whether the process is Defunct or Not 
            }
            else if(strcmp(option, "-so") == 0) {
                    get_status(process_pid);                    //prints the whether the process is Orphan or Not
            }
            else if (strcmp(option, "-bz") == 0) {
                list_siblings(process_pid, 1);                  //prints all the siblings of process pid that are defunct if found
            }
            else if (strcmp(option, "-zd") == 0) {
                list_descendants(process_pid, 0, 1);            //prints all the descendants of process pid that are defunct if found 
            }
            else if (strcmp(option, "-kz") == 0) {
                kill_zombieParent(process_pid);                 //Kills the parents of all zombie process that are the descendants of proceed_id
            }
         }
         
    }
    else
    {
        //prints help statement
        fprintf(stderr, "./%s [option] [root_process] [process_id]\n", argv[0]);
        return 1;
    }
    
    return 0;
}
