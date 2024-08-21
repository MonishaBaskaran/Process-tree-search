# Process-tree-search

Write a C program prc24s.c that searches for processes in the process tree (rooted at 
a specified process) and prints the requested information based on the input 
parameters. 

Synopsis : 
prc24s [Option] [root_process] [process_id] 

 When [Option] is not provided: Lists the PID, PPID of process_id if process_id 
belongs to the process tree rooted at root_process else does not print anything 
o Both root_process and process_id are the PIDs of processes that are 
descendants of any Bash Process that belongs to the user 
 When [Option] is provided: Please see the next page for the action to be 
performed for each option. 
Note: In any of the following options, if process_id does not belong to the process tree 
rooted at root_process , you need to print “ The process (list the process_id) does not 
belong to the tree rooted at (list the root_process)” 

OPTION 
-dx The root_process kills all its descendants using SIGKILL 
-dt The root_process sends SIGSTOP to all its descendants 
-dc The root_process sends SIGCONT to all its descendants that have been paused 
-rp root_process kills process_id 
-nd lists the PIDs of all the non-direct descendants of process_id 
-dd lists the PIDs of all the immediate descendants of process_id 
 -sb lists the PIDs of all the sibling processes of process_id 
 -bz lists the PIDs of all the sibling processes of process_id that are defunct 
- zd Lists the PIDs of all descendents of process_id that are defunct 
- od Lists the PIDs of all descendents of process_id that are orphans 
- gc lists the PIDs of all the grandchildren of process_id
- sz prints the status of the process_id (Defunct/ Not Defunct) 
- so prints the status of the process_id (Orphan/Not Orphan) 
- kz Kills the parents of all zombie process that are the descendants of proceed_id 
 //Note: process_id might also get killed 
