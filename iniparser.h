#ifndef __INIPARSER_H_
#define __INIPARSER_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INI_PARSER_FILE_ENABLE

#define INI_ERR  (-1)

typedef struct ini_item {
    char *key;
    char *value;
    struct list_head node;
} ini_item_t;

typedef struct ini_section {
    char *name;
    struct list_head node;
    struct list_head item_head;
} ini_section_t;

typedef struct ini_parser {
    struct list_head section_head;
} ini_parser_t;

/*
 * \brief Create Ini-parser
 * \return ini-parser object or Null.
 */
ini_parser_t *ini_parser_create();

/*
 * \brief Delete Ini-parser
 * \retval  0: delete ok.
 * \retval -1: obj is NULL.
 */
int ini_parser_delete(struct ini_parser *obj);

/*
 * \brief Parse INI-String.
 * \param[in] str: ini-string.
 * \return ini-parser obj if parse success, otherwise NULL.
 */
struct ini_parser *ini_parser_str_parse(const char *str);

/*
 * \brief ini-parser-obj convert to string.
 * \param[in] obj : ini-parser object.
 * \return ini string if success, otherwise NULL.
 */
char *ini_parser_str_output(struct ini_parser *obj);

#ifdef INI_PARSER_FILE_ENABLE
/*
 * \brief Parse INI-File
 * \param[in] file_name: ini-file name.
 * \return ini-parser obj if parse success, otherwise NULL.
 */
struct ini_parser *ini_parser_file_parse(const char *file_name);

/*
 * \brief ini-parser object save to ini-file.
 * \param[in] file_name: ini-file name.
 * \retval 0 : save successfully.
 * \retval -1: save failed.
 */
int ini_parser_file_save(struct ini_parser *obj, const char *file_name);
#endif

/*
 * \brief Set value by s_name and key.
 * \param[in] obj    : ini-parser object.
 * \param[in] s_name : section name.
 * \param[in] key    : key name.
 * \param[in] value  : value name.
 * \return 0 : set successfully.
 */
int ini_parser_value_set(struct ini_parser *obj, const char *s_name, const char *key, const char *name);

/*
 * \brief Get value from s_name and key.
 * \param[in] obj    : ini-parser object.
 * \param[in] s_name : section name.
 * \param[in] key    : key name.
 * \param[in][out] value  : value name..
 * \return 0 : get successfully.
 */
int ini_parser_value_get(struct ini_parser *obj, const char *s_name, const char *key, char **name);

/*
 * \brief Add Section to ini-parser object.
 * \param[in] obj    : ini-parser object.
 * \param[in] s_name : section name.
 * \return ini-section. if failed , retval is NULL.
 */
struct ini_section *ini_parser_section_add(struct ini_parser *obj, const char *s_name);

/*
 * \brief Remove Section by s_name from ini-parser object.
 * \param[in] obj    : ini-parser object.
 * \param[in] s_name : removed section name.
 * \retval  0: success.
 * \retval -1: failed.
 */
int ini_parser_section_remove(struct ini_parser *obj, const char *s_name);

/*
 * \brief Get Section by s_name from ini-parser object.
 * \param[in] obj    : ini-parser object.
 * \param[in] s_name : removed section name.
 * \return ini-section. if failed , retval is NULL.
 */
struct ini_section *ini_parser_section_get(struct ini_parser *obj, const char *s_name);

/*
 * \brief Set key-value into section.
 * \param[in] section: section object.
 * \param[in] key    : set key name.
 * \param[in] value  : set value name.
 * \retval 0 : success.
 * \retval -1: failed.
 */
int ini_parser_item_from_section_set(struct ini_section *section, const char *key, const char *value);

/*
 * \brief Get Value from section
 * \param[in] section: section object.
 * \param[in] key    : key name.
 * \param[in][out] value: get value name about key.
 * \retval 0 : success.
 * \retval -1: failed.
 */
int ini_parser_value_from_section_get(struct ini_section *section, const char *key, char **value);

#ifdef __cplusplus
}
#endif

#endif /* __INIPARSER_H_ */
