#ifndef _LITEZIP_H
#define _LITEZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
 * ZIP.H
 *
 * For creating zip files using LITEZIP.DLL.
 *
 * This file is a repackaged form of extracts from the zlib code available
 * at www.gzip.org/zlib, by Jean-Loup Gailly and Mark Adler. The original
 * copyright notice may be found in LiteZip.c. The repackaging was done
 * by Lucian Wischik to simplify and extend its use in Windows/C++. Also
 * encryption and unicode filenames have been added. Code was further
 * revamped and turned into a DLL (which supports both UNICODE and ANSI
 * C strings) by Jeff Glatt.
 */
// An HZIP identifies a zip archive that is being created
#define HZIP	void *

#define IDS_OK        20
#define IDS_UNKNOWN   21

// ================== CONSTANTS =====================

typedef unsigned char UCH;      // unsigned 8-bit value
typedef unsigned short USH;     // unsigned 16-bit value
typedef unsigned long ULG;      // unsigned 32-bit value

// internal file attribute
#define UNKNOWN (-1)
#define BINARY  0
#define ASCII   1

#define BEST		-1		// Use best method (deflation or store)
#define STORE		0		// Store method
#define DEFLATE		8		// Deflation method

// MSDOS file or directory attributes
#define MSDOS_HIDDEN_ATTR	0x02
#define MSDOS_DIR_ATTR		0x10

// Lengths of headers after signatures in bytes
#define LOCHEAD		26
#define CENHEAD		42
#define ENDHEAD		18

// Definitions for extra field handling:
#define EB_HEADSIZE			4		// length of a extra field block header
#define EB_LEN				2		// offset of data length field in header
#define EB_UT_MINLEN		1		// minimal UT field contains Flags byte
#define EB_UT_FLAGS			0		// byte offset of Flags field
#define EB_UT_TIME1			1		// byte offset of 1st time value
#define EB_UT_FL_MTIME		(1 << 0)	// mtime present
#define EB_UT_FL_ATIME		(1 << 1)	// atime present
#define EB_UT_FL_CTIME		(1 << 2)	// ctime present
#define EB_UT_LEN(n)		(EB_UT_MINLEN + 4 * (n))
#define EB_L_UT_SIZE		(EB_HEADSIZE + EB_UT_LEN(3))
#define EB_C_UT_SIZE		(EB_HEADSIZE + EB_UT_LEN(1))

// Signatures for zip file information headers
#define LOCSIG     0x04034b50L
#define CENSIG     0x02014b50L
#define ENDSIG     0x06054b50L
#define EXTLOCSIG  0x08074b50L

// The minimum and maximum match lengths
#define MIN_MATCH  3
#define MAX_MATCH  258

// Maximum window size = 32K. If you are really short of memory, compile
// with a smaller WSIZE but this reduces the compression ratio for files
// of size > WSIZE. WSIZE must be a power of two in the current implementation.
#define WSIZE  (0x8000)

// Minimum amount of lookahead, except at the end of the input file.
// See deflate.c for comments about the MIN_MATCH+1.
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)

// In order to simplify the code, particularly on 16 bit machines, match
// distances are limited to MAX_DIST instead of WSIZE.
#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)

// All codes must not exceed MAX_BITS bits
#define MAX_BITS		15

// Bit length codes must not exceed MAX_BL_BITS bits
#define MAX_BL_BITS		7

// number of length codes, not counting the special END_BLOCK code
#define LENGTH_CODES	29

// number of literal bytes 0..255
#define LITERALS		256

// end of block literal code
#define END_BLOCK		256

// number of Literal or Length codes, including the END_BLOCK code
#define L_CODES			(LITERALS+1+LENGTH_CODES)

// number of distance codes
#define D_CODES			30

// number of codes used to transfer the bit lengths
#define BL_CODES		19

// The three kinds of block type
#define STORED_BLOCK	0
#define STATIC_TREES	1
#define DYN_TREES		2

// Sizes of match buffers for literals/lengths and distances.  There are
// 4 reasons for limiting LIT_BUFSIZE to 64K:
//   - frequencies can be kept in 16 bit counters
//   - if compression is not successful for the first block, all input data is
//     still in the window so we can still emit a stored block even when input
//     comes from standard input.  (This can also be done for all blocks if
//     LIT_BUFSIZE is not greater than 32K.)
//   - if compression is not successful for a file smaller than 64K, we can
//     even emit a stored file instead of a stored block (saving 5 bytes).
//   - creating new Huffman trees less frequently may not provide fast
//     adaptation to changes in the input data statistics. (Take for
//     example a binary file with poorly compressible code followed by
//     a highly compressible string table.) Smaller buffer sizes give
//     fast adaptation but have of course the overhead of transmitting trees
//     more frequently.
//   - I can't count above 4
// The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
// memory at the expense of compression). Some optimizations would be possible
// if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
#define LIT_BUFSIZE		0x8000
#define DIST_BUFSIZE	LIT_BUFSIZE

