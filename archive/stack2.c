
#include <stdio.h>
#include <stdlib.h>
#include "stack2.h"

/* Current amount of memory used by all stacks. */
static size_t tot_mem = 0;

/* Initialize a stack object. */
void int_stack_init(int_stack * s)
{
	*s = NULL;
}

/* Push the integer val onto the stack. */
int int_stack_push(int_stack * s, int val)
{
	/* Get a new node. */
	struct node_struct *newnode =
	    (struct node_struct *)malloc(sizeof(struct node_struct));

	/* If malloc failed, let 'em know. */
	if (newnode == NULL)
		return 0;

	/* Increment usage count. */
	tot_mem += sizeof(struct node_struct);

	/* Fill and link. */
	newnode->data = val;
	newnode->next = *s;
	*s = newnode;

	/* Worked. */
	return 1;
}

/* Pop the top integer off the stack.  Return success. */
int int_stack_pop(int_stack * s)
{
	struct node_struct *zombie;	/* Points to node which is *
					 * about to be freed.      */
	/* If no such node, say so. */
	if (*s == NULL)
		return 0;

	/* Okay.  Get that node out of that list. */
	zombie = *s;
	*s = (*s)->next;

	/* Get the data out of the node, lose the node, and reduce
	   the total allocation figure. */
	int val = zombie->data;
	free(zombie);
	tot_mem -= sizeof(struct node_struct);

	return val;
}

/* Return the top item of stack s, if any, else 0. */
int int_stack_top(const int_stack * s)
{
	if (*s == NULL)
		return 0;
	else
		return (*s)->data;
}

/* Tell if the stack is empty. */
int int_stack_empty(const int_stack * s)
{
	return *s == NULL;
}

/* Destroy the stack object denoted by s.  This frees any dynamically-allocated
   memory associated with the stack. */
void int_stack_tini(int_stack * s)
{
	struct node_struct *zombie,	/* Node about to be freed. */
	*scan;			/* Scan list of nodes. */

	/* Get rid of all the nodes, and count 'em. */
	scan = *s;
	while (scan != NULL) {
		/* Remember where we are before we split. */
		zombie = scan;
		scan = scan->next;

		/* Kill it, and and reduce the total used. */
		free(zombie);
		tot_mem -= sizeof(struct node_struct);
	}

	*s = NULL;
}

/* Return the total amount of dynamically-allocated memory
   currently in use by all int_stacks. */
size_t int_stack_mem_usage(void)
{
	return tot_mem;
}

/*
int main(){
	struct node_struct *newnode = 
                (struct node_struct *)malloc(sizeof(struct node_struct));
    newnode = NULL; 
    int val = 2;
    int_stack_push(&newnode, val); 
    
    printf("\n %i one node \n",(int)tot_mem); 
    
    int i; 
    for (i = 0; i < 1000; i++){ 
		int_stack_push(&newnode, i ); 
	}

    printf("\n %i all nodes added \n",(int)tot_mem); 
         
    while(!int_stack_empty(&newnode)){
		int curr;
		curr  = int_stack_pop(&newnode); 
		//printf("%i ",curr); 
	}
		
	int j = 9; 
	int *ptr; 
	ptr = &j; 
	printf("\n %i all nodes popped\n",(int)tot_mem); 
    return 1;
}
                
	*/
