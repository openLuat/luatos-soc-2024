#ifndef __MD5_H__
#define __MD5_H__
#include "stddef.h"

int WinMD5(const unsigned char *input,
	size_t ilen,
	unsigned char output[16]);

#endif