// repeat previous bit length 3-6 times (2 bits of repeat count)
#define REP_3_6			16

// repeat a zero length 3-10 times  (3 bits of repeat count)
#define REPZ_3_10		17

// repeat a zero length 11-138 times  (7 bits of repeat count)
#define REPZ_11_138		18

// maximum heap size
#define HEAP_SIZE		(2*L_CODES+1)

// Number of bits used within bi_buf. (bi_buf may be implemented on
// more than 16 bits on some systems.)
#define ZIP_BUF_SIZE	(8 * 2*sizeof(char))

// For portability to 16 bit machines, do not use values above 15.
#define HASH_BITS	15

#define HASH_SIZE	(unsigned)(1<<HASH_BITS)
#define HASH_MASK	(HASH_SIZE-1)
#define WMASK		(WSIZE-1)
// HASH_SIZE and WSIZE must be powers of two

#define FAST		4
#define SLOW		2
// speed options for the general purpose bit flag

#define TOO_FAR		4096
// Matches of length 3 are discarded if their distance exceeds TOO_FAR

// Number of bits by which ins_h and del_h must be shifted at each
// input step. It must be such that after MIN_MATCH steps, the oldest
// byte no longer takes part in the hash key, that is:
//   H_SHIFT * MIN_MATCH >= HASH_BITS
#define H_SHIFT		((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)

// Index within the heap array of least frequent node in the Huffman tree
#define SMALLEST 1















// ================== STRUCTS =====================

typedef struct {
	USH good_length; // reduce lazy search above this match length
	USH max_lazy;    // do not perform lazy search above this match length
	USH nice_length; // quit search above this match length
	USH max_chain;
} CONFIG;

// Data structure describing a single value and its code string.
typedef struct {
	union {
		USH		freq;		// frequency count
		USH		code;		// bit string
	} fc;
	union {
		USH		dad;		// father node in Huffman tree
		USH		len;		// length of bit string
	} dl;
} CT_DATA;

typedef struct {
	CT_DATA		*dyn_tree;		// the dynamic tree
	CT_DATA		*static_tree;	// corresponding static tree or NULL
	const int	*extra_bits;	// extra bits for each code or NULL
	int			extra_base;		// base index for extra_bits
	int			elems;			// max number of elements in the tree
	int			max_length;		// max bit length for the codes
	int			max_code;		// largest code with non zero frequency
} TREE_DESC;

typedef struct
{
	// ... Since the bit lengths are imposed, there is no need for the L_CODES
	// extra codes used during heap construction. However the codes 286 and 287
	// are needed to build a canonical tree (see ct_init below).
	CT_DATA		dyn_ltree[HEAP_SIZE];		// literal and length tree
	CT_DATA		dyn_dtree[2*D_CODES+1];		// distance tree
	CT_DATA		static_ltree[L_CODES+2];	// the static literal tree...
	CT_DATA		static_dtree[D_CODES];		// the static distance tree...
	// ... (Actually a trivial tree since all codes use 5 bits.)
	CT_DATA		bl_tree[2*BL_CODES+1];		// Huffman tree for the bit lengths

	TREE_DESC	l_desc;
	TREE_DESC	d_desc;
	TREE_DESC	bl_desc;

	USH			bl_count[MAX_BITS+1];	// number of codes at each bit length for an optimal tree

	// The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
	// The same heap array is used to build all trees.
	int			heap[2*L_CODES+1];		// heap used to build the Huffman trees
	int			heap_len;				// number of elements in the heap
	int			heap_max;				// element of largest frequency

	// Depth of each subtree used as tie breaker for trees of equal frequency
	UCH			depth[2*L_CODES+1];

	UCH			length_code[MAX_MATCH-MIN_MATCH+1];
	// length code for each normalized match length (0 == MIN_MATCH)

	// distance codes. The first 256 values correspond to the distances
	// 3 .. 258, the last 256 values correspond to the top 8 bits of
	// the 15 bit distances.
	UCH			dist_code[512];

	// First normalized length for each code (0 = MIN_MATCH)
	int			base_length[LENGTH_CODES];

	// First normalized distance for each code (0 = distance of 1)
	int			base_dist[D_CODES];

	UCH			l_buf[LIT_BUFSIZE];		// buffer for literals/lengths
	USH			d_buf[DIST_BUFSIZE];	// buffer for distances

	// flag_buf is a bit array distinguishing literals from lengths in
	// l_buf, and thus indicating the presence or absence of a distance.
	UCH			flag_buf[(LIT_BUFSIZE/8)];

	// bits are filled in flags starting at bit 0 (least significant).
	// Note: these flags are overkill in the current code since we don't
	// take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
	unsigned	last_lit;			// running index in l_buf
	unsigned	last_dist;			// running index in d_buf
	unsigned	last_flags;			// running index in flag_buf
	UCH			flags;				// current flags not yet saved in flag_buf
	UCH			flag_bit;			// current bit used in flags

	ULG			opt_len;			// bit length of current block with optimal trees
	ULG			static_len;			// bit length of current block with static trees

	ULG			cmpr_bytelen;		// total byte length of compressed file (within the ZIP)
	ULG			cmpr_len_bits;		// number of bits past 'cmpr_bytelen'

	USH			*file_type;			// pointer to UNKNOWN, BINARY or ASCII

/*#ifndef NDEBUG
	// input_len is for debugging only since we can't get it by other means.
	ULG			input_len;			// total byte length of source file
#endif*/

} TTREESTATE;

