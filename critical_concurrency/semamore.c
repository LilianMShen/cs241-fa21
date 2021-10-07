/**
 * critical_concurrency
 * CS 241 - Fall 2021
 */

#include "semamore.h"

/**
 * Initializes the Semamore. Important: the struct is assumed to have been
 * allocated by the user.
 * Example:
 * 	Semamore *s = malloc(sizeof(Semamore));
 * 	semm_init(s, 5, 10);
 *
 */
void semm_init(Semamore *s, int value, int max_val) {
    /* Your code here */
    s->value = value;
    s->max_val = max_val;
}

/**
 *  Should block when the value in the Semamore struct (See semamore.h) is at 0.
 *  Otherwise, should decrement the value.
 */
void semm_wait(Semamore *s) {
    /* Your code here */

    if (s->value == 0) {
        return;
    }

    s->value -= 1;
}

/**
 *  Should block when the value in the Semamore struct (See semamore.h) is at
 * max_value.
 *  Otherwise, should increment the value.
 */
void semm_post(Semamore *s) {
    /* Your code here */

    if (s->value == s->max_val) {
        return;
    }
    
    s->value += 1;
}

/**
 * Takes a pointer to a Semamore struct to help cleanup some members of the
 * struct.
 * The actual Semamore struct must be freed by the user.
 */
void semm_destroy(Semamore *s) {
    /* Your code here */
}
