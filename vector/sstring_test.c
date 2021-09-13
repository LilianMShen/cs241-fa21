/**
 * vector
 * CS 241 - Fall 2021
 */
#include "sstring.h"
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    // Testing basic constructors
    printf("Testing basic constructors... ");
    sstring *str = cstr_to_sstring("testing123");
    char *cstr = sstring_to_cstr(str);
    assert(strcmp(cstr, "testing123") == 0);
    // Destroy memory associated with return value
    string_destructor((void*)cstr);
    printf("done\n");

    // Testing append
    printf("Testing basic append... ");
    sstring *str2 = cstr_to_sstring(". okay now what?");
    assert(sstring_append(str, str2) == 26);
    char *append1 = sstring_to_cstr(str);
    char *append2 = sstring_to_cstr(str2);
    assert(strcmp(append1, "testing123. okay now what?") == 0);
    assert(strcmp(append2, ". okay now what?") == 0);
    string_destructor((void*)append1);
    string_destructor((void*)append2);
    printf("done\n");

    // Test destructor
    printf("Testing destructor... ");
    sstring_destroy(str2);
    printf("done\n");

    // Test slice
    printf("Testing slice... ");
    char *slice_result = sstring_slice(str, 1, 3);
    assert(strcmp(slice_result, "es") == 0);
    string_destructor((void*)slice_result);
    printf("done\n");

    // Test split
    printf("Testing split... ");
    vector *split_result = sstring_split(str, '.');
    
    assert(strcmp(vector_get(split_result, 0), "testing123") == 0);
    assert(strcmp(vector_get(split_result, 1), " okay now what?") == 0);
    vector_destroy(split_result);

    split_result = sstring_split(str, 'x');
    assert(strcmp(vector_get(split_result, 0), "testing123. okay now what?") == 0);
    vector_destroy(split_result);

    split_result = sstring_split(str, '?');
    assert(strcmp(vector_get(split_result, 1), "") == 0);
    vector_destroy(split_result);

    printf("done\n");

    // Test substitute
    printf("Testing substitute... ");
    assert(sstring_substitute(str, 0, "lol", "wut?") == -1);
    assert(sstring_substitute(str, 0, "testing", "workkking") == 0);

    char* sub_result = sstring_to_cstr(str);
    assert(strcmp(sub_result, "workkking123. okay now what?") == 0);
    string_destructor((void*)sub_result);
    assert(sstring_substitute(str, 1, "workkking", "slacccccing") == -1);
    assert(sstring_substitute(str, 1, "kkk", "cc") == 0);
    sub_result = sstring_to_cstr(str);
    assert(strcmp(sub_result, "worccing123. okay now what?") == 0);
    string_destructor((void*)sub_result);

    assert(sstring_substitute(str, 5, "okay", "ummm") == 0);
    sub_result = sstring_to_cstr(str);
    assert(strcmp(sub_result, "worccing123. ummm now what?") == 0);
    string_destructor((void*)sub_result);
    printf("done\n");
    
    

    sstring_destroy(str);
    str = NULL;

    printf("All tests passed!\n");

    return 0;
}