typedef struct
{
	unsigned	bi_buf;			// Output buffer. bits are inserted starting at the bottom (least significant
								// bits). The width of bi_buf must be at least 16 bits.
	int			bi_valid;		// Number of valid bits in bi_buf. All bits above the last valid bit are always zero.
	char		*out_buf;		// Current output buffer.
	DWORD		out_offset;		// Current offset in output buffer
	DWORD		out_size;		// Size of current output buffer
/*#ifndef NDEBUG
	ULG			bits_sent;		// bit length of the compressed data  only needed for debugging
#endif*/
} TBITSTATE;

typedef struct
{
	// Sliding window. Input bytes are read into the second half of the window,
	// and move to the first half later to keep a dictionary of at least WSIZE
	// bytes. With this organization, matches are limited to a distance of
	// WSIZE-MAX_MATCH bytes, but this ensures that IO is always
	// performed with a length multiple of the block size. Also, it limits
	// the window size to 64K, which is quite useful on MSDOS.
	// To do: limit the window size to WSIZE+CBSZ if SMALL_MEM (the code would
	// be less efficient since the data would have to be copied WSIZE/CBSZ times)
	UCH				window[2L*WSIZE];

	// Link to older string with same hash index. To limit the size of this
	// array to 64K, this link is maintained only for the last 32K strings.
	// An index in this array is thus a window index modulo 32K.
	unsigned		prev[WSIZE];

	// Heads of the hash chains or 0. If your compiler thinks that
	// HASH_SIZE is a dynamic value, recompile with -DDYN_ALLOC.
	unsigned		head[HASH_SIZE];

	// window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
	// input file length plus MIN_LOOKAHEAD.
	ULG				window_size;

	// window position at the beginning of the current output block. Gets
	// negative when the window is moved backwards.
	long			block_start;

	// hash index of string to be inserted
	unsigned		ins_h;

	// Length of the best match at previous step. Matches not greater than this
	// are discarded. This is used in the lazy match evaluation.
	unsigned int	prev_length;

	unsigned		strstart;		// start of string to insert
	unsigned		match_start;	// start of matching string
	unsigned		lookahead;		// number of valid bytes ahead in window

	// To speed up deflation, hash chains are never searched beyond this length.
	// A higher limit improves compression ratio but degrades the speed.
	unsigned		max_chain_length;

	// Attempt to find a better match only when the current match is strictly
	// smaller than this value. This mechanism is used only for compression
	// levels >= 4.
	unsigned int	max_lazy_match;
	unsigned		good_match;		// Use a faster search when the previous match is longer than this
	int				nice_match;		// Stop searching when current match exceeds this

	unsigned char	eofile;			// flag set at end of source file
	unsigned char	sliding;		// Set to false when the source is already in memory

} TDEFLATESTATE;

typedef struct
{
	struct _TZIP	*tzip;
	TTREESTATE		ts;
	TBITSTATE		bs;
	TDEFLATESTATE	ds;
/*#ifndef NDEBUG
	const char		*err;
#endif*/
	unsigned char	level;		// compression level
//	unsigned char	seekable;	// 1 if we can seek() in the source
}  TSTATE;

