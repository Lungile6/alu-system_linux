#include <stdio.h>
#include <pthread.h>
#include "multithreading.h"

/**
 * thread_entry - Function to be executed by the thread.
 * @arg: The argument passed to the thread, expected to be a string.
 *
 * Return: Always returns NULL.
 */
void *thread_entry(void *arg)
{
    /* Print the string passed as argument */
    printf("%s\n", (char *)arg);
    
    /* End the thread */
    pthread_exit(NULL);

    return (NULL); /* Return NULL to satisfy the function signature */
}
