/** BEGIN COPYRIGHT BLOCK
 * This Program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; version 2 of the License.
 * 
 * This Program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this Program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * In addition, as a special exception, Red Hat, Inc. gives You the additional
 * right to link the code of this Program with code not covered under the GNU
 * General Public License ("Non-GPL Code") and to distribute linked combinations
 * including the two, subject to the limitations in this paragraph. Non-GPL Code
 * permitted under this exception must only link to the code of this Program
 * through those well defined interfaces identified in the file named EXCEPTION
 * found in the source code files (the "Approved Interfaces"). The files of
 * Non-GPL Code may instantiate templates or use macros or inline functions from
 * the Approved Interfaces without causing the resulting work to be covered by
 * the GNU General Public License. Only Red Hat, Inc. may make changes or
 * additions to the list of Approved Interfaces. You must obey the GNU General
 * Public License in all respects for all of the Program code and other code used
 * in conjunction with the Program except the Non-GPL Code covered by this
 * exception. If you modify this file, you may extend this exception to your
 * version of the file, but you are not obligated to do so. If you do not wish to
 * provide this exception without modification, you must delete this exception
 * statement from your version and license this file solely under the GPL without
 * exception. 
 * 
 * 
 * Copyright (C) 2001 Sun Microsystems, Inc. Used by permission.
 * Copyright (C) 2005 Red Hat, Inc.
 * All rights reserved.
 * END COPYRIGHT BLOCK **/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* back-ldbm.h - ldap ldbm back-end header file */

#ifndef _BACK_LDBM_H_
#define _BACK_LDBM_H_

#define	SLAPD_LOGGING	1

#if defined(irix) || defined(AIX) || defined(HPUX11) || defined(OS_solaris) || defined(linux)
/* built-in 64-bit file I/O support */
#define DB_USE_64LFS
#endif

/* needed by at least HPUX and Solaris, to define off64_t */
#ifdef DB_USE_64LFS
#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE
#endif
#endif

/* A bunch of random system headers taken from all the source files, no source file should #include
   any system headers now */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "prio.h"  /* for PR_OpenDir etc */
#include "prlog.h" /* for PR_ASSERT */
/* The following cruft is for ldif2db only */
#ifndef XP_WIN32
#include <unistd.h>	/* write/close (ldbm2ldif_write) */
#else
#include <io.h>	/* write/close (ldbm2ldif_write) */
#endif
#include <fcntl.h>
#include <time.h>
/* And this cruft is from nextid.c */
#ifndef _WIN32
#include <sys/param.h>
#endif /* ! _WIN32 */
#include <limits.h> /* Used in search.c (why?) */



#ifndef _WIN32
/* for MAXPATHLEN */
#include <sys/param.h>
#define MKDIR(path,mode) mkdir((path),(mode))
#else
/* for mkdir */
#include <direct.h>
#define MKDIR(path,mode) mkdir(path)
#endif

#ifdef HPUX11
#define	__BIT_TYPES_DEFINED__
typedef unsigned char u_int8_t;
typedef unsigned int u_int32_t;
typedef unsigned short u_int16_t;
#endif
#include "db.h"

#define dptr data
#define dsize size

#define ID2ENTRY "id2entry"	/* main db file name: ID2ENTRY+LDBM_SUFFIX */

#define LDBM_SUFFIX_OLD	".db3"
#define LDBM_SUFFIX	".db4"

#define MEGABYTE (1024 * 1024)
#define GIGABYTE (1024 * MEGABYTE)


/* include NSPR header files */
#include "nspr.h"
#include "plhash.h"

#include "slap.h"
#include "slapi-plugin.h"
#include "slapi-private.h"
#include "avl.h"
#include "ldaplog.h"
#include "portable.h"
#include "proto-slap.h"

/* We should only change the LDBM_VERSION when the format of the db files
 * is changing in some (possibly incompatible) way -- so we can detect and
 * treat older ldbm versions.  Thus, f.e., DS4.1 will still use the same
 * LDBM_VERSION as 4.0 and so on...
 * Don't make the length of LDBM_VERSION longer than LDBM_VERSION_MAXBUF - 1
 */
