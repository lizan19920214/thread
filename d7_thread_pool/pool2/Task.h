#ifndef _TASK_H_
#define _TASK_H_

#include <iostream>
#include <string>

using callback = void (*)(void *);

struct Task
{
    callback function;
    void* arg;
    Task() : function(nullptr), arg(nullptr) {}
    Task(callback f, void* arg_f) : function(f), arg(arg_f) {}
    const int getArg() { return *(int*)arg; }
};

#endif // _TASK_H_