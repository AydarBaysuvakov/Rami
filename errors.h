#ifndef ERROR_H
#define ERROR_H

typedef int State_t;

enum Error_t
    {
    Ok                    = 0,
    FileError             = 1,
    AllocationError       = 2,
    SyntaxError           = 3,
    CalculationError      = 4,
    BadCode               = 5,
    Exit                  = 6,
    NodeExist             = 7,
    NodeNotExist          = 8,
    CopyError             = 9,
    DifferentiationError  = 10,
    NotAssigment          = 11
    };

#endif //ERROR_H