#define LDBM_VERSION_MAXBUF		64
#define LDBM_DATABASE_TYPE_NAME	"ldbm database"
/*
 * 232050: Change format of DBVERSION and guardian files
 * new format:
 * implementation/version/server backend plugin name[/other tag][/other tag]....
 * For example:
 * bdb/4.2/libback-ldbm/newidl
 * This indicates that the files use Berkeley DB version 4.2, they are used 
 * by the server libback-ldbm database plugin, and the index files use the 
 * newidl format.
 * Starting from DS7.2
 */
#define BDB_IMPL        "bdb"
#define BDB_BACKEND     "libback-ldbm" /* This backend plugin */

/*
 * While we support both new and old idl index,
 * we distinguish them by the following 2 macros.
 * When we drop the old idl code, we eliminate LDBM_VERSION_OLD.
 * bug #604922
 */
#define LDBM_VERSION_BASE       "Netscape-ldbm/"
#define LDBM_VERSION            "Netscape-ldbm/7.0" /* db42: new idl -> old */
#define LDBM_VERSION_NEW        "Netscape-ldbm/7.0_NEW"     /* db42: new idl */
#define LDBM_VERSION_OLD        "Netscape-ldbm/7.0_CLASSIC" /* db42: old idl */
#define LDBM_VERSION_62         "Netscape-ldbm/6.2" /* db42: old idl */
#define LDBM_VERSION_61         "Netscape-ldbm/6.1" /* db33: old idl */
#define LDBM_VERSION_60         "Netscape-ldbm/6.0" /* db33: old idl */

#define LDBM_VERSION_50			"Netscape-ldbm/5.0"
#define LDBM_VERSION_40			"Netscape-ldbm/4.0"
#define LDBM_VERSION_30			"Netscape-ldbm/3.0"
#define LDBM_VERSION_31			"Netscape-ldbm/3.1"
#define LDBM_FILENAME_SUFFIX		".db4"
#define	DBVERSION_FILENAME		"DBVERSION"
#define DEFAULT_CACHE_SIZE		(size_t)10485760
#define DEFAULT_CACHE_ENTRIES		-1		/* no limit */
#define DEFAULT_DBCACHE_SIZE		1000000
#define DEFAULT_MODE			0600
#define DEFAULT_ALLIDSTHRESHOLD		4000
#define DEFAULT_LOOKTHROUGHLIMIT	5000
#define DEFAULT_IDL_TUNE		1
#define DEFAULT_SEARCH_TUNE		0
#define DEFAULT_IMPORT_INDEX_BUFFER_SIZE  0
#define SUBLEN			3
#define LDBM_CACHE_RETRY_COUNT 1000 /* Number of times we re-try a cache operation */
#define IDL_FETCH_RETRY_COUNT 5 /* Number of times we re-try idl_fetch if it returns deadlock */
#define IMPORT_SUBCOUNT_HASHTABLE_SIZE 500 /* Number of buckets in hash used to accumulate subcount for broody parents */

/* minimum max ids that a single index entry can map to in ldbm */
#define SLAPD_LDBM_MIN_MAXIDS		4000

/* clear the following flag to suppress "database files do not exist" warning */
extern int ldbm_warn_if_no_db;

/*
 * there is a single index for each attribute.  these prefixes insure
 * that there is no collision among keys.
 */
#define EQ_PREFIX	'='	/* prefix for equality keys     */
#define APPROX_PREFIX	'~'	/* prefix for approx keys       */
#define SUB_PREFIX	'*'	/* prefix for substring keys    */
#define CONT_PREFIX	'\\'	/* prefix for continuation keys */
#define RULE_PREFIX	':'	/* prefix for matchingRule keys */
#define PRES_PREFIX	'+'

/* Values for "disposition" value in idl_insert_key() */
#define IDL_INSERT_NORMAL 1
#define IDL_INSERT_ALLIDS 2
#define IDL_INSERT_NOW_ALLIDS 3

#define DEFAULT_BLOCKSIZE	8192

