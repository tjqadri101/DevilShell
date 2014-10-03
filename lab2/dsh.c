#include "dsh.h"

void seize_tty(pid_t callingprocess_pgid); /* Grab control of the terminal for the calling process pgid.  */
void continue_job(job_t *j); /* resume a stopped job */
void spawn_job(job_t *j, bool fg); /* spawn a new job */
void set_job_status(job_t *j);/*set process status for a spawned job*/

job_t *active_list = NULL; //active list of jobs


/* Sets the process group id for a given job and process */
int set_child_pgid(job_t *j, process_t *p)
{
    if (j->pgid < 0) /* first child: use its pid for job pgid */
        j->pgid = p->pid;
    return(setpgid(p->pid,j->pgid));
}

/* Creates the context for a new child by setting the pid, pgid and tcsetpgrp */
void new_child(job_t *j, process_t *p, bool fg)
{
         /* establish a new process group, and put the child in
          * foreground if requested
          */

         /* Put the process into the process group and give the process
          * group the terminal, if appropriate.  This has to be done both by
          * the dsh and in the individual child processes because of
          * potential race conditions.  
          * */

         p->pid = getpid();

         /* also establish child process group in child to avoid race (if parent has not done it yet). */
         set_child_pgid(j, p);

         if(fg) // if fg is set
		seize_tty(j->pgid); // assign the terminal

         /* Set the handling for job control signals back to the default. */
         signal(SIGTTOU, SIG_DFL);
}

/* Spawning a process with job control. fg is true if the 
 * newly-created process is to be placed in the foreground. 
 * (This implicitly puts the calling process in the background, 
 * so watch out for tty I/O after doing this.) pgid is -1 to 
 * create a new job, in which case the returned pid is also the 
 * pgid of the new job.  Else pgid specifies an existing job's 
 * pgid: this feature is used to start the second or 
 * subsequent processes in a pipeline.
 * */

void spawn_job(job_t *j, bool fg) 
{

	pid_t pid;
	process_t *p;

	for(p = j->first_process; p; p = p->next) {

	  /* YOUR CODE HERE? */
	  /* Builtin commands are already taken care earlier */
	  
	  switch (pid = fork()) {

          case -1: /* fork failure */
            perror("fork");
            exit(EXIT_FAILURE);

          case 0: /* child process  */
            p->pid = getpid();	    
	    //printf("Child pid %d\n", getpid());
            new_child(j, p, fg);
            
	    /* YOUR CODE HERE?  Child-side code for new process. */

		//Negatu's code
		if(p->ofile != NULL){
		 close(STDOUT_FILENO);
		 open(p->ofile, O_CREAT|O_WRONLY, S_IRWXU);
		}
		if(p->ifile != NULL){
		 close(STDIN_FILENO);
		 open(p->ifile, O_RDONLY, S_IRWXU);
		}
		if(p->next != NULL){
		 process_t *p2;//next process in pipeline
		 p2 = p->next;
		 
		}

		execvp(p->argv[0],p->argv);
		//Negatu's code ends here

            perror("New child should have done an exec");
            exit(EXIT_FAILURE);  /* NOT REACHED */
            break;    /* NOT REACHED */

          default: /* parent */
            /* establish child process group */
            p->pid = pid;
            set_child_pgid(j, p);
            /* Parent-side code for new process.  */
	    //Talal's code
	    if (waitpid(pid, &p->status, WNOHANG|WUNTRACED) < 0){//waitpid here returns immediately, with a return value of 0,...
							      //if none of the children in the wait set has stopped or terminated, or with a...
								//return value equal to the PID of one of the stopped or terminated children.
			 perror("fork");
            		 exit(EXIT_FAILURE);
	    }
          }
	}
	/* Parent-side code for new job.*/
	//By this point, all child processes have been created
	//We know a job is completed only when all the processes have exited
	//However,we want the processes to run concurrently si waitpid in the for loop above is non-blocking
	set_job_status(j);
	//By this point all processes for this job have exited or stopped
	seize_tty(getpid()); // assign the terminal back to dsh
	
}
//Sets the status int of a process_t, as well as the completed and stopped bools
//Method ensures that these values are updated after a process has completed
void set_job_status(job_t *j){
	process_t *p;
	pid_t pid;
	for(p = j->first_process; p; p = p->next){
		
		while((pid = waitpid(p->pid, &p->status, WNOHANG|WUNTRACED)) == 0); //waitpid returns 0 here if child has not terminated; otherwise returns child pid or negative number on error
			//p has exited by now, either normally or abnormally		
			if(pid < 0){
				perror("Process exited abnormally");
				exit(EXIT_FAILURE);
			}
			else{
				if (WIFEXITED(p->status)){
					printf("Process %d of job %ld terminated normally with exit status=%d\n",p->pid, (long)j->pgid, WEXITSTATUS(p->status));
					p->completed = true;
				}
				else if (WIFSTOPPED(p->status)){
					printf("Process %d of job %ld stopped by signal number=%d\n",p->pid, (long)j->pgid, WSTOPSIG(p->status));
					p->stopped = true;
				}
				else if (WIFSIGNALED(p->status)){
					printf("Process %d of job %ld terminated by signal number=%d\n",p->pid, (long)j->pgid, WTERMSIG(p->status));
					p->completed = true;
				}
			}
	}
}
//Talal's code ends here

/* Sends SIGCONT signal to wake up the blocked job */
void continue_job(job_t *j) 
{
     if(kill(-j->pgid, SIGCONT) < 0)
          perror("kill(SIGCONT)");
}

