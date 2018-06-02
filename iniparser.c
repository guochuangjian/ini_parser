#include "iniparser.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "unistd.h"
#include "string.h"

#ifndef NULL
#define NULL 0
#endif

#define INI_LINE_MAX_SIZE 256
#define INI_BUF_MAX_SIZE  1024
#define ENTER_NEW_LINE    "\n"

/*****************************************************************************/
ini_parser_t *ini_parser_create()
{
    ini_parser_t *obj = malloc(sizeof(ini_parser_t));
    if (!obj) {
        return obj;
    }

    INIT_LIST_HEAD(&obj->section_head);
    
    return obj;
}
 
int ini_parser_delete(ini_parser_t *obj) 
{
    if (!obj) {
        return 0;
    }

    if (!list_empty(&obj->section_head)) {
        ini_section_t *section, *tmp = NULL;
        list_for_each_entry_safe(section, tmp, &obj->section_head, ini_section_t, node) {
            ini_item_t *item, *item_tmp = NULL;
            list_for_each_entry_safe(item, item_tmp, &section->item_head, ini_item_t, node) {
                if (item->key) {
                    free(item->key);
                }

                if (item->value) {
                    free(item->value);
                }
                list_del(&item->node);
                free(item);
            }
            list_del(&section->node);
            free(section);
        }
    }

    free(obj);

    return 0;
}

static ini_section_t *__ini_parser_section_create (ini_parser_t *obj, const char *s_name, size_t s_name_len)
{
    ini_section_t *section = malloc(sizeof(ini_section_t));
    if (!section) {
        return NULL;
    }

    INIT_LIST_HEAD(&section->item_head);
    INIT_LIST_HEAD(&section->node);

    if (s_name) {
        section->name = malloc(s_name_len + 1);
        if (!section->name) {
            free(section);
            return NULL;
        }
        memcpy(section->name, s_name, s_name_len);
        section->name[s_name_len] = '\0';
    } else {
        section->name = NULL;
    }

    list_add_tail(&section->node, &obj->section_head);

    return section;
}

static inline ini_item_t *__item_create ()
{
    ini_item_t *item = malloc(sizeof(ini_item_t));
    if (!item) {
        return item;
    }

    INIT_LIST_HEAD(&item->node);
    item->key   = NULL;
    item->value = NULL;

    return item;
}

static inline void __item_delete (ini_item_t *item)
{
    if (item->key) {
        free(item->key);
    }

    if (item->value) {
        free(item->value);
    }

    list_del(&item->node);

    free(item);
}

static int __section_add_new_item (ini_section_t *section, const char *key, const char *value)
{
    ini_item_t *item = NULL;

    item = malloc(sizeof(ini_item_t));
    if (!item) {
        return -INI_ERR;
    }

    INIT_LIST_HEAD(&item->node);

    item->key   = strdup(key);
    item->value = strdup(value);

    list_add_tail(&item->node, &section->item_head);

    return 0;
}


int ini_parser_value_set(struct ini_parser *obj, const char *s_name, const char *key, const char *value)
{
    if (!obj) {
        return INI_ERR;
    }

    ini_section_t *section;
    list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
        if (!strcmp(section->name, s_name)) {
            ini_item_t *item; 
            list_for_each_entry(item, &section->item_head, ini_item_t, node) {
                if (!strcmp(item->key, key)) {
                    item->value = realloc(item->value, strlen(value) + 1);
//                    strcpy(item->value, value);
                    return 0;
                }
            }

            return __section_add_new_item(section, key, value);
        }
    }

    //if not found section then create it.
    section = __ini_parser_section_create(obj, s_name, strlen(s_name));
    if (!section) {
        return INI_ERR;
    }

    return __section_add_new_item(section, key, value);
}

int ini_parser_value_get(struct ini_parser *obj, const char *s_name, const char *key, char **name)
{
    if (!obj || (list_empty(&obj->section_head))) {
        return INI_ERR; 
    }

    ini_section_t *section;
    list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
        if (!strcmp(section->name, s_name)) {
            ini_item_t *item; 
            list_for_each_entry(item, &section->item_head, ini_item_t, node) {
                if (!strcmp(item->key, key)) {
                    *name = item->value;
                    return 0;
                }
            }
            return INI_ERR;
        }
    }

    return INI_ERR;
}

struct ini_section *ini_parser_section_add (struct ini_parser *obj, const char *s_name)
{
    if (!obj) {
        return NULL;
    }

    //search it
    ini_section_t *section = NULL;
    list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
        if (!strcmp(s_name, section->name)) {
            return NULL;
        }
    }

    return __ini_parser_section_create(obj, s_name, strlen(s_name));
}