typedef long lutime_t;       // define it ourselves since we don't include time.h

// Holds the Access, Modify, Create times, and DOS timestamp. Also the file attributes
typedef struct {
	lutime_t		atime, mtime, ctime;
	unsigned long	timestamp;
	unsigned long	attributes;
} IZTIMES;

// For storing values to be written to the ZIP Central Directory. Note: We write
// default values for some of the fields
typedef struct _TZIPFILEINFO {
	USH			flg, how;
	ULG			tim, crc, siz, len;
	DWORD		nam, ext, cext;			// offset of ext must be >= LOCHEAD
	USH			dsk, att, lflg;			// offset of lflg must be >= LOCHEAD
	ULG			atx, off;
	char		*extra;					// Extra field (set only if ext != 0)
	char		*cextra;				// Extra in central (set only if cext != 0)
	char		iname[MAX_PATH];		// Internal file name after cleanup
	struct _TZIPFILEINFO	*nxt;		// Pointer to next header in list
} TZIPFILEINFO;

// For TZIP->flags
#define TZIP_DESTMEMORY			0x0000001	// Set if TZIP->destination is memory, instead of a file, handle
#define TZIP_DESTCLOSEFH		0x0000002	// Set if we open the file handle in zipCreate() and must close it later
#define TZIP_CANSEEK			0x0000004	// Set if the destination (where we write the ZIP) is seekable
#define TZIP_DONECENTRALDIR		0x0000008	// Set after we've written out the Central Directory
#define TZIP_ENCRYPT			0x0000010	// Set if we should apply encrpytion to the output
#define TZIP_SRCCANSEEK			0x0000020	// Set if source (that supplies the data to add to the ZIP file) is seekable
#define TZIP_SRCCLOSEFH			0x0000040	// Set if we've opened the source file handle, and therefore need to close it
#define TZIP_SRCMEMORY			0x0000080	// Set if TZIP->source is memory, instead of a file, handle
//#define TZIP_OPTION_GZIP		0x8000000	// Defined in LiteZip.h. Must not be used for another purpose

typedef struct _TZIP
{
	DWORD		flags;

	// ====================================================================
	// These variables are for the destination (that we're writing the ZIP to).
	// We can write to pipe, file-by-handle, file-by-name, memory-to-memmapfile
	HANDLE		destination;	// If not TZIP_DESTMEMORY, this is the handle to the zip file we write to. Otherwise.
								// it points to a memory buffer where we write the zip.
	char		*password;		// A copy of the password from the application.
	DWORD		writ;			// How many bytes we've written to destination. This is maintained by addSrc(), not
								// writeDestination(), to avoid confusion over seeks.
	DWORD		ooffset;		// The initial offset where we start writing the zip within destination. (This allows
								// the app to write the zip to an already open file that has data in it).
	DWORD		lasterr;		// The last error code.

	// Memory map stuff
	HANDLE		memorymap;		// If not 0, then this is a memory mapped file handle.
	DWORD		opos;			// Current (byte) position in "destination".
	DWORD		mapsize;		// The size of the memory buffer.

	// Encryption
	unsigned long keys[3];		// keys are initialised inside addSrc()
	char		*encbuf;		// If encrypting, then this is a temporary workspace for encrypting the data (to
	unsigned int encbufsize;	// be used and resized inside writeDestination(), and deleted when the TZIP is freed)

	TZIPFILEINFO	*zfis;		// Each file gets added onto this list, for writing the table at the end

	// ====================================================================
	// These variables are for the source (that supplies the data to be added to the ZIP)
	DWORD		isize, totalRead;	// size is not set until close() on pipes
	ULG			crc;				// crc is not set until close(). iwrit is cumulative
	HANDLE		source;
	DWORD		lenin, posin;		// These are for a memory buffer source
	// and a variable for what we've done with the input: (i.e. compressed it!)
	ULG			csize;				// Compressed size, set by the compression routines.
	TSTATE		*state;				// We allocate just one state object per zip, because it's big (500k), and store a ptr here. It is freed when the TZIP is freed
	char		buf[16384];			// Used by some of the compression routines. This must be last!!
} TZIP;

