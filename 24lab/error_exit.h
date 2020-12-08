#pragma once

#define SUCCESS 0
#define ERRNO_SET 0
#define ERROR_EXIT 1

void error_exit(const char *const msg, int error);