/*
 * The candidate list size at which it is cheaper to apply the filter test
 * to the whole list than to continue ANDing in IDLs.
 */
#define FILTER_TEST_THRESHOLD (NIDS)10

/* flags to indicate what kind of startup the dblayer should do */
#define DBLAYER_IMPORT_MODE                 0x1
#define DBLAYER_NORMAL_MODE                 0x2
#define DBLAYER_EXPORT_MODE                 0x4
#define DBLAYER_ARCHIVE_MODE                0x8
#define DBLAYER_RESTORE_MODE                0x10
#define DBLAYER_RESTORE_NO_RECOVERY_MODE    0x20
#define DBLAYER_TEST_MODE                   0x40
#define DBLAYER_INDEX_MODE                  0x80
#define DBLAYER_CLEAN_RECOVER_MODE          0x100

#define DBLAYER_NO_DBTHREADS_MODE           0x1000

#define DBLAYER_RESTORE_MASK                (DBLAYER_RESTORE_MODE|DBLAYER_RESTORE_NO_RECOVERY_MODE)


/*
 * the id used in the indexes to refer to an entry
 */
typedef u_int32_t	ID;
#define MAXID   ((ID)-3)
#define NOID	((ID)-2)
#define ALLID	((ID)-1)

/*
 * effective only on idl_new_fetch
 */
#define NEW_IDL_NOOP     1	  /* no need to fetch on new idl */
#define NEW_IDL_NO_ALLID 2	  /* force to return full idl (no allids) */
#define NEW_IDL_DEFAULT  0

/*
 * if the id of any backend instance is above the threshold, then warning
 * message will be logged about the need of rebuilding the database in question
 */
#define ID_WARNING_THRESHOLD (MAXID * 0.9)

/*
 * Use this to count and index into an array of ID.
 */
typedef u_int32_t	NIDS;

/*
 * This structure represents an id block on disk and an id list
 * in core.
 *
 * The fields have the following meanings:
 *
 *	b_nmax	maximum number of ids in this block. if this is == ALLIDSBLOCK,
 *		then this block represents all ids.
 *	b_nids	current number of ids in use in this block.  if this
 *		is == INDBLOCK, then this block is an indirect block
 *		containing a list of other blocks containing actual ids.
 *		the list is terminated by an id of NOID.
 *	b_ids	a list of the actual ids themselves
 */
typedef struct block {
	NIDS		b_nmax;		/* max number of ids in this list  */
#define ALLIDSBLOCK	0		/* == 0 => this is an allid block  */
	NIDS		b_nids;		/* current number of ids used	   */
#define INDBLOCK	0		/* == 0 => this is an indirect blk */
	ID		b_ids[1];	/* the ids - actually bigger 	   */
} Block, IDList;

#define ALLIDS( idl )		((idl)->b_nmax == ALLIDSBLOCK)
#define INDIRECT_BLOCK( idl )	((idl)->b_nids == INDBLOCK)
#define IDL_NIDS(idl)           (idl ? (idl)->b_nids : (NIDS)0)

typedef size_t idl_iterator;

/* small hashtable implementation used in the entry cache -- the table
 * stores little identical structs, and relies on using a (void *) inside
 * the struct to store linkage information.
 */
typedef int (*HashTestFn)(const void *, const void *);
typedef unsigned long (*HashFn)(const void *, size_t);
typedef struct {
    u_long	offset;		/* offset of linkage info in user struct */
    u_long	size;		/* members in array below */
    HashFn	hashfn;		/* compute a hash value on a key */
    HashTestFn	testfn;		/* function to test if two entries are equal */
    void *	slot[1];	/* actually much bigger */
} Hashtable;

/* use this macro to find the offset of the linkage info into your structure
 * (required for hashtable to work correctly)
 * HASHLOC(struct mything, linkptr)
 */
#define HASHLOC(mem, node)      (u_long)&(((mem *)0L)->node)

