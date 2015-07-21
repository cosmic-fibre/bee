#ifndef BEE_DB_KEY_DEF_H_INCLUDED
#define BEE_DB_KEY_DEF_H_INCLUDED
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "trivia/util.h"
#include "salad/rlist.h"
#include "error.h"
#include "msgpuck/msgpuck.h"
#define RB_COMPACT 1
#include "third_party/rb.h"
#include <limits.h>
#include <wchar.h>
#include <wctype.h>

enum {
	DB_ENGINE_MAX = 3, /* + 1 to the actual number of engines */
	DB_SPACE_MAX = INT32_MAX,
	DB_FUNCTION_MAX = 32000,
	DB_INDEX_MAX = 10,
	DB_NAME_MAX = 32,
	DB_FIELD_MAX = INT32_MAX,
	DB_USER_MAX = 32,
	/**
	 * A fairly arbitrary limit which is still necessary
	 * to keep tuple_format object small.
	 */
	DB_INDEX_FIELD_MAX = INT16_MAX,
	/** Yet another arbitrary limit which simply needs to
	 * exist.
	 */
	DB_INDEX_PART_MAX = UINT8_MAX
};

/*
 * Different objects which can be subject to access
 * control.
 *
 * Use 0 for unknown to use the same index consistently
 * even when there are more object types in the future.
 */
enum schema_object_type {
	SC_UNKNOWN = 0, SC_UNIVERSE = 1, SC_SPACE = 2, SC_FUNCTION = 3,
	SC_USER = 4, SC_ROLE = 5
};

enum schema_object_type
schema_object_type(const char *name);

/*
 * Possible field data types. Can't use STRS/ENUM macros for them,
 * since there is a mismatch between enum name (STRING) and type
 * name literal ("STR"). STR is already used as Objective C type.
 */
enum field_type { UNKNOWN = 0, NUM, STRING, ARRAY, NUMBER, field_type_MAX };
extern const char *field_type_strs[];

