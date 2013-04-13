/*
 * parsecfg.c, parse a "DragonEx" image config file.
 * Copyright (c) 2013, Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and COPYING for more details.
 */

/*
 * roughly the syntax is as follows:
 *
 * configfile := { <configline> }
 * configline := <group> | <groupitem>
 * group := '[' <identifier> ']'
 * groupitem := <listitem> | <keyvalue>
 * listitem := '{' { <keyvalue>> ',' } '}'
 * keyvalue :=  <identifier> '=' <expr>
 * expr := <number> | <strexpr>
 * strexpr := [<identifier> | <string>] { '..' [<identifier> | <string>] }
 *
 * ';' on a line is meant as comment-to-eol marker
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define false 0
#define true 1

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

static int lineno;

variable_t*
cfg_find_var(const char *name, group_t *head) {

    for (; head; head = head->next) {
        variable_t *var;
        for (var = head->vars; var; var=var->next)
            if (var->name && strcmp(name, var->name) == 0)
                return var;
    }

    return NULL;
}

static variable_t*
new_variable(const char *name, valtype_t type) {
    variable_t *var = malloc(sizeof(variable_t));
    if (var) {
        memset(var, 0, sizeof(*var));
        var->type = type;
        if (name)
            var->name = strdup(name);
    }
    return var;
}

static int
add_to_listitem(variable_t *listitem, variable_t *sub) {
    variable_t *last = listitem->items;
    if (last == NULL) {
        listitem->items = sub;
        return 0;
    }

    while(last->next != NULL)
        last = last->next;

    last->next = sub;
    return 0;
}

static int
add_var_to_group(group_t *group, variable_t *var) {
    variable_t *last = group->vars;
    if (last == NULL) {
        group->vars = var;
        return 0;
    }

    while(last->next != NULL)
        last = last->next;

    last->next = var;
    return 0;
}

static char*
skip_whitespace(char **ptr) {
    char *p = *ptr;
    while(*p && isspace(*p)) ++p;
    if (*p == ';')
        while(*p) ++p;

    *ptr = p;
    return p;
}

static char*
parse_identifier(char **ptr, char *buf, size_t buflen)
{
    char *p = *ptr;
    while(*p && (isalnum(*p) || *p == '_')) {
        if (buflen > 0) {
            *buf++ = *p;
            --buflen;
        }
        ++p;
    }

    *buf = '\0';
    *ptr = p;

    return buf;
}

/* Parse a string; assuming *ptr == '"' or '\'' */
static char*
parse_string(char **ptr, char *buf, size_t buflen)
{
    char *s = buf, *p = *ptr;
    char delim = *p++;

    while(*p && *p != delim) {
       if (*p == '\\' && p[1])
           ++p;
       if (buflen > 0) {
            *buf++ = *p;
            --buflen;
        }

        p++;
    }
    if (*p == delim)
        ++p;
    *buf = '\0';
    *ptr = p;
    return s;
}

static variable_t*
parse_expr(char **ptr, variable_t *var, group_t *head) {
    char value[MAXLINELEN];
    char *buf = value;
    size_t buflen = sizeof(value);
    int is_str = false;
    char *p = *ptr;
    long number;

    do {
        skip_whitespace(&p);
        if (!is_str && isdigit(*p)) {
            char *end;
            /* parse number */
            number = strtol(p, &end, 0);
            p = end;
        } else if (*p == '"' || *p == '\'') {
            char *s = parse_string(&p, buf, buflen);
            buf += strlen(s);
            buflen -= strlen(s);
            is_str = true;
        } else if (isalpha(*p)) {
            /* parse identifier */
            char ident[MAXIDLEN];
            variable_t *var;
            parse_identifier(&p, ident, sizeof(ident));
            var = cfg_find_var(ident, head);
            if (var  == NULL || var->type == VT_STRING) {
                const char *str = var ? var->str : ident;
                strncpy(buf, str, buflen);
                buflen -= strlen(str);
                buf += strlen(str);
                is_str = true;
            } else if (var && var->type == VT_NUMBER) {
                buflen -= snprintf(buf, buflen, "0x%lx", var->number);
            } else {
                fprintf(stderr, "%s:%d: listitem used in expression!\n", __func__, lineno);
                break;
            }
        } else {
            fprintf(stderr, "%s:%d: unknown token!\n", __func__, lineno);
            break;
        }
        skip_whitespace(&p);
        if (is_str && *p == '.' && p[1] == '.') {
            p += 2;
        } else {
            break;
        }
    } while( true );

    if (is_str) {
        var->type = VT_STRING;
        var->str = strdup(value);
    } else {
        var->type = VT_NUMBER;
        var->number = number;
    }

    *ptr = p;

    return var;
}