int ini_parser_section_remove (struct ini_parser *obj, const char *s_name)
{
    if (!obj) {
        return INI_ERR;
    }

    if (!list_empty(&obj->section_head)) {
        ini_section_t *cur_section, *tmp = NULL;
        list_for_each_entry_safe(cur_section, tmp, &obj->section_head, ini_section_t, node) {
            if (!strcmp(cur_section->name, s_name)) {
                ini_item_t *item, *item_tmp = NULL;
                list_for_each_entry_safe(item, item_tmp, &cur_section->item_head, ini_item_t, node) {
                    if (item->key) {
                        free(item->key);
                    }
                    if (item->value) {
                        free(item->value);
                    }
                    list_del(&item->node);
                    free(item);
                }
                list_del(&cur_section->node);
                free(cur_section);
                return 0;
            }
        }
    }

    return INI_ERR;
}

struct ini_section *ini_parser_section_get (struct ini_parser *obj, const char *s_name)
{
    ini_section_t *section = NULL;

    if (obj && !list_empty(&obj->section_head)) {
        list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
            if (!strcmp(s_name, section->name)) {
                return section;
            }
        }
    }
    return NULL;
}

int ini_parser_item_from_section_set (struct ini_section *section, const char *key, const char *value)
{
    if (!section) {
        return INI_ERR;
    }

    ini_item_t *item;
    list_for_each_entry(item, &section->item_head, ini_item_t, node) {
        if (!strcmp(item->key, key)) {
            item->value = realloc(item->value, strlen(value) + 1);
            return 0;
        }
    }

    return __section_add_new_item(section, key, value);
}

int ini_parser_value_from_section_get (struct ini_section *section, const char *key, char **value)
{
    if (!section) {
        return INI_ERR;
    }

    ini_item_t *item;
    list_for_each_entry(item, &section->item_head, ini_item_t, node) {
        if (!strcmp(item->key, key)) {
            *value = item->value;
            return 0;
        }
    }

    return INI_ERR;
}


static const char *__next_line_get (const char *str)
{
    const char *line = strchr(str, '\n');

    if (line && (*(line + 1) != '\0')) {
        return line + 1;
    }

    return NULL;
}

struct ini_parser *ini_parser_str_parse (const char *str)
{
    if (!str) {
        return NULL;
    }

    ini_parser_t *obj = ini_parser_create();
    if (!obj) {
        return NULL;
    }

    const char *line = str;
    ini_section_t *cur_section = NULL;

    for (; line; line = __next_line_get(line)) {
        //skip space
        while (*line == ' ') {
            line++;
        }

        const char *key_start = line;
        while (*line != '\n') {
            if (*line == '[') {
                const char *section_end = strchr(line, ']');
                const char *next_line   = __next_line_get(line);
                if (!section_end ||
                    (next_line && (section_end > next_line))) {
                    ini_parser_delete(obj);
                    return NULL;
                }

                cur_section = __ini_parser_section_create(obj, line + 1, (section_end - line - 1));
                if (!cur_section) {
                    ini_parser_delete(obj);
                    return NULL;
                }

                line = section_end + 1;
            } else if (*line == '=') {
                //if no section then create section but no name
                if (!cur_section) {
                    cur_section = __ini_parser_section_create(obj, NULL, 0);
                    if (!cur_section) {
                        ini_parser_delete(obj);
                        return NULL;
                    }
                }

                ini_item_t *item = __item_create();
                if (!item) {
                    ini_parser_delete(obj);
                    return NULL;
                }

                //save key name
                uint32_t key_len = line - key_start;
                item->key = malloc(key_len + 1);
                if (!item->key) {
                    __item_delete(item);
                    ini_parser_delete(obj);
                    return NULL;
                }

                memcpy(item->key, key_start, key_len);
                item->key[key_len] = '\0';

                //get value end
                const char *value_start = line + 1;
                while (*line != '\r' && *line != '\n' && *line != '\0') {
                    line++;
                }

                uint32_t value_len = line - value_start;
                item->value = malloc(value_len + 1);
                if (!item->value) {
                    __item_delete(item);
                    ini_parser_delete(obj);
                    return NULL;
                }

                memcpy(item->value, value_start, value_len);
                item->value[value_len] = '\0';

                //add item to current section
                list_add_tail(&item->node, &cur_section->item_head);

            } else {
                line++;
            }
        }
    }

    return obj;
}

