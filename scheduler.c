/**********************************************************************/
/*                                                                    */
/* Program Name: Scheduler - Simlulate an OS scheduler in Linux       */
/* Author        Juan Sulse                                           */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: March 14, 2020                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge this assignment is my own first time work.                */
/* I pledge I did not copy or try to copy work from the Internet.     */
/* I pledge I did not copy or try to copy work from any student.      */
/* I pledge I did not copy or try to copy work from any where else.   */
/* I pledge the only person I asked for help from was my teacher.     */
/* I pledge I did not attempt to help any student on this assignment. */
/* I understand if I violate this pledge I will receive a 0 grade.    */
/*                                                                    */
/*                                                                    */
/*                      Signed: ______________Juan Sulse_____________ */
/*                                           (signature)              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* This program simulates the environment in Unix where new processes */
/* are continually arriving, existing process are vying for the CPU,  */
/* processes are using their given quantum (CPU bound) or blocking    */
/* because of I/O operations, and processes are terminating when      */
/* their work is finished.                                            */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define COURSE_NUMBER     "CS326"   /* PCC assigned course number     */
#define LAST_NAME         "Sulse"   /* The programmer's last name     */
#define MAX_PID           99        /* Maximum PID value              */
#define MIN_PID           1         /* Minimum PID value              */
#define LIST_HEADER       MIN_PID-1 /* Lowest possible PID number     */
#define LIST_TRAILER      MAX_PID+1 /* Highest possible PID number    */
#define HEAD_ALLOC_ERR    1         /* Cannot allocate header memory  */
#define TRAILER_ALLOC_ERR 2         /* Cannot allocate trailer memory */
#define ID_ALLOC_ERR      0         /* Cannot allocate new PID memory */

/**********************************************************************/
/*                         Program Structures                         */
/**********************************************************************/
/* The process table                                                  */
struct process_table
{
   int PID,          /* The process ID number                         */
       CPU_used,     /* Total clock ticks the process used so far     */
       MAX_time,     /* Maximum CPU time needed by the process        */
       PRI,          /* The priority of the process                   */
       QUANTUM_USED, /* Amount of quantum used by the process         */
       BLK_TIME,     /* Clock ticks used until the process blocks     */
       WAIT_TKS;     /* Total clock ticks the ready process waited    */

   char STATE;       /* The state of the process                      */
   struct process_table *p_next_process;
                     /* Points to the next process */
};
typedef struct process_table PROCESS_TABLE;

/**********************************************************************/
/*                         Function Prototypes                        */
/**********************************************************************/
PROCESS_TABLE *create_process_table(int *PID, int *num_processes);
   /* Create a process table with header, 5 processes, and trailer    */
PROCESS_TABLE *create_process(int PID);
   /* Create and initalize a process                                  */
void sort_processes(PROCESS_TABLE *p_process_table, int num_processes);
   /* Sort the process table by priority in ascending order           */
void print_table(PROCESS_TABLE *p_process_table, int PID,
                                                     int num_processes);
   /* Print the process table                                         */
void terminate_process(PROCESS_TABLE *p_running_process,
                       PROCESS_TABLE *p_process_table);
   /* Terminate a process                                             */
void swap_priority(PROCESS_TABLE *p_process_table);
   /* Swap priorty of negative numbers                                */
void preempt_process(PROCESS_TABLE *p_running_process, int PID);
   /* Preempt the running process                                     */
int recalculate_priority(PROCESS_TABLE *p_running_process);
   /* Recalculate the running process's priority                      */
PROCESS_TABLE *schedule_next_process(PROCESS_TABLE *p_process_table);
   /* Schedule the next process                                       */
PROCESS_TABLE *next_ready_process(PROCESS_TABLE *p_process_table);
   /* Find the next ready process                                     */
void p_update_processes(PROCESS_TABLE *p_process_table);
   /* Update the process table                                        */
void unblock_process(PROCESS_TABLE *p_process_table);
   /* Unblock a blocked process                                       */
