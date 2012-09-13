/**
 * @file hashmap.c
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
 */

#include "hashmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/** @brief structure to represent a hashbucket */
struct _hashbucket_s
{
	/** the key */
	uint8_t* key;

	/** the size of the key */
	uint8_t key_size;

	/** the actual payload */
	void* value;

	/** next element in bucket list */
	struct _hashbucket_s* next;
};

/**
 * @brief create a new hasbucket
 *
 * @param key			the key of the newly created hashbucket
 * @param key_size		size of the key in bytes
 * @param value			the value to insert
 * @param value_clone_function	the function used to clone the value - if NULL is passed a shallow copy is done
 *
 * @warning since this is a private function, parameter checks are omitted
 * @return the newly created hashbucket on success, NULL otherwise
 */
static struct _hashbucket_s*
_hashbucket_new (const uint8_t* key,
                 uint8_t key_size,
                 void* value,
                 void* (*value_clone_function) (const void*))
{
	struct _hashbucket_s* newbucket = NULL;

	assert(key != NULL);

	newbucket = (struct _hashbucket_s*) malloc (sizeof (struct _hashbucket_s));
	if (newbucket == NULL)
		return NULL;

	newbucket->key = (uint8_t*) malloc (key_size);
	if (newbucket->key == NULL)
	{
		free (newbucket);
		return NULL;
	}
	memcpy (newbucket->key, key, key_size);
	newbucket->key_size = key_size;

	if (value_clone_function != NULL)
		newbucket->value = (*(value_clone_function)) (value);
	else
		newbucket->value = value;
	
	return newbucket;
}

/**
 * @brief free a hashbucket
 *
 * @param bucket		the bucket to free
 * @param value_free_function	the function used to free the buckets value - if NULL is passed, only the hashmap structures (buckets, keycopy) will be freed
 *
 * @warning since this is a private function, parameter checks are omitted
 */
static void
_hashbucket_free (struct _hashbucket_s* bucket,
                  void (*value_free_function) (void*))
{
	assert(bucket != NULL);

	if (bucket->value != NULL && value_free_function != NULL)
		value_free_function (bucket->value);

	free (bucket->key);
	free (bucket);
}

/**
 * @brief apply function to all following hashbucket values
 * 
 * @param bucket		the bucket to begin with
 * @param value_free_function	the function to apply
 *
 * @warning since this is a private function, parameter checks are omitted
 */
static void
_hashbucket_apply (struct _hashbucket_s* bucket,
                   void (*apply_function) (void*))
{
	struct _hashbucket_s* next;
	struct _hashbucket_s* current;

	assert(apply_function != NULL);

	next = bucket;
	while (next != NULL)
	{
		current = next;
		next = next->next;
		(*apply_function) (current->value);
	}
}

/**
 * @brief apply function to all following hashbuckets
 * 
 * @param bucket		the bucket to begin with
 * @param value_free_function	the function to apply
 *
 * @warning since this is a private function, parameter checks are omitted
 */
static void
_hashbucket_free_all (struct _hashbucket_s* bucket,
                      void (*value_free_function) (void*))
{
	struct _hashbucket_s* next;
	struct _hashbucket_s* current;

	next = bucket;
	while (next != NULL)
	{
		current = next;
		next = next->next;
		_hashbucket_free (current, value_free_function);
	}
}

/** @brief structure representing a hash map */
struct _hashmap_s
{
	/** number of buckets in the hashmap */
	uint32_t bucketcount;

	/** the actual buckets */
	struct _hashbucket_s** buckets;

	/** mask that will be bitwise conjucted with the result in order to reduce it to a given size */
	uint32_t hashmask;

	/** hash function used in this hashmap */
	uint32_t (*hashmap_hash_function) (const uint8_t* value, size_t value_size);

	/** function to free the bucket values */
	void (*value_free_function) (void*);

	/** function for cloning of values */
	void* (*value_clone_function) (const void*);
};

typedef struct _hashmap_s hashmap_s;

