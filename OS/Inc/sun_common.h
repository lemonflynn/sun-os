/*
 * sun operating system
 *
 * Copyright (c) Siemens AG, 2018, 2019
 *
 * Authors:
 *  Francisco flynn<lin.xu@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#ifndef __SUN_COMMON_H
#define __SUN_COMMON_H

#define TRUE    1
#define FALSE   0

enum sun_err{
    MSG_POOL_EMPTY  = -4,
    QUEUE_EMPTY_ERR = -3,
    QUEUE_FULL_ERR  = -2,
    PARA_ERR        = -1,
    NO_ERR          = 0,
};

#endif
