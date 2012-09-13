#ifndef _HASHMAP_H_
#define _HASHMAP_H_

/**
 * @file hashmap.h
 *
 * @brief simple hashmap implementation using linked lists
 *
 * @author Christoph Mertz
 *
 * @par Copyright
 * Christoph Mertz
 *  
 * @par Licence
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * @par Disclaimer
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 *
 * @defgroup hashmap
 *
 * @mainpage
 *
 * @par Overall
 * This is a basic hashmap implementation using linked lists
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
 
/** @ingroup hashmap */
typedef struct _hashmap_s* hashmap_t;

/**
 * @brief construct a new hashmap object
 * @ingroup hashmap
 *
 * @param bucketcount		the bucketcount - is roundet to the next power of 2
 * @param hashmap_hash_function	the hashfunction to use
 * @param value_clone_function	function to clone the values upon insertion - if a NULL pointer is passed the original values are inserted without cloning
 * @param value_free_function	function to free the values upon destruction - if a NULL pointer is passed the values will NOT be freed at destroy
 *
 * @return the newly allocated hashmap
 */
extern hashmap_t
hashmap_new (uint32_t bucketcount, 
             uint32_t (*hashmap_hash_function) (const uint8_t* value, size_t value_size), 
             void* (*value_clone_function) (const void*), 
             void (*value_free_function) (void*));

/**
 * @brief frees a given hashmap
 * @ingroup hashmap
 * 
 * @param map	the map to free
 */
extern void
hashmap_destroy (hashmap_t map);

/**
 * @brief insert a new bucket in the hashmap
 * @ingroup hashmap
 *
 * @param key		the key to insert
 * @param key_size	the size of the key in bytes
 * @param value		the value to insert
 *
 * @return 0 in case of success, a negative integer otherwise
 */
extern bool
hashmap_add (hashmap_t map, 
             const uint8_t* key, 
             size_t key_size, 
             void* value);

/**
 * @brief search a value in a hashmap
 * @ingroup hashmap
 *
 * @param map		the hashmap to search in
 * @param key		the key to search
 * @param key_size	the size of the key in bytes
 *
 * @return the found value, NULL otherwise or in case of error
 */
extern void*
hashmap_get (hashmap_t map, const uint8_t* key, size_t key_size);

/**
 * @brief removes the entry for a key from the map
 * @ingroup hashmap
 *
 * @param map		the map to manipulate
 * @param key 		the key to delete
 * @param kes_size	size of the key
 *
 * @return 0 on success, a negative integer otherwise
 */
extern bool
hashmap_remove (hashmap_t map, const uint8_t* key, size_t key_size);

/**
 * @brief applies a given function to all hashbuckets in the hashmap
 * @ingroup hashmap
 *
 * @param map		the map to manipulate
 * @param function	the function to apply
 */
extern void
hashmap_apply (hashmap_t map, void (*function)(void*));

#endif /* !_HASHMAP_H_ */
