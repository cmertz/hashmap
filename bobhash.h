/**
 * @file bobhash.h
 *
 * @brief hash algorithm for fast lookups
 *
 * @author Bob Jenkins
 *
 * See http://burtleburtle.net/bob/hash/evahash.html
 * 
 * @par Licence
 * public domain
 */

#ifndef _BOB_HASH_H_
#define _BOB_HASH_H_

#include <stdint.h>
#include <stdlib.h>

/** 
 * @brief hash a variable-length key into a 32-bit value
 *
 * @param k		the key to hash
 * @param length	the length of the key in bytes
 *
 * @return a 32-bit hash value
 */
uint32_t
bobhash (const void *key, size_t key_size);

#endif /* _BOB_HASH_H_ */