struct backentry {
    Slapi_Entry			*ep_entry;	/* real entry		*/
    Slapi_Entry			*ep_vlventry;
    ID				ep_id;		/* entry id		*/
    char			ep_state;	/* state in the cache	*/
#define ENTRY_STATE_DELETED     0x1 /* entry is marked as deleted */
#define ENTRY_STATE_CREATING    0x2 /* entry is being created; don't touch it */
#define ENTRY_STATE_NOTINCACHE  0x4 /* cache_add failed; not in the cache */
    int				ep_refcnt;	/* entry reference cnt	*/
    void *			ep_dn_link;	/* linkage for the 3 hash */
    void *			ep_id_link;	/*     tables used for */
    void *			ep_uuid_link;	/*     looking up entries */
    struct backentry		*ep_lrunext;	/* for the cache	*/
    struct backentry		*ep_lruprev;	/* for the cache	*/
    PRLock			*ep_mutexp;	/* protection for mods 	*/
    size_t			size;		/* for cache tracking */
};

/* for the in-core cache of entries */
struct cache {
    size_t c_maxsize;		/* max size in bytes */
    size_t c_cursize;		/* size in bytes */
    long c_maxentries;		/* max entries allowed (-1: no limit) */
    long c_curentries;		/* current # entries in cache */
    Hashtable *c_dntable;
    Hashtable *c_idtable;
#ifdef UUIDCACHE_ON 
    Hashtable *c_uuidtable;
#endif
    u_long c_hits;		/* for analysis of hits/misses */
    u_long c_tries;
    struct backentry *c_lruhead;	/* add entries here */
    struct backentry *c_lrutail;	/* remove entries here */
    PRLock *c_mutex;			/* lock for cache operations */
    PRLock *c_emutexalloc_mutex;
};

/* various modules keep private data inside the attrinfo structure */
typedef struct dblayer_private dblayer_private;
typedef struct dblayer_private_env dblayer_private_env;
typedef struct idl_private idl_private;
typedef struct attrcrypt_private attrcrypt_private;


/* for the cache of attribute information (which are indexed, etc.) */
struct attrinfo {
	char	*ai_type;	  /* type name (cn, sn, ...)	*/
	int	ai_indexmask;	  /* how the attr is indexed	*/
#define INDEX_PRESENCE	0x01
#define INDEX_EQUALITY	0x02
#define INDEX_APPROX	0x04
#define INDEX_SUB	0x08
#define INDEX_UNKNOWN	0x10
#define INDEX_FROMINIT	0x20
#define INDEX_RULES	0x40
#define INDEX_VLV	0x80
#define	INDEX_ANY	(INDEX_PRESENCE | INDEX_EQUALITY | INDEX_APPROX | INDEX_SUB | INDEX_RULES | INDEX_VLV)

#define INDEX_OFFLINE   0x1000          /* index is being generated, or
                                         * has been created but not indexed
                                         * yet. */

#define	IS_INDEXED( a )	( a & INDEX_ANY )
	void	*ai_plugin; /* the syntax plugin for this attribute */
	char	**ai_index_rules; /* matching rule OIDs */
	void	*ai_dblayer;	  /* private data used by the dblayer code */
	PRInt32 ai_dblayer_count; /* used by the dblayer code */
	idl_private	*ai_idl;  /* private data used by the IDL code (eg locking the IDLs) */
	attrcrypt_private	*ai_attrcrypt;  /* private data used by the attribute encryption code (eg is it enabled or not) */
	value_compare_fn_type ai_key_cmp_fn; /* function used to compare two index keys -
											The function is the compare function provided by
											ai_plugin - this function is used to order
											the keys in the index so that we can use ORDERING
											searches.  In order for this function to be used,
											the syntax plugin must define a compare function,
											and either the attribute definition schema must
											specify an ORDERING matching rule, or the index
											configuration must define an ORDERING matching rule.
										 */
};

#define MAXDBCACHE	20

struct id_array {
  int  ida_next_index; /*The next index that is free*/
  int  ida_size;       /*The size of this puppy*/
  ID  *ida_ids;        /*The array of ids*/

};
typedef struct id_array Id_Array;

