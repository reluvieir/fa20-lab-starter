/* Include the system headers we need */
#include <stdlib.h>
#include <stdio.h>

/* Include our header */
#include "vector.h"

/* Define what our struct is */
struct vector_t {
    size_t size;
    int *data;
};

/* Utility function to handle allocation failures. In this
   case we print a message and exit. */
static void allocation_failed() {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
}

/* Bad example of how to create a new vector */
vector_t *bad_vector_new() {
    /* Create the vector and a pointer to it */
    vector_t *retval, v;
    retval = &v;

    /* Initialize attributes */
    retval->size = 1;
    retval->data = malloc(sizeof(int));
    if (retval->data == NULL) {
        allocation_failed();
    }

    retval->data[0] = 0;
    return retval;
}
// 函数返回时，v的生命周期就结束了，return retval返回的是一个悬空指针
// 虽然 v.data 指向的 heap 内存还存在，但保存这个指针的结构体 v 已经失效
// 如果函数需要返回vector_t*，通常要给结构体本身也分配堆内存

/* Another suboptimal way of creating a vector */
vector_t also_bad_vector_new() {
    /* Create the vector */
    vector_t v;

    /* Initialize attributes */
    v.size = 1;
    v.data = malloc(sizeof(int));
    if (v.data == NULL) {
        allocation_failed();
    }
    v.data[0] = 0;
    return v;
}   
// 这里return v返回，复制的是 size 和 data 的值

// return &v;  // 返回 v 的地址，之后还要通过地址访问已经失效的 v → 有问题
// return v;   // 返回 v 的结构体值，调用者接收这个值 → 合法

/* Create a new vector with a size (length) of 1
   and set its single component to zero... the
   RIGHT WAY */
vector_t *vector_new() {
    /* Declare what this function will return */
    vector_t *retval;

    /* First, we need to allocate memory on the heap for the struct */
    retval = malloc(sizeof(vector_t));

    /* Check our return value to make sure we got memory */
    if (retval == NULL) {
        allocation_failed();
    }

    /* Now we need to initialize our data.
       Since retval->data should be able to dynamically grow,
       what do you need to do? */
    retval->size = 1;
    retval->data = malloc(sizeof(int));

    /* Check the data attribute of our vector to make sure we got memory */
    if (retval->data == NULL) {
        free(retval);				//Why is this line necessary?
        allocation_failed();
    }

    /* Complete the initialization by setting the single component to zero */
    retval->data[0] = 0;

    /* and return... */
    return retval;
}

/* Return the value at the specified location/component "loc" of the vector */
int vector_get(vector_t *v, size_t loc) {

    /* If we are passed a NULL pointer for our vector, complain about it and exit. */
    if(v == NULL) {
        fprintf(stderr, "vector_get: passed a NULL vector.\n");
        abort();
    }

    /* If the requested location is higher than we have allocated, return 0.
     * Otherwise, return what is in the passed location.
     */
    if (loc < v->size) {
        return v->data[loc];
    } else {
        return 0;
    }
}

/* Free up the memory allocated for the passed vector.
   Remember, you need to free up ALL the memory that was allocated. */
void vector_delete(vector_t *v) {
    /* YOUR SOLUTION HERE */
    free(v->data);  // 因为 malloc 的时候分配的是 *int
    free(v);
}

/* Set a value in the vector. If the extra memory allocation fails, call
   allocation_failed(). */
void vector_set(vector_t *v, size_t loc, int value) {
    /* What do you need to do if the location is greater than the size we have
     * allocated?  Remember that unset locations should contain a value of 0.
     */
    
    /* YOUR SOLUTION HERE */
    if(v == NULL){
        fprintf(stderr, "vector_get: passed a NULL vector.\n");
        abort();
    }

    if(loc >= v->size){
        size_t new_size = loc + 1;

        int* new_data = (int*)realloc(v->data, new_size * sizeof(int));
        if(new_data == NULL){
            allocation_failed();
        }
        v->data = new_data;

        // 新增加的位置要初始化
        for(size_t i=v->size;i<new_size;i++){
            v->data[i] = 0;
        }

        v->size = new_size;
    }

    v->data[loc] = value;
}