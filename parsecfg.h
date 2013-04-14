/*
 * parsecfg.h, parse a "DragonEx" image config file.
 * Copyright (c) 2013, Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and COPYING for more details.
 */

#ifndef PARSECFG_H
#define PARSECFG_H

#include <stdio.h>

#define MAXIDLEN	32
#define MAXLINELEN	256

typedef enum valtype_e {
     VT_NUMBER,
     VT_STRING,
     VT_LISTITEM,
} valtype_t;

typedef struct variable_s {
    const char *name;
    valtype_t type;
    union {
        char *str;
        long number;
        struct variable_s *items;
    };
    struct variable_s *next;
} variable_t;

typedef struct group_s {
    const char *name;
    variable_t *vars;
    struct group_s *next;
} group_t;

variable_t *cfg_find_var(const char *name, group_t *head);
group_t *cfg_find_group(const char *name, group_t *head);
long cfg_get_number(const char *name, group_t *head);
long cfg_count_vars(group_t *head);
group_t *cfg_load(FILE *fp);
void cfg_free(group_t *head);

#endif /* PARSECFG_H */