static group_t*
parse_group(char *line) {
    char *grp, *p = line +1;
    group_t *group;

    skip_whitespace(&p);
    grp = p;
    while(*p && !isspace(*p) && *p != ']') ++p;
    if (*p)
        *p = '\0';

    group = malloc(sizeof(group_t));
    if (group) {
        memset(group, 0, sizeof(group_t));
        group->name = strdup(grp);
    }

    return group;
}

static variable_t*
parse_keyvalue(char **ptr, group_t *head) {
    char *p = *ptr;
    char name[MAXIDLEN];
    variable_t* var;

    parse_identifier(&p, name, sizeof(name));

    skip_whitespace(&p);
    if (*p != '=') {
        fprintf(stderr, "%s:%d: No assignment detected!\n", __func__, lineno);
        return NULL;
    } else {
        p++;
    }
    skip_whitespace(&p);

    /* create variable, assume string */
    var = new_variable(name, VT_STRING);
    /* parse value of variable */
    parse_expr(&p, var, head);

    *ptr = p;
    return var;
}

static variable_t*
parse_listitem(char *line, group_t *head) {
    int found_comma = false;
    variable_t *listitem;
    variable_t *subitem;
    char *p;

    listitem = new_variable(NULL, VT_LISTITEM);
    if (listitem == NULL)
        return NULL;

    p = line +1; /* skip past { */

    do {
        /* Skip whitespace before identifier */
        skip_whitespace(&p);
        subitem = parse_keyvalue(&p,head);
        add_to_listitem(listitem, subitem);
        skip_whitespace(&p);
        if (*p == ',') {
           p++;
           found_comma = true;
       }
       if (*p == '}') {
           p++;
           break;
        }
    } while( found_comma );

    return listitem;
}

group_t*
cfg_load(const char *fname) {
    char line[MAXLINELEN];
    group_t *head = NULL, *curr = NULL;
    FILE *fp;

    fp = fopen(fname, "r");
    if (fp == NULL)
        return NULL;

    lineno = 0;
    while(fgets(line, sizeof(line), fp)) {
        char *p = line;
	++lineno;
        /* Skip any whitespace at start of line */
        skip_whitespace(&p);
        /* Skip line if only whitespace */
        if (!*p) continue;

        switch(*p) {
            case '[':
                /* parse config group */
                {
                    group_t *newgrp = parse_group(p);
                    if (curr == NULL) {
                        head = curr = newgrp;
                    } else {
                        curr->next = newgrp;
                        curr = newgrp;
                    }
                }
                break;
            case '{':
                /* parse list item */
                {
                    variable_t *var = parse_listitem(p,head);
                    if (curr != NULL) {
                        add_var_to_group(curr, var);
                    } else {
                        fprintf(stderr, "%s:%d: Found list item but no current group!\n", __func__, lineno);
                    }
                }
                break;
            default:
                /* check for simple key/value */
                if (isalpha(*p)) {
                    variable_t *var = parse_keyvalue(&p,head);
                    if (curr != NULL) {
                        add_var_to_group(curr, var);
                    } else {
                        fprintf(stderr, "%s:%d: Found variable but no current group!\n", __func__, lineno);
                    }
                } else {
                    fprintf(stderr, "%s:%d: unknown line: %s\n", __func__, lineno, line);
                }
                break;
        }
    }

    fclose(fp);

    return head;
}

/* below is a small test framework for the parser. It can be comoiled
 * by using something like:
 *    cc -DSTANDALONE -o parsecfg parsecfg.c
 * it will parse files passed on the commandline, and dump
 * the parsed result on stdout.
 */

#ifdef STANDALONE
static void
dump_vars(variable_t *var) {
    for (; var; var=var->next) {
        switch(var->type) {
            case VT_NUMBER:
                printf("%s=0x%lx\n", var->name, var->number);
                break;
            case VT_STRING:
                printf("%s=\"%s\"\n", var->name, var->str);
                break;
            case VT_LISTITEM:
                {
                    printf("{\n");
                    dump_vars(var->items);
                    printf("},\n");
                }
                break;
            }
        }

}

static void
cfg_dump(group_t *head) {
    if (head == NULL) {
        fprintf(stderr, "%s: head == NULL!\n", __func__);
        return;
    }

    for(; head; head = head->next) {
        printf("[%s]\n", head->name);
        dump_vars(head->vars);
    }
}

int
main(int argc, char **argv) {
    group_t *head;
    int i;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <cfgfile>...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (i=1; i < argc; i++) {
        fprintf(stderr, "Parsing file %s...\n", argv[i]);
        head = cfg_load(argv[i]);
        cfg_dump(head);
    }

    exit(EXIT_SUCCESS);
}
#endif