hashmap_t
hashmap_new (uint32_t bucketcount, 
             uint32_t (*hashmap_hash_function) (const uint8_t* value, size_t value_size), 
             void* (*value_clone_function) (const void*), 
             void (*value_free_function) (void*))
{
	hashmap_t newmap;
	uint8_t shifts;
	uint32_t count;

	if (bucketcount < 1 || hashmap_hash_function == NULL)
		return NULL;

	newmap = (hashmap_t) malloc (sizeof(hashmap_s));
	if (newmap == NULL)
		return NULL;

	/* get the highest bit - position is stored in shifts */
	count = bucketcount;
	for (shifts = 0; count > 1; count >>= 1)
		shifts++;

	/* round to the nearest power of 2 */
	if ((1 << (shifts + 1)) - bucketcount < bucketcount - (1 << shifts))
		shifts++;

	newmap->bucketcount = (uint32_t)1 << shifts;

	newmap->buckets = (struct _hashbucket_s**) calloc (newmap->bucketcount, sizeof(struct _hashbucket_s*));
	if (newmap->buckets == NULL)
	{
		free (newmap);
		return NULL;
	}

	newmap->hashmask = newmap->bucketcount - 1;

	newmap->hashmap_hash_function = hashmap_hash_function;

	newmap->value_clone_function = value_clone_function;
	
	newmap->value_free_function = value_free_function;

	return newmap;
}

void
hashmap_apply (hashmap_t map, 
               void (*apply_function) (void*))
{
	uint32_t i;

	if (map == NULL || apply_function == NULL)
		return;

	for (i = 0; i < map->bucketcount; i++)
		_hashbucket_apply (map->buckets[i], apply_function);
}

void
hashmap_destroy (hashmap_t map)
{
	uint32_t i;

	if (map == NULL)
		return;

	for (i = 0; i < map->bucketcount; i++)
		_hashbucket_free_all (map->buckets[i], map->value_free_function);

	free (map->buckets);
	free (map);
}

/**
 * @brief insert a hashbucket at a given position
 *
 * @param map		the map where the new element is inserted
 * @param newbucket	the new bucket to insert
 * @param index		the index at which the new bucket is inserted
 *
 * @warning since this is a private function, parameter checks are omitted
 *
 * @return 0 on success, a negative integer otherwise
 */
static bool
_hashmap_hashbucket_insert_at (hashmap_t map, 
                               struct _hashbucket_s* newbucket,
                               uint32_t index)
{
	struct _hashbucket_s* iterator;
	struct _hashbucket_s* previous;

	if (map->buckets[index] == NULL)
	{
		newbucket->next = NULL;
		map->buckets[index] = newbucket;
	}
	else
	{
		iterator = map->buckets[index];
		while (iterator != NULL && memcmp (iterator->key, newbucket->key, newbucket->key_size) != 0)
		{
			previous = iterator;
			iterator = iterator->next;
		}

		if (iterator != NULL)
			return false;

		previous->next = newbucket;
		newbucket->next = NULL;
	}

	return true;
}

bool
hashmap_add (hashmap_t map, 
             const uint8_t* key,
             size_t key_size,
             void* value)
{
	struct _hashbucket_s* newbucket = NULL;

	if (map == NULL || key == NULL || key_size == 0)
		return false;

	newbucket = _hashbucket_new (key, key_size, value, map->value_clone_function);
	if (newbucket == NULL)
		return false;

	if (_hashmap_hashbucket_insert_at (map, newbucket, (*(map->hashmap_hash_function)) (key, key_size) & map->hashmask) != true)
	{
		_hashbucket_free (newbucket, map->value_free_function);
		return false;
	}

	return true;
}

void*
hashmap_get (hashmap_t map,
              const uint8_t* key,
              size_t key_size)
{
	uint32_t hash;
	struct _hashbucket_s* iterator;

	if (map == NULL || key == NULL || key_size == 0)
		return NULL;

	hash = (*(map->hashmap_hash_function)) (key, key_size);
	hash &= map->hashmask;

	if (hash >= map->bucketcount || map->buckets[hash] == NULL)
		return NULL;

	iterator = map->buckets[hash];
	while (iterator != NULL && memcmp (iterator->key, key, key_size) != 0)
		iterator = iterator->next;
	
	if (iterator == NULL)
		return NULL;

	return iterator->value;
}

bool
hashmap_remove (hashmap_t map,
                const uint8_t* key,
                size_t key_size)
{
	uint32_t hash;
	struct _hashbucket_s* iterator;
	struct _hashbucket_s* previous;

	if (map == NULL || key == NULL || key_size == 0)
		return false;

	hash = (*(map->hashmap_hash_function)) (key, key_size);
	hash &= map->hashmask;

	if (hash >= map->bucketcount || map->buckets[hash] == NULL)
		return false;

	iterator = map->buckets[hash];
	previous = NULL;

	while (iterator != NULL && memcmp (iterator->key, key, key_size) != 0)
	{
		previous = iterator;
		iterator = iterator->next;
	}

	if (iterator == NULL)
		return false;

	if (previous != NULL)
		previous->next = iterator->next;
	else
		map->buckets[hash] = iterator->next;

	_hashbucket_free (iterator, map->value_free_function);

	return true;
}