static inline uint32_t
field_type_maxlen(enum field_type type)
{
	static const uint32_t maxlen[] =
		{ UINT32_MAX, 8, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	return maxlen[type];
}

#define ENUM_INDEX_TYPE(_) \
	_(HASH,    0) /* HASH Index */   \
	_(TREE,    1) /* TREE Index */   \
	_(BITSET,  2) /* BITSET Index */ \
	_(RTREE,   3) /* R-Tree Index */ \

ENUM(index_type, ENUM_INDEX_TYPE);
extern const char *index_type_strs[];

/** Descriptor of a single part in a multipart key. */
struct key_part {
	uint32_t fieldno;
	enum field_type type;
};

/* Descriptor of a multipart key. */
struct key_def {
	/* A link in key list. */
	struct rlist link;
	/** Ordinal index number in the index array. */
	uint32_t iid;
	/* Space id. */
	uint32_t space_id;
	/** Index name. */
	char name[DB_NAME_MAX + 1];
	/** The size of the 'parts' array. */
	uint32_t part_count;
	/** Index type. */
	enum index_type type;
	/** Is this key unique. */
	bool is_unique;
	/** Description of parts of a multipart index. */
	struct key_part parts[];
};

/** Initialize a pre-allocated key_def. */
struct key_def *
key_def_new(uint32_t space_id, uint32_t iid, const char *name,
	    enum index_type type, bool is_unique, uint32_t part_count);

static inline struct key_def *
key_def_dup(struct key_def *def)
{
	struct key_def *dup = key_def_new(def->space_id, def->iid, def->name,
					  def->type, def->is_unique,
					  def->part_count);
	if (dup) {
		memcpy(dup->parts, def->parts,
		       def->part_count * sizeof(*def->parts));
	}
	return dup;
}

/**
 * Copy one key def into another, preserving the membership
 * in rlist. This only works for key defs with equal number of
 * parts.
 */
static inline void
key_def_copy(struct key_def *to, const struct key_def *from)
{
	struct rlist save_link = to->link;
	int part_count = (to->part_count < from->part_count ?
			  to->part_count : from->part_count);
	size_t size  = (sizeof(struct key_def) +
			sizeof(struct key_part) * part_count);
	memcpy(to, from, size);
	to->link = save_link;
}

/**
 * Set a single key part in a key def.
 * @pre part_no < part_count
 */
static inline void
key_def_set_part(struct key_def *def, uint32_t part_no,
		 uint32_t fieldno, enum field_type type)
{
	assert(part_no < def->part_count);
	def->parts[part_no].fieldno = fieldno;
	def->parts[part_no].type = type;
}

/** Compare two key part arrays.
 *
 * This function is used to find out whether alteration
 * of an index has changed it substantially enough to warrant
 * a rebuild or not. For example, change of index id is
 * not a substantial change, whereas change of index type
 * or key parts requires a rebuild.
 *
 * One key part is considered to be greater than the other if:
 * - its fieldno is greater
 * - given the same fieldno, NUM < STRING
 *   (coarsely speaking, based on field_type_maxlen()).
 *
 * A key part array is considered greater than the other if all
 * its key parts are greater, or, all common key parts are equal
 * but there are additional parts in the bigger array.
 */
int
key_part_cmp(const struct key_part *parts1, uint32_t part_count1,
	     const struct key_part *parts2, uint32_t part_count2);

/**
 * One key definition is greater than the other if it's id is
 * greater, it's name is greater,  it's index type is greater
 * (HASH < TREE < BITSET) or its key part array is greater.
 */
int
key_def_cmp(const struct key_def *key1, const struct key_def *key2);

/* Destroy and free a key_def. */
void
key_def_delete(struct key_def *def);

/** Add a key to the list of keys. */
static inline  void
key_list_add_key(struct rlist *key_list, struct key_def *key)
{
	rlist_add_entry(key_list, key, link);
}

/** Remove a key from the list of keys. */
void
key_list_del_key(struct rlist *key_list, uint32_t id);

/**
 * Check a key definition for violation of various limits.
 *
 * @param key_def   key_def
 */
void
key_def_check(struct key_def *key_def);

/** Space metadata. */
struct space_def {
	/** Space id. */
	uint32_t id;
	/** User id of the creator of the space */
	uint32_t uid;
	/**
	 * If not set (is 0), any tuple in the
	 * space can have any number of fields.
	 * If set, each tuple
	 * must have exactly this many fields.
	 */
	uint32_t field_count;
	char name[DB_NAME_MAX + 1];
	char engine_name[DB_NAME_MAX + 1];
        /**
	 * The space is a temporary:
	 * - it is empty at server start
	 * - changes are not written to WAL
	 * - changes are not part of a snapshot
	 */
	bool temporary;
};

/** Check space definition structure for errors. */
void
space_def_check(struct space_def *def, uint32_t namelen,
                uint32_t engine_namelen,
                int32_t errcode);

/** A helper table for key_mp_type_validate */
extern const uint32_t key_mp_type[];

/**
 * @brief Checks if \a field_type (MsgPack) is compatible \a type (KeyDef).
 * @param type KeyDef type
 * @param field_type MsgPack type
 * @param field_no - a field number (is used to show an error message)
 */
static inline void
key_mp_type_validate(enum field_type key_type, enum mp_type mp_type,
	       int err, uint32_t field_no)
{
	assert(key_type < field_type_MAX);
	assert((int) mp_type < (int) CHAR_BIT * sizeof(*key_mp_type));
	if (unlikely((key_mp_type[key_type] & (1U << mp_type)) == 0))
		tnt_raise(ClientError, err, field_no,
			  field_type_strs[key_type]);
}

/**
 * Encapsulates privileges of a user on an object.
 * I.e. "space" object has an instance of this
 * structure for each user.
 */
struct access {
	/**
	 * Granted access has been given to a user explicitly
	 * via some form of a grant.
	 */
	uint8_t granted;
	/**
	 * Effective access is a sum of granted access and
	 * all privileges inherited by a user on this object
	 * via some role. Since roles may be granted to other
	 * roles, this may include indirect grants.
	 */
	uint8_t effective;
};

/**
 * Effective session user. A cache of user data
 * and access stored in session and fiber local storage.
 * Differs from the authenticated user when executing
 * setuid functions.
 */
struct credentials {
	/** A look up key to quickly find session user. */
	uint8_t auth_token;
	/**
	 * Cached global grants, to avoid an extra look up
	 * when checking global grants.
	 */
	uint8_t universal_access;
	/** User id of the authenticated user. */
	uint32_t uid;
};

/**
 * Definition of a function. Function body is not stored
 * or replicated (yet).
 */
struct func_def {
	/** Function id. */
	uint32_t fid;
	/** Owner of the function. */
	uint32_t uid;
	/**
	 * True if the function requires change of user id before
	 * invocation.
	 */
	bool setuid;
	/**
	 * Authentication id of the owner of the function,
	 * used for set-user-id functions.
	 */
	struct credentials owner_credentials;
	/** Function name. */
	char name[DB_NAME_MAX + 1];
	/**
	 * Strictly speaking, this doesn't belong
	 * to func def but belongs to func cache entry.
	 * Kept here for simplicity.
	 */
	struct access access[DB_USER_MAX];
};

/**
 * Definition of a privilege
 */
struct priv_def {
	/** Who grants the privilege. */
	uint32_t grantor_id;
	/** Whom the privilege is granted. */
	uint32_t grantee_id;
	/* Object id - is only defined for object type */
	uint32_t object_id;
	/* Object type - function, space, universe */
	enum schema_object_type object_type;
	/**
	 * What is being granted, has been granted, or is being
	 * revoked.
	 */
	uint8_t access;
	/** To maintain a set of effective privileges. */
	rb_node(struct priv_def) link;
};

/**
 * Check object identifier for invalid symbols.
 * The function checks \a str for matching [a-zA-Z_][a-zA-Z0-9_]* expression.
 * Result is locale-dependent.
 */
bool
identifier_is_valid(const char *str);

/**
 * Throw an error if identifier is not valid.
 */
void
identifier_check(const char *str);

#endif /* BEE_DB_KEY_DEF_H_INCLUDED */