struct _db_upgrade_info {
	char *old_version_string;
	int old_dbversion_major;
	int old_dbversion_minor;
	int type;
	int action;
};
typedef struct _db_upgrade_info db_upgrade_info;
/* Values for dbversion_stuff->type */
#define DBVERSION_COMPATIBLE 0x10
#define DBVERSION_UPGRADABLE 0x20
#define DBVERSION_SOL 0x40
#define DBVERSION_OLD_IDL 0x1
#define DBVERSION_NEW_IDL 0x2

/* Values for dbversion_stuff->action + return value */
#define DBVERSION_NO_UPGRADE       0x0
#define DBVERSION_NEED_IDL_OLD2NEW 0x100
#define DBVERSION_NEED_IDL_NEW2OLD 0x200
#define DBVERSION_UPGRADE_3_4      0x400 /* bdb 3.3 -> 4.2 */
                                         /* The log file format changed; 
                                          * No database formats changed;
                                          * db extention: .db3 -> .db4
                                          */
#define DBVERSION_UPGRADE_4_4      0x800 /* bdb 4.2 -> 4.3 -> 4.4 -> 4.5 */
                                         /* The log file format changed; 
                                          * No database formats changed;
                                          * no db extention change
                                          */
#define DBVERSION_NOT_SUPPORTED    0x10000000

#define DBVERSION_TYPE   0x1
#define DBVERSION_ACTION 0x2

struct ldbminfo {
    int             li_mode;
    int             li_lookthroughlimit;
    int             li_allidsthreshold;
    char            *li_directory;
    int             li_reslimit_lookthrough_handle;
    size_t          li_dbcachesize;
    int             li_dbncache;
    int             li_import_cache_autosize; /* % of free memory to use
                                               * for the import caches
                                               * (-1=default, 80% on cmd import)
                                               * (0 = off) -- overrides 
                                               * import cache size settings */
    int             li_cache_autosize;        /* % of free memory to use
                                               * for the combined caches
                                               * (0 = off) -- overrides
                                               * other cache size settings */
    int             li_cache_autosize_split;  /* % of li_cache_autosize to
                                               * use for the libdb cache.
                                               * the rest is split up among
                                               * the instance entry caches */
    unsigned long   li_cache_autosize_ec;     /* new instances created while
                                               * the server is up, should
                                               * use this as the entry cache
                                               * size (0 = autosize off) */
    size_t          li_import_cachesize;      /* size of the mpool for
                                               * imports */
    PRLock          *li_dbcache_mutex;
    PRCondVar       *li_dbcache_cv;
    int             li_shutdown;              /* flag to tell any BE threads
                                               * to end */
    PRLock          *li_shutdown_mutex;       /* protect shutdown flag */
    dblayer_private *li_dblayer_private;      /* session ptr for databases */
    int             li_noparentcheck;         /* check if parent exists on 
                                               * add */

    /* the next 2 fields are for the params that don't get changed until
     * the server is restarted (used by the admin console)
     */
    char            *li_new_directory;
    size_t          li_new_dbcachesize;

    int             li_new_dbncache;
    
    db_upgrade_info *upgrade_info;
    int             li_filter_bypass;         /* bypass filter testing, 
                                               * when possible */
    int             li_filter_bypass_check;   /* check that filter bypass 
                                               * is doing the right thing */
    int             li_use_vlv;               /* use vlv indexes to short-
                                               * circuit matches when
                                               * possible */
    void            *li_identity;             /* The ldbm plugin needs to keep
                                               * track of its identity so it can
                                               * perform internal ops.  Its 
                                               * identity is given to it when
                                               * its init function is called. */

    Objset        *li_instance_set;           /* A set containing the ldbm
                                               * instances. */

    PRLock        *li_config_mutex;

    /* There are times when we need a pointer to the ldbm database
     * plugin, so we will store a pointer to it here.  Examples of
     * when we need it are when we create a new instance and when
     * we need the name of the plugin to do internal ops. */
    struct slapdplugin    *li_plugin;

