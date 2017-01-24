//
// Created by a on 1/19/17.
//

#include <ctype.h>
#include "string_helper.h"

char *str_to_lower(char *s) {
  for (char *ss = s; *ss; ss++) *ss = tolower(*ss);
  return s;
}