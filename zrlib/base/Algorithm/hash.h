/**
 * @author zuri
 * @date jeu. 30 juil. 2020 15:04:45 CEST
 */

#ifndef ZRHASH_H
#define ZRHASH_H

#include <zrlib/config.h>

#include <stdlib.h>

size_t zrhash_jenkins_one_at_a_time(char *key, size_t len);

#endif