// Functions to create a ZIP archive
//DWORD WINAPI ZipCreateFileA(HZIP *, const char *, const char *);
//DWORD WINAPI ZipCreateFileW(HZIP *, const WCHAR *, const char *);
#ifdef UNICODE
#define ZipCreateFile ZipCreateFileW
#define ZIPCREATEFILENAME "ZipCreateFileW"
typedef DWORD WINAPI ZipCreateFilePtr(HZIP *, const WCHAR *, const char *);
#else
#define ZipCreateFile ZipCreateFileA
#define ZIPCREATEFILENAME "ZipCreateFileA"
typedef DWORD WINAPI ZipCreateFilePtr(HZIP *, const char *, const char *);
#endif
DWORD WINAPI ZipCreateBuffer(HZIP *, void *, DWORD, const char *);
#define ZIPCREATEBUFFERNAME "ZipCreateBuffer"
typedef DWORD WINAPI ZipCreateBufferPtr(HZIP *, void *, DWORD, const char *);
DWORD WINAPI ZipCreateHandle(HZIP *, HANDLE, const char *);
#define ZIPCREATEHANDLENAME "ZipCreateHandle"
typedef DWORD WINAPI ZipCreateHandlePtr(HZIP *, HANDLE, const char *);

// Functions for adding a "file" to a ZIP archive
//DWORD WINAPI ZipAddFileW(HZIP, const WCHAR *, const WCHAR *);
//DWORD WINAPI ZipAddFileA(HZIP, const char *, const char *);
#ifdef UNICODE
#define ZipAddFile ZipAddFileW
#define ZIPADDFILENAME "ZipAddFileW"
typedef DWORD WINAPI ZipAddFilePtr(HZIP, const WCHAR *, const WCHAR *);
#else
#define ZipAddFile ZipAddFileA
#define ZIPADDFILENAME "ZipAddFileA"
typedef DWORD WINAPI ZipAddFilePtr(HZIP, const char *, const char *);
#endif

DWORD WINAPI ZipAddHandleW(HZIP, const WCHAR *, HANDLE);
DWORD WINAPI ZipAddHandleA(HZIP, const char *, HANDLE);
#ifdef UNICODE
#define ZipAddHandle ZipAddHandleW
#define ZIPADDHANDLENAME "ZipAddHandleW"
typedef DWORD WINAPI ZipAddHandlePtr(HZIP, const WCHAR *, HANDLE);
#else
#define ZipAddHandle ZipAddHandleA
#define ZIPADDHANDLENAME "ZipAddHandleA"
typedef DWORD WINAPI ZipAddHandlePtr(HZIP, const char *, HANDLE);
#endif

DWORD WINAPI ZipAddPipeW(HZIP, const WCHAR *, HANDLE, DWORD);
DWORD WINAPI ZipAddPipeA(HZIP, const char *, HANDLE, DWORD);
#ifdef UNICODE
#define ZipAddPipe ZipAddPipeW
#define ZIPADDPIPENAME "ZipAddPipeW"
typedef DWORD WINAPI ZipAddPipePtr(HZIP, const WCHAR *, HANDLE, DWORD);
#else
#define ZipAddPipe ZipAddPipeA
#define ZIPADDPIPENAME "ZipAddPipeA"
typedef DWORD WINAPI ZipAddPipePtr(HZIP, const char *, HANDLE, DWORD);
#endif

//DWORD WINAPI ZipAddBufferW(HZIP, const WCHAR *, const void *, DWORD);
//DWORD WINAPI ZipAddBufferA(HZIP, const char *, const void *, DWORD);
#ifdef UNICODE
#define ZipAddBuffer ZipAddBufferW
#define ZIPADDBUFFERNAME "ZipAddFolderW"
typedef DWORD WINAPI ZipAddBufferPtr(HZIP, const WCHAR *, const void *, DWORD);
#else
#define ZipAddBuffer ZipAddBufferA
#define ZIPADDBUFFERNAME "ZipAddBufferA"
typedef DWORD WINAPI ZipAddBufferPtr(HZIP, const char *, const void *, DWORD);
#endif

//DWORD WINAPI ZipAddFolderW(HZIP, const WCHAR *);
//DWORD WINAPI ZipAddFolderA(HZIP, const char *);
#ifdef UNICODE
#define ZipAddFolder ZipAddFolderW
#define ZIPADDFOLDERNAME "ZipAddFolderW"
typedef DWORD WINAPI ZipAddFolderPtr(HZIP, const WCHAR *);
#else
#define ZipAddFolder ZipAddFolderA
#define ZIPADDFOLDERNAME "ZipAddFolderA"
typedef DWORD WINAPI ZipAddFolderPtr(HZIP, const char *);
#endif

