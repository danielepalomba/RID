#include <string.h>
#include "abuf.h"

/** @copydoc ab_append */
void ab_append(Abuf ab, const char *s, int len){
    char *new_b = realloc(ab->b, ab->len + len);
    if(!new_b)  return;

    memcpy(&new_b[ab->len], s, len);
    ab->b = new_b;
    ab->len += len;
}

/** @copydoc ab_free */
void ab_free(Abuf ab){
    free(ab->b);
}
