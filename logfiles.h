#ifndef LOGFILE_H
#define LOGFILE_H

const int     LOG_FILE_NAME_LENGTH      = 40;
const int     LOG_FILE_PREFIX_LENGTH    = 12;
const int     LOG_FILE_OBJ_NAME_LENGTH  = 12;
const int     LOG_FILE_EXTENSION_LENGTH = 8;
const int     COMMAND_LENGTH            = 100;

Error_t LogFileInit(FILE **file, const char* prefix, const char* name, const char* extension, char* dest = nullptr);

#endif //LOGFILE_H