char *ini_parser_str_output (struct ini_parser *obj)
{
    if (!obj || list_empty(&obj->section_head)) {
        return NULL;
    }

    char *ini_str = malloc(INI_BUF_MAX_SIZE);
    if (!ini_str) {
        return NULL;
    }

    ini_section_t *section = NULL;
    int    ret     = 0;
    size_t offset  = 0;
    size_t max_len = INI_BUF_MAX_SIZE;
    list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
        if (section->name) {
            ret = strlen(section->name) + 3;
            if (ret > (max_len - offset)) {
                max_len += (ret << 1) > INI_BUF_MAX_SIZE ? (ret << 1) : INI_BUF_MAX_SIZE;
                ini_str = realloc(ini_str, max_len); //fixme realloc can use old ptr ?
            }

            ret = sprintf(ini_str + offset, "[%s]"ENTER_NEW_LINE, section->name);
            offset += ret;
        }

        ini_item_t *item = NULL;
        list_for_each_entry(item, &section->item_head, ini_item_t, node) {
            if ((strlen(item->key) + strlen(item->value) + 2) > (max_len  - offset)) {
                //realloc memory
                max_len += (ret << 1) > INI_BUF_MAX_SIZE ? (ret << 1) : INI_BUF_MAX_SIZE;
                ini_str = realloc(ini_str, max_len);
            }

            ret = sprintf(ini_str + offset, "%s=%s"ENTER_NEW_LINE, item->key, item->value);
            offset += ret;
        }
    }

    return ini_str;
}

static const char *__file_get_line (FILE *fd, char *s, int len)
{
    char ch;
    char *line = s;
    while ((--len) && (fread(&ch, 1, 1, fd) == 1) && (ch != '\0')) {
        if ((*line++ = ch) == '\n') {
            break;
        }
    }

    *line = '\0';
    return s;
}

struct ini_parser *ini_parser_file_parse(const char *file_name)
{
    char line_str[INI_LINE_MAX_SIZE];
    FILE *fd = fopen(file_name, "r");
    if (!fd) {
        return NULL;
    }

    ini_parser_t *obj = ini_parser_create();
    if (!obj) {
        fclose(fd);
        return NULL;
    }

    ini_section_t *cur_section = NULL;
    const char *line = __file_get_line(fd, line_str, INI_LINE_MAX_SIZE);
    for (; *line != '\0' && line;
             line = __file_get_line(fd, line_str, INI_LINE_MAX_SIZE)) {
        //skip space
        while (*line == ' ') {
            line++;
        }

        const char *key_start = line;
        while (*line != '\n') {
            if (*line == '[') {
                const char *section_end = strchr(line, ']');
                if (!section_end) {
                    goto __exit;
                }

                cur_section = __ini_parser_section_create(obj, line + 1, (section_end - line - 1));
                if (!cur_section) {
                    goto __exit;
                }

                line = section_end + 1;
            } else if (*line == '=') {
                //if no section then create section but no name
                if (!cur_section) {
                    cur_section = __ini_parser_section_create(obj, NULL, 0);
                    if (!cur_section) {
                        goto __exit;
                    }
                }

                ini_item_t *item = __item_create();
                if (!item) {
                    goto __exit;
                }

                //save key name
                uint32_t key_len = line - key_start;
                item->key = malloc(key_len + 1);
                if (!item->key) {
                    __item_delete(item);
                    goto __exit;
                }

                memcpy(item->key, key_start, key_len);
                item->key[key_len] = '\0';

                //get value end
                const char *value_start = line + 1;
                while (*line != '\r' && *line != '\n' && *line != '\0') {
                    line++;
                }

                uint32_t value_len = line - value_start;
                item->value = malloc(value_len + 1);
                if (!item->value) {
                    __item_delete(item);
                    goto __exit;
                }

                memcpy(item->value, value_start, value_len);
                item->value[value_len] = '\0';

                //add item to current section
                list_add_tail(&item->node, &cur_section->item_head);

            } else {
                line++;
            }
        }
    }

    fclose(fd);
    return obj;
__exit:
    fclose(fd);
    ini_parser_delete(obj);
    return NULL;
}

int ini_parser_file_save(struct ini_parser *obj, const char *file_name)
{
    if (!obj || list_empty(&obj->section_head)) {
        return INI_ERR;
    }

    FILE* fd = fopen(file_name, "w+");
    if (fd < 0) {
        return INI_ERR;
    }

    char line[INI_LINE_MAX_SIZE];
    ini_section_t *section = NULL;
    int ret = 0;
    list_for_each_entry(section, &obj->section_head, ini_section_t, node) {
        if (section->name) {
            ret = snprintf(line, INI_LINE_MAX_SIZE, "[%s]"ENTER_NEW_LINE, section->name);
            if (fwrite(line, 1, ret, fd) != ret) {
                goto __exit;
            }
        }

        ini_item_t *item = NULL;
        list_for_each_entry(item, &section->item_head, ini_item_t, node) {
            ret = snprintf(line, INI_LINE_MAX_SIZE, "%s=%s"ENTER_NEW_LINE, item->key, item->value);
            if (fwrite(line, 1, ret, fd) != ret) {
                goto __exit;
            }
        }
    }

    ret = 0;

__exit:
    fclose(fd);

    return ret ? INI_ERR : 0;
}

