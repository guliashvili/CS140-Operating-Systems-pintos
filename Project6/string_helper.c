//
// Created by a on 1/19/17.
//

#include <ctype.h>
#include "string_helper.h"
/*
 * transforms c string to lower case c string
 */
char *str_to_lower(char *s) {
  for (char *ss = s; *ss; ss++) *ss = tolower(*ss);
  return s;
}
