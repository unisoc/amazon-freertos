/*
 * Copyright (c) 2015, Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <sys/stat.h>

int _isatty(int file)
{
	return 1;
}

int _kill(int i, int j)
{
	return 0;
}

int _getpid(void)
{
	return 0;
}