    /* factory extension markers for the Connection struct -- bulk import
     * uses this to store state info on a Connection.
     */
    int li_bulk_import_object;
    int li_bulk_import_handle;
    /* maximum number of pass before merging the files during an import */
    int li_maxpassbeforemerge;

    /* charray of attributes to exclude from LDIF export */
    char **li_attrs_to_exclude_from_export;

    int li_flags;
    int li_fat_lock;         /* 608146 -- make this configurable, first */
    int li_legacy_errcode;   /* 615428 -- in case legacy err code is expected */
};

/* li_flags could store these bits defined in ../slapi-plugin.h
 * task flag (pb_task_flags) *
 *   SLAPI_TASK_RUNNING_AS_TASK
 *   SLAPI_TASK_RUNNING_FROM_COMMANDLINE
 */
/* allow conf w/o CONFIG_FLAG_ALLOW_RUNNING_CHANGE to be updated */
#define LI_FORCE_MOD_CONFIG     0x10 

/* Structure used to hold stuff for the lifetime of an LDAP transaction */
/* If we do clever stuff like LDAP transactions, we'll need a stack of TXN ID's */
typedef struct back_txn back_txn;
struct back_txn {
	DB_TXN	*back_txn_txn;		/* Transaction ID for the database */
};
typedef void * back_txnid;

#define RETRY_TIMES 50

/* Structure used to communicate information about subordinatecount on import/upgrade */
struct _import_subcount_stuff {
	PLHashTable *hashtable;
};
typedef struct _import_subcount_stuff import_subcount_stuff;

/* Handy structures for modify operations */

struct _modify_context {
        int new_entry_in_cache;
	struct backentry *old_entry;
	struct backentry *new_entry;
	Slapi_Mods *smods;
};
typedef struct _modify_context modify_context;

#define INSTANCE_DB_SUFFIX "-db"
#define INSTANCE_CHANGELOG_SUFFIX "-changelog"


/* This structure was moved here from dblayer.c because the ldbm_instance 
 * structure uses the dblayer_handle structure.  */
struct tag_dblayer_handle; typedef struct tag_dblayer_handle dblayer_handle;
struct tag_dblayer_handle
{
	DB*	dblayer_dbp;
	PRLock *dblayer_lock;	/* used when anyone wants exclusive access to a file */
	dblayer_handle *dblayer_handle_next;
	void **dblayer_handle_ai_backpointer; /* Voodo magic pointer to the place where we store a 
											pointer to this handle in the attrinfo structure */
};

/* This structure was moved here from perfctrs.c so the ldbm_instance structure
 * could use it. */
struct _perfctrs_private {
#if defined(_WIN32)
	/* Handle to the shared memory object */
	HANDLE hMemory;
	/* Handle to the update event */
	HANDLE hEvent;
#else
	/* Nothing yet */
#endif
	/* Pointer to the shared memory */
	void *memory;
};
typedef struct _perfctrs_private perfctrs_private;

typedef struct _attrcrypt_state_private attrcrypt_state_private;

/* flags for ldbm_instance */
/* please lock inst_config_mutex before changing inst_flags */
#define INST_FLAG_BUSY          0x0001  /* instance is doing an import or
                                         * restore. */
#define INST_FLAG_READONLY      0x0002  /* instance is truly readonly */