//DWORD WINAPI ZipAddDirW(HZIP, const WCHAR *, DWORD);
//DWORD WINAPI ZipAddDirA(HZIP, const char *, DWORD);
#ifdef UNICODE
#define ZipAddDir ZipAddDirW
#define ZIPADDDIRNAME "ZipAddDirW"
//typedef DWORD WINAPI ZipAddDirPtr(HZIP, const WCHAR *, DWORD);
#else
#define ZipAddDir ZipAddDirA
#define ZIPADDDIRNAME "ZipAddDirA"
//typedef DWORD WINAPI ZipAddDirPtr(HZIP, const char *, DWORD);
#endif

// Function to get a pointer to the ZIP archive created in memory by ZipCreateBuffer(0, len)
DWORD WINAPI ZipGetMemory(HZIP, void **, unsigned long *, HANDLE *);
#define ZIPGETMEMORYNAME "ZipGetMemory"
typedef DWORD WINAPI ZipGetMemoryPtr(HZIP, void **, DWORD *, HANDLE *);

// Function to reset a memory-buffer ZIP archive to compress a new ZIP archive
DWORD WINAPI ZipResetMemory(HZIP);
#define ZIPRESETMEMORYNAME "ZipResetMemory"
typedef DWORD WINAPI ZipResetMemoryPtr(HZIP);

// Function to close the ZIP archive
DWORD WINAPI ZipClose(HZIP);
#define ZIPCLOSENAME "ZipClose"
//typedef DWORD WINAPI ZipClosePtr(HZIP);

// Function to set options for ZipAdd* functions
DWORD WINAPI ZipOptions(HZIP, DWORD);
#define ZIPOPTIONSNAME "ZipOptions"
typedef DWORD WINAPI ZipOptionsPtr(HZIP, DWORD);

#define TZIP_OPTION_GZIP	0x80000000

// Function to get an appropriate error message for a given error code return by Zip functions
DWORD WINAPI ZipFormatMessageW(DWORD, WCHAR *, DWORD);
DWORD WINAPI ZipFormatMessageA(DWORD, char *, DWORD);
#ifdef UNICODE
#define ZipFormatMessage ZipFormatMessageW
#define ZIPFORMATMESSAGENAME "ZipFormatMessageW"
typedef DWORD WINAPI ZipFormatMessagePtr(DWORD, WCHAR *, DWORD);
#else
#define ZipFormatMessage ZipFormatMessageA
#define ZIPFORMATMESSAGENAME "ZipFormatMessageA"
typedef DWORD WINAPI ZipFormatMessagePtr(DWORD, char *, DWORD);
#endif

#if !defined(ZR_OK)
// These are the return codes from Zip functions
#define ZR_OK			0		// Success
// The following come from general system stuff (e.g. files not openable)
#define ZR_NOFILE		1		// Can't create/open the file
#define ZR_NOALLOC		2		// Failed to allocate memory
#define ZR_WRITE		3		// A general error writing to the file
#define ZR_NOTFOUND		4		// Can't find the specified file in the zip
#define ZR_MORE			5		// There's still more data to be unzipped
#define ZR_CORRUPT		6		// The zipfile is corrupt or not a zipfile
#define ZR_READ			7		// An error reading the file
#define ZR_NOTSUPPORTED	8		// The entry is in a format that can't be decompressed by this Unzip add-on
// The following come from mistakes on the part of the caller
#define ZR_ARGS			9		// Bad arguments passed
#define ZR_NOTMMAP		10		// Tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
#define ZR_MEMSIZE		11		// The memory-buffer size is too small
#define ZR_FAILED		12		// Already failed when you called this function
#define ZR_ENDED		13		// The zip creation has already been closed
#define ZR_MISSIZE		14		// The source file size turned out mistaken
#define ZR_ZMODE		15		// Tried to mix creating/opening a zip
// The following come from bugs within the zip library itself
#define ZR_SEEK			16		// trying to seek in an unseekable file
#define ZR_NOCHANGE		17		// changed its mind on storage, but not allowed
#define ZR_FLATE		18		// An error in the de/inflation code
#define ZR_PASSWORD		19		// Password is incorrect
#endif

#ifdef __cplusplus
}
#endif

DWORD createZip(HZIP *zipHandle, void *z, DWORD len, DWORD flags, const char *pwd);
DWORD addSrc(register TZIP *tzip, const void *destname, const void *src, DWORD len, DWORD flags);
#endif // _LITEZIP_H