void insert_process(PROCESS_TABLE *p_process_table, int *PID,
                                                    int *num_processes);
   /* Insert a new process into the process table                     */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{
   PROCESS_TABLE *p_process_table, /* Points to the process table     */
                 *p_running_process = NULL;
                                   /* Points to the running process   */
   int           num_processes      = 0,
                                   /* Number of processes in table    */
                 PID;              /* Process ID number               */

   /* Create and initalize the process table with header, 5           */
   /* processes, and trailer                                          */
   p_process_table = create_process_table(&PID, &num_processes);

   /* Loop processing clock ticks for 99 processes                    */
   while(PID <= 100)
   {
      /* Schedule the next ready process and print out the process    */
      /* table if there are no running processes                      */ 
      if(p_running_process == NULL)
      {
         if(next_ready_process(p_process_table) != NULL)
         {
            printf                  ("\n\nBEFORE SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
            p_running_process =
               schedule_next_process(p_process_table);
            swap_priority           (p_process_table);
            sort_processes          (p_process_table, num_processes);
            swap_priority           (p_process_table);
            printf                  ("\n\nAFTER SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
         }
      }
      else
      {
         /* Terminate the runNing process if it has reached its       */
         /* maximum CPU time, schedule the next ready process, and    */
         /* print out the process table                               */
         if(p_running_process->CPU_used == p_running_process->MAX_time)
         {
            printf                  ("\n\nBEFORE SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
            terminate_process       (p_running_process, p_process_table);
            num_processes--;
            p_running_process =
               schedule_next_process(p_process_table);
            swap_priority           (p_process_table);
            sort_processes          (p_process_table, num_processes);
            swap_priority           (p_process_table);
            printf                  ("\n\nAFTER SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
         }
         /* Preempt the runNing process if it blocked or used all of  */
         /* its quantum, recalculate its priority, place it at the    */
         /* end of the process table, based on its new priority, and  */
         /* print out and update the process table                    */
         else if(p_running_process->QUANTUM_USED ==
                 p_running_process->BLK_TIME)
         {
            printf                  ("\n\nBEFORE SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
            preempt_process         (p_running_process, PID);
            swap_priority           (p_process_table);
            sort_processes          (p_process_table, num_processes);
            swap_priority           (p_process_table);
            p_running_process =
               schedule_next_process(p_process_table);
            printf                  ("\n\nAFTER SCHEDULING CPU: ");
            print_table             (p_process_table, PID, num_processes);
         }
      }
      p_update_processes(p_process_table);

      /* Make ready any blocked process that has become unblocked     */
      unblock_process   (p_process_table);

      /* Place any newly arrived processes into the process table     */
      if(rand() % 5 == 0)
         if(num_processes < 10)
         {
            insert_process(p_process_table, &PID, &num_processes);
            sort_processes(p_process_table, num_processes);
         }
   }
   printf("\n");
   return 0;
}

/**********************************************************************/
/*     Create a process table with header, 5 processes, and trailer   */
/**********************************************************************/
PROCESS_TABLE *create_process_table(int *PID, int *num_processes)
{
   PROCESS_TABLE *p_new_process, /* Points to every process           */
                 *p_new_process_table; 
                                 /* Points to the newly created       */
                                 /* process table                     */

   /* Create the header node                                          */
   if((p_new_process =
      (PROCESS_TABLE *) malloc(sizeof(PROCESS_TABLE))) == NULL)
   {
      printf("\nError #%d occurred in create_process_table()",
                                                        HEAD_ALLOC_ERR);
      printf("\nCannot allocate memory for the list header");
      printf("\nThe program is aborting");
      exit  (HEAD_ALLOC_ERR);
   }
   p_new_process->PID  = LIST_HEADER;
   p_new_process_table = p_new_process;

   /* Create 5 processes and insert them into the process table       */
   for(*PID = 1; *PID < 6; (*PID)++)
      if(*PID > LIST_HEADER)
      {
         p_new_process->p_next_process = create_process(*PID);
         (*num_processes)++;
         p_new_process                 = p_new_process->p_next_process;
      }

   /* Create the trailer node                                         */
   if((p_new_process->p_next_process =
      (PROCESS_TABLE *) malloc(sizeof(PROCESS_TABLE))) == NULL)
   {
      printf("\nError #%d occured in create_process_table()",
                                                     TRAILER_ALLOC_ERR);
      printf("\nCannot allocate memory for the list trailer");
      printf("\nThe program is aborting");
      exit  (TRAILER_ALLOC_ERR);
   }
   p_new_process->p_next_process->PID            = LIST_TRAILER;
   p_new_process->p_next_process->p_next_process = NULL;

   return p_new_process_table;
}

/**********************************************************************/
/*                   Create and initalize a process                   */
/**********************************************************************/
PROCESS_TABLE *create_process(int PID)
{
   PROCESS_TABLE *p_new_PID; /* Points to the newly created process   */

   if((p_new_PID = (PROCESS_TABLE *) malloc(sizeof(PROCESS_TABLE))) == NULL)
   {
      printf("\nError #%d occured in create_process_()", ID_ALLOC_ERR);
      printf("\nCannot allocate memory for the list trailer");
      printf("\nThe program is aborting");
      exit  (ID_ALLOC_ERR);
   }
   p_new_PID->PID          = PID;
   p_new_PID->CPU_used     = 0;
   p_new_PID->MAX_time     = rand() % 18 + 1;
   p_new_PID->STATE        = 'R';
   p_new_PID->PRI          = 0;
   p_new_PID->QUANTUM_USED = 0;
   p_new_PID->WAIT_TKS     = 0;

   if(rand() % 3)
      p_new_PID->BLK_TIME = rand() % 5 + 1;
   else
      p_new_PID->BLK_TIME = 6;

   return p_new_PID;
}

/**********************************************************************/
/*        Sort the process table by priority in ascending order       */
/**********************************************************************/
void sort_processes(PROCESS_TABLE *p_process_table, int num_processes)
{
   PROCESS_TABLE *p_PID = p_process_table,
                              /* Points to every process PID          */
                 *p_temp_PID; /* Temporary pointer for swap           */
   int           sort_count;  /* Count of sort iterations             */

   for(sort_count = 0; sort_count <= num_processes; sort_count++)
   {
      while(p_PID->p_next_process->p_next_process->PID < LIST_TRAILER)
      {
         if(p_PID->p_next_process->PRI >
            p_PID->p_next_process->p_next_process->PRI)
         {
            p_temp_PID
               = p_PID->p_next_process->p_next_process;
            p_PID->p_next_process->p_next_process
               = p_PID->p_next_process->p_next_process->p_next_process;
            p_temp_PID->p_next_process
               = p_PID->p_next_process;
            p_PID->p_next_process
               = p_temp_PID; 
         }
         p_PID = p_PID->p_next_process;
      }
      p_PID = p_process_table; 
   }
   return;
}

/**********************************************************************/
/*                       Print the process table                      */
/**********************************************************************/
void print_table(PROCESS_TABLE *p_process_table, int PID, int num_processes)
{
   if(p_process_table->p_next_process->PID < LIST_TRAILER)
   {
      printf("Next PID = %d, ", PID);
      printf("Number of Processes = %d", num_processes);
      printf("\n PID   CPU Used   MAX Time   STATE   PRI   ");
      printf("QUANTUM USED   BLK TIME   WAIT TKS");

      while(p_process_table = p_process_table->p_next_process,
            p_process_table->PID < LIST_TRAILER)
      printf("\n%4d %7d %10d %8c %7d %9d %12d %11d",
         p_process_table->PID, p_process_table->CPU_used,
         p_process_table->MAX_time, p_process_table->STATE,
         p_process_table->PRI, p_process_table->QUANTUM_USED,
         p_process_table->BLK_TIME, p_process_table->WAIT_TKS);
   }
}

/**********************************************************************/
/*                         Terminate a process                        */
/**********************************************************************/
void terminate_process(PROCESS_TABLE *p_running_process,
                       PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_previous_process = p_process_table,
                                     /* Points to previous PID        */
                 *p_current_process; /* Points to each PID            */

   while(p_current_process = p_previous_process->p_next_process,
         p_current_process->PID < LIST_TRAILER)
   {
      if(p_current_process->PID == p_running_process->PID)
      {
         p_previous_process->p_next_process
                            = p_current_process->p_next_process;
         p_current_process  = p_previous_process->p_next_process;
      }
      else
         p_previous_process = p_current_process;
   }
      return;
}

/**********************************************************************/
/*                  Swap priorty of negative numbers                  */
/**********************************************************************/
void swap_priority(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
         p_process_table->PRI < 0)
      switch(p_process_table->PRI)
      {
         case -5:
            p_process_table->PRI = -1;
            break;
         case -4:
            p_process_table->PRI = -2;
            break;
         case -2:
            p_process_table->PRI = -4;
            break;
         case -1:
            p_process_table->PRI = -5;
            break;
      }
   return;
}
/**********************************************************************/
/*                     Preempt the running process                    */
/**********************************************************************/
void preempt_process(PROCESS_TABLE *p_running_process, int PID)
{
   if(p_running_process->QUANTUM_USED == 6)
   {
      p_running_process->STATE = 'R';
      p_running_process->PRI   = recalculate_priority(p_running_process);
   }
   else
   {
      p_running_process->STATE = 'B';
      p_running_process->PRI   = -(recalculate_priority(p_running_process));
   }
   p_running_process->QUANTUM_USED = 0;
   return;
}

/**********************************************************************/
/*             Recalculate the running process's priority             */
/**********************************************************************/
int recalculate_priority(PROCESS_TABLE *p_running_process)
{
   return (int)(((float)(abs(p_running_process->PRI) +
                         p_running_process->QUANTUM_USED) / 2.0f) + 0.5f);
}

/**********************************************************************/
/*                      Schedule the next process                     */
/**********************************************************************/
PROCESS_TABLE *schedule_next_process(PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_process = NULL; /* Points to next process to      */
                                    /* schedule                       */

   while(p_process_table != NULL && p_process_table->PID < LIST_TRAILER)
   {
      p_process_table = p_process_table->p_next_process;
      if(p_process_table->STATE == 'R')
      {
         p_process_table->STATE = 'N';
         p_process              = p_process_table;
         p_process_table        = NULL;
      }   
   }
   return p_process;
}

/**********************************************************************/
/*                     Find the next ready process                    */
/**********************************************************************/
PROCESS_TABLE *next_ready_process(PROCESS_TABLE *p_process_table)
{
   PROCESS_TABLE *p_next_ready_process; /* Points to next ready       */
                                        /* process                    */

   while(p_process_table = p_process_table->p_next_process,
         p_process_table->PID < LIST_TRAILER)
      if(p_process_table->STATE == 'R');
         p_next_ready_process = p_process_table;
   return p_next_ready_process;
}

/**********************************************************************/
/*                      Update the process table                      */
/**********************************************************************/
void p_update_processes(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
         p_process_table->PID < LIST_TRAILER)
   {
      if(p_process_table->STATE == 'N')
      {
         p_process_table->QUANTUM_USED++;
         p_process_table->CPU_used++;
      }
      else
         p_process_table->WAIT_TKS++;
   }
   return;
}

/**********************************************************************/
/*                      Unblock a blocked process                     */
/**********************************************************************/
void unblock_process(PROCESS_TABLE *p_process_table)
{
   while(p_process_table = p_process_table->p_next_process,
         p_process_table->PID < LIST_TRAILER)
      if(p_process_table->STATE == 'B')
         if(rand()%20 == 0)
            p_process_table->STATE = 'R';
   return;
}

/**********************************************************************/
/*             Insert a new process into the process table            */
/**********************************************************************/
void insert_process(PROCESS_TABLE *p_process_table, int *PID,
                                                     int *num_processes)
{
   PROCESS_TABLE *p_new_process; /* Points to newly created process   */

   while(p_process_table = p_process_table->p_next_process,
         p_process_table->PID < LIST_TRAILER)
      if(p_process_table->p_next_process->PID == LIST_TRAILER)
      {
         p_new_process                   = create_process(*PID);
         p_new_process->p_next_process   = p_process_table->p_next_process;
         p_process_table->p_next_process = p_new_process;
         p_process_table                 = p_process_table->p_next_process;
         (*num_processes)++;
         (*PID)++;
      }
   return;
}
