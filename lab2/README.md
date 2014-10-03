/**********************************************
 * Please DO NOT MODIFY the format of this file
 **********************************************/

/*************************
 * Team Info & Time spent
 *************************/
/*
	Edit this to list all group members and time spent.
	Please follow the format or our scripts will break. 
*/

	Name1: Negatu Asmamaw
	NetId1: nna4@duke.edu
	Time spent: 15 hours

	Name2: Talal Javad Qadri
	NetId2: 
	Time spent: 15 hours



/******************
 * Files to submit
 ******************/

	dsh.c 	// Header file is not necessary; other *.c files if necessary
	README	// This file filled with the lab implementation details

/************************
 * Implementation details
 *************************/

/* 
 * This section should contain the implementation details and a overview of the
 * results. 

 * You are expected to provide a good README document including the
 * implementation details. In particular, you can use pseudocode to describe your
 * implementation details where necessary. However that does not mean to
 * copy/paste your C code. Specifically, you should summarize details corresponding 
 * to (1) multiple pipelines (2) job control (3) logging (3) .c commands 
 * compilation and execution. We expect the design and implementation details to 
 * be at most 1-2 pages.  A plain textfile is requested. 

 * In case of lab is limited in some functionality, you should provide the
 * details to maximize your partial credit.  
 * */



(1)Multiple Pipelines
For multiple pipelines, we created a potential pipe for each process first. 
Then we check if that process is in a pipeline sequence i.e if it need to read from or right to a pipeline. 
For the case that it needs to write, we write to the pipe and we made the parent hold the value of the read end of this pipe so that the next process in the pipeline can read from it. 
And thus, process that need to read from a pipeline recieve a pipeline from the parent to read from. 

(2)Job control 

(3)logging

(4)Built in commands

(5)Input Output redirection
For the input output redirection, most of the job is done for us by the parser. 
The parser renders a process_t object that contains the input and output files that are associated with the process. 
If those files do exist, we just make our stdin and stdout read and write to them. 

/************************
 * Feedback on the lab
 ************************/
This was a great lab experience. This lab is different from other labs in other Comp Sci classes we have taken so far.
It emphasizes on implemetation of design to handle porcesses. We are used to many algorithms and front end user projects in other labs. 
We greatly enjoyed this lab. It was a very good learning experience. 


/************************
 * References
 ************************/
We mainly refered to the recommended texts. In addition, we disscussed main concepts with TAs and students. 
We also used many web sources and the man command to learn about different system calls and their functionalities. 
