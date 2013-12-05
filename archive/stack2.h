/*
 * This package to provides a stack of integers.  It supports multiple
 * stacks with the following interface:
 *
 * typedef ... int_stack;
 *        Type name for declaring stack variables.
 *
 * void int_stack_init(int_stack *s);
 *        Initialize a stack object which s points to. No stack should be
 *        used until it is initialized. Once initialized, it should not
 *        be initialized again, unless it is first uninitialized with
 *        int_stack_tini. After initialization, the stack is empty.
 *        
 * int int_stack_push(int_stack *s, int val);
 *        Push the integer val onto the stack. Int_stack_push returns a
 *        value indicating success: if it worked, return nonzero. If it
 *        failed, return 0.
 *        
 * int int_stack_pop(int_stack *s, int *val);
 *        Pop the top integer off the stack. If the stack is not empty,
 *        int_stack_pop removes the top item from the int_stack which s
 *        points to, places it into the location denoted by val, and
 *        returns a non-zero value to indicate success. If the stack is
 *        empty, int_stack_pop returns 0 and does nothing else.
 *        
 * int int_stack_top(const int_stack *s);
 *        Return the top item of stack s, if any. If s is empty, return
 *        0.
 *        
 * int int_stack_empty(const int_stack *s);
 *        Tell if the stack is empty
 *        
 * void int_stack_tini(int_stack *s);
 *        Destroy the stack object denoted by s. This frees any
 *        dynamically-allocated memory associated with the stack. 
 *        
 * size_t int_stack_mem_usage(void);
 *        Return the total amount of dynamically-allocated memory
 *        currently in use by all int_stacks.
 */

#ifndef _STACK2_H_
#define _STACK2_H_

#include <stdlib.h>

/* Type of a stack node. */
struct node_struct {
	int data;
	struct node_struct *next;
};

/* Type of a stack object. */
typedef struct node_struct *int_stack;

/* Initialize a stack object. */
void int_stack_init(int_stack * s);

/* Push the integer val onto the stack. */
int int_stack_push(int_stack * s, int val);

/* Pop the top integer off the stack.  Return success. */
int int_stack_pop(int_stack * s);

/* Return the top item of stack s, if any, else 0. */
int int_stack_top(const int_stack * s);

/* Tell if the stack is empty. */
int int_stack_empty(const int_stack * s);

/* Destroy the stack object denoted by s.  This frees any dynamically-allocated
   memory associated with the stack. */
void int_stack_tini(int_stack * s);

/* Return the total amount of dynamically-allocated memory
   currently in use by all int_stacks. */
size_t int_stack_mem_usage(void);

#endif