/*Talal's code block here*/
//Given a pgid and a head of the job list, returns the job_t with the pgid in the list, if no such job then returns null
job_t* find_job_in_list(pid_t pgid, job_t *first_job){
	if(!pgid || !first_job)
		return NULL;
	job_t *cur_job;
	cur_job = first_job;
	while(cur_job != NULL) {
		if(cur_job->pgid == pgid)
			return cur_job;
		cur_job = cur_job->next;
	}
	//job not found in list
	printf("Job with group id = %d was not found.\n", pgid);
	return NULL;

}
/*Wakes up a blocked job, updates the status of the blocked processes to false, then updates a process's overall status flags after a process has completed*/
void wake_update_job(job_t *j){
	process_t *p;
	continue_job(j);
	printf("Resuming job with group id = %d\n",j->pgid);
	//update stopped field of the processes
	for(p = j->first_process; p; p = p->next){
		p->stopped = false;
	}
	//for each process of the job, see the status at the end of process (completed, stopped, interrupted) and set the flags in the process_t sturct appropriately*/
	set_job_status(j);
	if(job_is_stopped(j)){
		if(job_is_completed(j)){
			delete_job(j, active_list); //remove from active list
		}
		//else if job suspended again, leave in active_list
	}
}
/*Talal's code ends*/

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 * it immediately.  
 */
bool builtin_cmd(job_t *last_job, int argc, char **argv) 
{
	/*Talal's code*/

	    /* check whether the cmd is a built in command
        */

        if (!strcmp(argv[0], "quit")) {
            /* Your code here */
            exit(EXIT_SUCCESS);
	}
        else if (!strcmp("jobs", argv[0])) {
            // Only prints out the suspended jobs that have not yet completed 
	    //The user is notified of completed jobs when the prompt appears again...
            //which means that the job has completed and the shell is running in the foreground again
	    print_job(active_list->next);
            return true;
        }
	else if (!strcmp("cd", argv[0])) {
		if(chdir(argv[1]) < 0)
			perror("Incorrect directory path was given.");
		return true;
        }
        else if (!strcmp("bg", argv[0])) {
            /* Your code here */
        }
        else if (!strcmp("fg", argv[0])) {
            if(argv[1]){//the pgid was given as an argument
		int pgid = (int) (strtol(argv[1], NULL, 0));
		wake_update_job(find_job_in_list(pgid, active_list));
	    }
	    else{//No pgid was given as an argument
		wake_update_job(find_last_job(active_list));
	    }
	    return true;
        }
        return false;       /* not a builtin command */
	/*Talals code ends here*/
}
/*Talal's code block here*/
void append_to_joblist(job_t *j, job_t * head){
	j->next = NULL;
	find_last_job(head)->next = j;
}



/*Talal's code block ends here*/

/* Build prompt messaage */
char* promptmsg() 
{
    /* Modified by Talal to include pid */
	char *prompt = ((char *) (malloc(sizeof(char)*50)));//4 bytes needed for pid int, 1 for null character in the end
	int pid = getpid();
	sprintf(prompt, "dsh(PID = %d)$ ", pid);
	return prompt;
}

int main() 
{

	init_dsh();
	DEBUG("Successfully initialized\n");
	/*Talal's code block starts here*/
	active_list = (job_t *)malloc(sizeof(job_t));
	if(!init_job(active_list)) {
	        	fprintf(stderr, "%s\n","malloc: no space for prologue header of active list of jobs");
			exit(EXIT_FAILURE);
        }
	job_t *j = (job_t *)malloc(sizeof(job_t));
	if(!init_job(j)) {
	        	fprintf(stderr, "%s\n","malloc: no space for prologue header for job list from command line");
			exit(EXIT_FAILURE);
        }
	/*End of Talal's code block*/
	while(1) {
		/*The block starting here was modified by Talal*/
		char* prompt;
		prompt = promptmsg();
			if(!(j->next = readcmdline(prompt))) {
				if (feof(stdin)) { /* End of file (ctrl-d) */
					fflush(stdout);
					printf("\n");
					exit(EXIT_SUCCESS);
					}
				continue; /* NOOP; user entered return or spaces with return */
			}
		free(prompt);//free the pointer to the prompt message
		/*End of modified block*/
			/* Only for debugging purposes to show parser output; turn off in the
			 * final code */
			//if(PRINT_INFO) print_job(j->next);

			/* Your code goes here */
			/* You need to loop through jobs list since a command line can contain ;*/
			/* Check for built-in commands */
			/* If not built-in */
				/* If job j runs in foreground */
				/* spawn_job(j,true) */
				/* else */
				/* spawn_job(j,false) */


		/* This block was added by Talal and Negatu */
		job_t *temp_job; //current job
		job_t *cur_job = j->next;
		while(cur_job != NULL) {
		//As per instructions, if a job contains a builtin command then it is the only command in the job
			if(!builtin_cmd(cur_job, cur_job->first_process->argc, cur_job->first_process->argv)){
				if (cur_job->bg){
					spawn_job(cur_job,false);
				}
				else{
					spawn_job(cur_job,true);
				}
				if(job_is_stopped(cur_job)){
					if(job_is_completed(cur_job)){
						temp_job = cur_job->next;
						delete_job(cur_job, j);
						cur_job = temp_job;
					}
					else{//job was stopped so add to active list
						temp_job = cur_job;
						cur_job = cur_job->next;
						append_to_joblist(temp_job, active_list);
					}


				}
				
			}
			else{//command was a built in command
				cur_job = cur_job->next;

			} 

		}
		
		/* Block ends here*/



    }
}
