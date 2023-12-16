#ifndef ERROR_H
#define ERROR_H

typedef int State_t;

enum Error_t
    {
    Ok                    = 0,
    FileError             = 1,
    AllocationError       = 2,
    BufferOverflowError   = 3,
    CalculationError      = 4,
    HashNotCompare        = 5,
    SyntaxError           = 6,
    EmptyStack            = 7,
    BadCode               = 8,
    NotValidIndexError    = 9,
    EmptyList			  = 10,
    Exit                  = 11,
    NodeExist             = 12,
    NodeNotExist          = 13,
    CopyError             = 14,
    DifferentiationError  = 15
    };

#endif //ERROR_H