/* Structure used to hold instance specific information. */
typedef struct ldbm_instance {
    char *inst_name;                  /* Name given for this instance. */
    backend *inst_be;                 /* pointer back to the backend */
    struct ldbminfo *inst_li;         /* pointer back to global info */
    int inst_flags;                   /* see above */

    PRLock *inst_config_mutex;

    PRInt32 *inst_ref_count;          /* Keeps track of how many operations 
                                       * are currently using this instance */

    char *inst_dir_name;              /* The name of the directory in the db
                                       * directory that holds the index files
                                       * for this instance. Relative to the
                                       * parent of the instance name dir */
    char *inst_parent_dir_name;       /* Absolute parent dir for this inst */

    PRLock *inst_db_mutex;            /* Used to synchronize modify operations
                                       * on this instance. */

    dblayer_handle *inst_handle_head; /* These are used to maintain a list */
    dblayer_handle *inst_handle_tail; /* of open db handles for this instance */
    PRLock *inst_handle_list_mutex;
    
    DB *inst_id2entry;                /* id2entry for this instance. */

    perfctrs_private inst_perf_private; /* Private data for the performace
                                         * counters specific to this instance */
    attrcrypt_state_private *inst_attrcrypt_state_private;                                     
    int attrcrypt_configured;         /* Are any attributes configured for encryption ? */                            

    Avlnode *inst_attrs;              /* Keeps track of what's indexed for
                                       * this instance. */ 

    struct cache inst_cache;          /* The entry cache for this instance. */

    PRLock *inst_nextid_mutex;
    ID inst_nextid;

    PRCondVar *inst_indexer_cv;       /* indexer thread cond var */
    PRThread *inst_indexer_tid;       /* for the indexer thread */

    long inst_cache_hits;             /* used during imports to figure out when
                                       * a pass should end. */
    long inst_cache_misses;

    char *inst_dataversion;           /* The user data version tag.  Used by 
                                       * replication. */
    dblayer_private_env *import_env;  /* use a different DB_ENV for imports */
    int require_index;                /* set to 1 to require an index be used
                                       * in search */
} ldbm_instance;

/*
 * This structure is passed through the PBlock from ldbm_back_search to
 * ldbm_back_next_search_entry.  It contains the candidate result set
 * determined by ldbm_back_search, to be served up by ldbm_back_next_search_entry.
 */
typedef struct _back_search_result_set
{
    IDList*           sr_candidates;        /* the search results */
    idl_iterator      sr_current;           /* the current position in the search results */
    struct backentry* sr_entry;             /* the last entry returned */
    int               sr_lookthroughcount;  /* how many have we examined? */
    int               sr_lookthroughlimit;  /* how many can we examine? */
    int               sr_virtuallistview;   /* is this a VLV Search */
    Slapi_Entry*      sr_vlventry;          /* a special VLV Entry for when the ACL check fails */
	int				  sr_flags;				/* Magic flags, defined below */
} back_search_result_set;
#define SR_FLAG_CAN_SKIP_FILTER_TEST 1 /* If set in sr_flags, means that we can safely skip the filter test */

#include "proto-back-ldbm.h"
#include "ldbm_config.h"

/* flags used when adding/removing index items */
#define BE_INDEX_ADD		1
#define BE_INDEX_DEL		2
#define BE_INDEX_PRESENCE	4	/* (w/DEL) remove the presence index */
#define BE_INDEX_TOMBSTONE	8   /* Index entry as a tombstone */
#define BE_INDEX_DONT_ENCRYPT	16   /* Disable any encryption if this flag is set */
#define BE_INDEX_EQUALITY	32  /* (w/DEL) remove the equality index */
#define BE_INDEX_NORMALIZED SLAPI_ATTR_FLAG_NORMALIZED /* value already normalized */

/* Name of attribute type used for binder-based look through limit */
#define LDBM_LOOKTHROUGHLIMIT_AT	"nsLookThroughLimit"

/* OIDs for attribute types used internally */
#define LDBM_ENTRYDN_OID			"2.16.840.1.113730.3.1.602"
#define LDBM_DNCOMP_OID				"2.16.840.1.113730.3.1.603"
#define LDBM_PARENTID_OID			"2.16.840.1.113730.3.1.604"
#define LDBM_ENTRYID_OID			"2.16.840.1.113730.3.1.605"

/* Name of psuedo attribute used to track default indexes */
#define LDBM_PSEUDO_ATTR_DEFAULT	".default"

/* for checking disk full errors. */
#define LDBM_OS_ERR_IS_DISKFULL( err ) ((err)==ENOSPC || (err)==EFBIG)

/* flag: open_flag for dblayer_get_index_file -> dblayer_open_file */
#define DBOPEN_CREATE 0x1	/* oprinary mode: create a db file if needed */

/* whether we call fat lock or not [608146] */
#define SERIALLOCK(li)	(li->li_fat_lock)
#endif /* _back_ldbm_h_ */
