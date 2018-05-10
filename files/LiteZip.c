#include <windows.h>
#include "LiteZip.h"

// ========================== DATA ============================

// NOTE: I specify this data section to be Shared (ie, each running rexx
// script shares these variables, rather than getting its own copies of these
// variables). This is because, since I have only globals that are read-only
// or whose value is the same for all processes, I don't need a separate copy
// of these for each process that uses this DLL. In Visual C++'s Linker
// settings, I add "/section:Shared,rws"
#ifdef VISUAL_STUDIO
  #pragma data_seg("shared")
#endif

static HINSTANCE	ThisInstance;

static const int Extra_lbits[LENGTH_CODES] // extra bits for each length code
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static const int Extra_dbits[D_CODES] // extra bits for each distance code
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static const int Extra_blbits[BL_CODES]// extra bits for each bit length code
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

// Values for max_lazy_match, good_match, nice_match and max_chain_length,
// depending on the desired pack level (0..9). The values given below have
// been tuned to exclude worst case performance for pathological files.
// Better values may be found for specific files.
//

static const CONFIG ConfigurationTable[10] = {
	//  good lazy nice chain
	{0,    0,  0,    0},  // 0 store only
	{4,    4,  8,    4},  // 1 maximum speed, no lazy matches
	{4,    5, 16,    8},  // 2
	{4,    6, 32,   32},  // 3
	{4,    4, 16,   16},  // 4 lazy matches */
	{8,   16, 32,   32},  // 5
	{8,   16, 128, 128},  // 6
	{8,   32, 128, 256},  // 7
	{32, 128, 258, 1024}, // 8
	{32, 258, 258, 4096}};// 9 maximum compression */

// Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
// For deflate_fast() (levels <= 3) good is ignored and lazy has a different meaning.

static const ULG CrcTable[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

static const char ZipSuffixes[] = {".z\0"
".zip\0"
".zoo\0"
".arc\0"
".lzh\0"
".arj\0"
".gz\0"
".tgz\0"};

// The lengths of the bit length codes are sent in order of decreasing
// probability, to avoid transmitting the lengths for unused bit length codes.
static const UCH BL_order[BL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};


#ifdef VISUAL_STUDIO
  #pragma data_seg()
#endif
// ====================== LOCAL DECLARATIONS =======================

static void			writeDestShort(TZIP *, DWORD);
static void			writeDestination(TZIP *, const char *, DWORD);
static unsigned		readFromSource(register TZIP *, char *buf, unsigned size);
static void			pqdownheap(register TSTATE *, CT_DATA *, int);
static void			gen_codes(register TSTATE *, CT_DATA *, int);
static void			compress_block(register TSTATE *, CT_DATA *, CT_DATA *);
static BOOL			send_bits(register TSTATE *, int, int);
static unsigned		bi_reverse(register unsigned, register unsigned char);
static void			bi_windup(register TSTATE *);
static void			copy_block(register TSTATE *, char *, DWORD, DWORD);
static void			fill_window(register TSTATE *);

// ==================== Unix/Windows Time conversion ====================

void filetime2dosdatetime(const FILETIME ft, WORD *dosdate,WORD *dostime)
{
	// date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
	// time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
	SYSTEMTIME st;

	FileTimeToSystemTime(&ft, &st);
	*dosdate = (WORD)(((st.wYear-1980) & 0x7f) << 9);
	*dosdate |= (WORD)((st.wMonth&0xf) << 5);
	*dosdate |= (WORD)((st.wDay&0x1f));
	*dostime = (WORD)((st.wHour&0x1f) << 11);
	*dostime |= (WORD)((st.wMinute&0x3f) << 5);
	*dostime |= (WORD)((st.wSecond*2)&0x1f);
}

lutime_t filetime2timet(const FILETIME ft)
{
	LONGLONG i;

	i = *(LONGLONG*)&ft;
	return (lutime_t)((i - 116444736000000000)/10000000);
}

void getNow(lutime_t *pft, WORD *dosdate, WORD *dostime)
{
	SYSTEMTIME	st;
	FILETIME	ft;

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	filetime2dosdatetime(ft, dosdate, dostime);
	*pft = filetime2timet(ft);
}

DWORD getFilePos(HANDLE hfout)
{
	return(SetFilePointer(hfout, 0, 0, FILE_CURRENT));
}

/********************* getFileInfo() ***********************
 * Retrieves the attributes, size, and timestamps (in ZIP
 * format) from the file handle.
 */

DWORD getFileInfo(TZIP *tzip, IZTIMES *times)
{
	BY_HANDLE_FILE_INFORMATION	bhi;
	ULG							a;

	// The date and time is returned in a long with the date most significant to allow
	// unsigned integer comparison of absolute times. The attributes have two
	// high bytes unix attr, and two low bytes a mapping of that to DOS attr.

	// struct stat s; int res=stat(fn,&s); if (res!=0) return false;
	// translate windows file attributes into zip ones.
	if (!GetFileInformationByHandle(tzip->source, &bhi)) return(ZR_NOFILE);

	a = 0;

	// Zip uses the lower word for its interpretation of windows stuff
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_READONLY) a |= 0x01;
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) a |= 0x02;
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) a |= 0x04;
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) a |= 0x10;
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) a |= 0x20;
	// It uses the upper word for standard unix attr, which we manually construct
	if (bhi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		a |= 0x40000000;	// directory
	else
		a |= 0x80000000;	// normal file
	a |= 0x01000000;		// readable
	if (!(bhi.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
		a |= 0x00800000;	// writeable

	// Check if it's an executable
	if ((tzip->isize = GetFileSize(tzip->source, 0)) > 40 && !tzip->ooffset)
	{
		DWORD			read;
		unsigned short	magic;
		unsigned long	hpos;

		SetFilePointer(tzip->source, 0, 0, FILE_BEGIN);
		ReadFile(tzip->source, &magic, sizeof(magic), &read, 0);
		SetFilePointer(tzip->source, 36, 0, FILE_BEGIN);
		ReadFile(tzip->source, &hpos, sizeof(hpos), &read, 0);
		if (magic == 0x54AD && tzip->isize > hpos+4+20+28)
		{
			unsigned long signature;

			SetFilePointer(tzip->source, hpos, 0, FILE_BEGIN);
			ReadFile(tzip->source, &signature, sizeof(signature), &read, 0);
			if ((signature == IMAGE_DOS_SIGNATURE) || (signature == IMAGE_OS2_SIGNATURE) || (signature == IMAGE_OS2_SIGNATURE_LE) || (signature == IMAGE_NT_SIGNATURE))
				a |= 0x00400000; // executable
		}

		// Reset file pointer back to where it initially was
		SetFilePointer(tzip->source, 0, 0, FILE_BEGIN);
	}

	times->attributes = a;

	// lutime_t is 32bit number of seconds elapsed since 0:0:0GMT, Jan1, 1970.
	// but FILETIME is 64bit number of 100-nanosecs since Jan1, 1601
	times->atime = filetime2timet(bhi.ftLastAccessTime);
	times->mtime = filetime2timet(bhi.ftLastWriteTime);
	times->ctime = filetime2timet(bhi.ftCreationTime);

	{
	WORD	dosdate,dostime;

	filetime2dosdatetime(bhi.ftLastWriteTime, &dosdate, &dostime);
	times->timestamp = (WORD)dostime | (((DWORD)dosdate)<<16);
	}

	return(ZR_OK);
}


// ==================== DEBUG STUFF ====================

/*#ifndef NDEBUG
void Assert(TSTATE *state, BOOL cond, const char *msg)
{
	if (!cond) state->err = msg;
}

void Trace(const char *x, ...)
{
	va_list paramList;
	va_start(paramList, x);
	paramList;
	va_end(paramList);
}
#endif*/

// =================== Low level compression ==================

// Send a code of the given tree. c and tree must not have side effects
#define send_code(state, c, tree) send_bits(state, tree[c].fc.code, tree[c].dl.len)

// Mapping from a distance to a distance code. dist is the distance - 1 and
// must not have side effects. dist_code[256] and dist_code[257] are never used.
#define d_code(dist) ((dist) < 256 ? state->ts.dist_code[dist] : state->ts.dist_code[256+((dist)>>7)])

// the arguments must not have side effects
#define Max(a,b) (a >= b ? a : b)

/********************* init_block() *********************
 * Initializes a new block.
 */

void init_block(register TSTATE *state)
{
    int n; // iterates over tree elements

    // Initialize the trees
    for (n = 0; n < L_CODES;  n++) state->ts.dyn_ltree[n].fc.freq = 0;
    for (n = 0; n < D_CODES;  n++) state->ts.dyn_dtree[n].fc.freq = 0;
    for (n = 0; n < BL_CODES; n++) state->ts.bl_tree[n].fc.freq = 0;

    state->ts.dyn_ltree[END_BLOCK].fc.freq = 1;
    state->ts.opt_len = state->ts.static_len = 0;
    state->ts.last_lit = state->ts.last_dist = state->ts.last_flags = 0;
    state->ts.flags = 0; state->ts.flag_bit = 1;
}


/************************ ct_init() ******************
 * Allocates the match buffer, initializes the various
 * tables and saves the location of the internal file
 * attribute (ascii/binary) and method (DEFLATE/STORE).
 */

void ct_init(register TSTATE *state, USH *attr)
{
	int		n;			// iterates over tree elements
	int		bits;		// bit counter
	int		length;		// length value
	int		code;		// code value
	int		dist;		// distance index

	state->ts.file_type = attr;

	state->ts.cmpr_bytelen = state->ts.cmpr_len_bits = 0;
/*#ifndef NDEBUG
	state->ts.input_len = 0;
#endif*/

	if (state->ts.static_dtree[0].dl.len) return;	// ct_init already called

	// Initialize the mapping length (0..255) -> length code (0..28)
	length = 0;
	for (code = 0; code < LENGTH_CODES - 1; code++)
	{
		state->ts.base_length[code] = length;
		for (n = 0; n < (1 << Extra_lbits[code]); n++)
			state->ts.length_code[length++] = (UCH)code;
	}

	// Note that the length 255 (match length 258) can be represented
	// in two different ways: code 284 + 5 bits or code 285, so we
	// overwrite length_code[255] to use the best encoding:
	state->ts.length_code[length - 1] = (UCH)code;

	// Initialize the mapping dist (0..32K) -> dist code (0..29)
	dist = 0;
	for (code = 0 ; code < 16; code++)
	{
		state->ts.base_dist[code] = dist;
		for (n = 0; n < (1 << Extra_dbits[code]); n++)
			state->ts.dist_code[dist++] = (UCH)code;
	}

	dist >>= 7; // from now on, all distances are divided by 128
	for ( ; code < D_CODES; code++)
	{
		state->ts.base_dist[code] = dist << 7;
		for (n = 0; n < (1 << (Extra_dbits[code] - 7)); n++)
			state->ts.dist_code[256 + dist++] = (UCH)code;
	}

	// Construct the codes of the static literal tree
	for (bits = 0; bits <= MAX_BITS; bits++) state->ts.bl_count[bits] = 0;
	n = 0;
	while (n <= 143) state->ts.static_ltree[n++].dl.len = 8, state->ts.bl_count[8]++;
	while (n <= 255) state->ts.static_ltree[n++].dl.len = 9, state->ts.bl_count[9]++;
	while (n <= 279) state->ts.static_ltree[n++].dl.len = 7, state->ts.bl_count[7]++;
	while (n <= 287) state->ts.static_ltree[n++].dl.len = 8, state->ts.bl_count[8]++;
	// fc.codes 286 and 287 do not exist, but we must include them in the
	// tree construction to get a canonical Huffman tree (longest code
	// all ones)
	gen_codes(state, (CT_DATA *)state->ts.static_ltree, L_CODES+1);

	// The static distance tree is trivial
	for (n = 0; n < D_CODES; n++)
	{
		state->ts.static_dtree[n].dl.len = 5;
		state->ts.static_dtree[n].fc.code = (USH)bi_reverse(n, 5);
	}

	// Initialize the first block of the first file
	init_block(state);
}

/********************** pqremove() ********************
 * Removes the smallest element from the heap and recreates
 * the heap with one less element. Updates heap and heap_len.
 */

#define pqremove(tree, top) \
{\
	top = state->ts.heap[SMALLEST]; \
	state->ts.heap[SMALLEST] = state->ts.heap[state->ts.heap_len--]; \
	pqdownheap(state, tree, SMALLEST); \
}

/*********************** smaller() *******************
 * Compares two subtrees, using the tree depth as tie
 * breaker when the subtrees have equal frequency. This
 * minimizes the worst case length.
 */

#define smaller(tree, n, m) \
	(tree[n].fc.freq < tree[m].fc.freq || \
	(tree[n].fc.freq == tree[m].fc.freq && state->ts.depth[n] <= state->ts.depth[m]))

/********************** pdownheap() ********************
 * Restores the heap property by moving down the tree
 * starting at node k, exchanging a node with the smallest
 * of its two sons if necessary, stopping when the heap
 * property is re-established (each father smaller than its
 * two sons).
 */

void pqdownheap(register TSTATE *state, CT_DATA *tree, int k)
{
	int		v = state->ts.heap[k];
	int		j = k << 1;  // left son of k
	int		htemp;       // required because of bug in SASC compiler

	while (j <= state->ts.heap_len)
	{
		// Set j to the smallest of the two sons:
		if (j < state->ts.heap_len && smaller(tree, state->ts.heap[j+1], state->ts.heap[j])) j++;

		// Exit if v is smaller than both sons
		htemp = state->ts.heap[j];
		if (smaller(tree, v, htemp)) break;

		// Exchange v with the smallest son
		state->ts.heap[k] = htemp;
		k = j;

		// And continue down the tree, setting j to the left son of k
		j <<= 1;
	}
	state->ts.heap[k] = v;
}

/********************** gen_bitlen() ********************
 * Computes the optimal bit lengths for a tree and updates
 * the total bit length for the current block.
 *
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
void gen_bitlen(register TSTATE *state, TREE_DESC *desc)
{
	CT_DATA		*tree		= desc->dyn_tree;
	const int	*extra		= desc->extra_bits;
	int			base		= desc->extra_base;
	int			max_code	= desc->max_code;
	int			max_length	= desc->max_length;
	CT_DATA		*stree		= desc->static_tree;
	int			h;              // heap index
	int			n, m;           // iterate over the tree elements
	int			bits;           // bit length
	int			xbits;          // extra bits
	USH			f;              // frequency
	int			overflow = 0;   // number of elements with bit length too large

	for (bits = 0; bits <= MAX_BITS; bits++) state->ts.bl_count[bits] = 0;

	// In a first pass, compute the optimal bit lengths (which may
	// overflow in the case of the bit length tree).
	tree[state->ts.heap[state->ts.heap_max]].dl.len = 0; // root of the heap

	for (h = state->ts.heap_max + 1; h < HEAP_SIZE; h++)
	{
		n = state->ts.heap[h];
		bits = tree[tree[n].dl.dad].dl.len + 1;
		if (bits > max_length)
		{
			bits = max_length;
			++overflow;
		}
		tree[n].dl.len = (USH)bits;
		// We overwrite tree[n].dl.dad which is no longer needed

		if (n > max_code) continue; // not a leaf node

		state->ts.bl_count[bits]++;
		xbits = 0;
		if (n >= base) xbits = extra[n-base];
		f = tree[n].fc.freq;
		state->ts.opt_len += (ULG)f * (bits + xbits);
		if (stree) state->ts.static_len += (ULG)f * (stree[n].dl.len + xbits);
	}
	if (overflow == 0) return;

/*#ifndef NDEBUG
	Trace("\nbit length overflow\n");
#endif*/
	// This happens for example on obj2 and pic of the Calgary corpus

	// Find the first bit length which could increase:
	do
	{
		bits = max_length - 1;
		while (state->ts.bl_count[bits] == 0) bits--;
		state->ts.bl_count[bits]--;           // move one leaf down the tree
		state->ts.bl_count[bits+1] += (USH)2; // move one overflow item as its brother
		state->ts.bl_count[max_length]--;
		// The brother of the overflow item also moves one step up,
		// but this does not affect bl_count[max_length]
		overflow -= 2;
	} while (overflow > 0);

	// Now recompute all bit lengths, scanning in increasing frequency.
	// h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
	// lengths instead of fixing only the wrong ones. This idea is taken
	// from 'ar' written by Haruhiko Okumura.)
	for (bits = max_length; bits != 0; bits--)
	{
		n = state->ts.bl_count[bits];
		while (n != 0)
		{
			m = state->ts.heap[--h];
			if (m > max_code) continue;
			if (tree[m].dl.len != (USH)bits)
			{
/*#ifndef NDEBUG
				Trace("code %d bits %d->%d\n", m, tree[m].dl.len, bits);
#endif*/
				state->ts.opt_len += ((long)bits-(long)tree[m].dl.len)*(long)tree[m].fc.freq;
				tree[m].dl.len = (USH)bits;
			}
			--n;
		}
	}
}

/************************ gen_codes() *******************
 * Generates the codes for a given tree and bit counts
 * (which need not be optimal).
 *
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 *
 * OUT assertion: the field code is set for all tree elements of non
 * zero code length.
 */
void gen_codes(register TSTATE *state, CT_DATA *tree, int max_code)
{
	USH				next_code[MAX_BITS+1];	// next code value for each bit length
	{
	register DWORD	bits;
	USH				code;

	// The distribution counts are first used to generate the code values
	// without bit reversal
	code = 0;
	for (bits = 1; bits <= MAX_BITS; bits++)
		next_code[bits] = code = (USH)((code + state->ts.bl_count[bits - 1]) << 1);

	// Check that the bit counts in bl_count are consistent. The last code
	// must be all ones
/*#ifndef NDEBUG
	Assert(state, code + state->ts.bl_count[MAX_BITS]-1 == (1<< ((USH) MAX_BITS)) - 1, "inconsistent bit counts");
	Trace("\ngen_codes: max_code %d ", max_code);
#endif*/
	}

	{
	int		n;

	for (n = 0;  n <= max_code; n++)
	{
		register DWORD len;

		// Reverse the bits
		if ((len = tree[n].dl.len)) tree[n].fc.code = (USH)bi_reverse(next_code[len]++, (unsigned char)len);
	}
	}
}

/************************** build_tree() **********************
 * Constructs one Huffman tree and assigns the code bit strings
 * and lengths. Updates the total bit length for the current block.
 *
 * RETURNS: The fields len and code are set to the optimal bit length
 * and corresponding code. The length opt_len is updated; static_len is
 * also updated if stree is not null. The field max_code is set.
 *
 * NOTE: Before calling here, the "freq" field of all tree
 * elements must be set to an appropriate value.
 */

void build_tree(register TSTATE *state, TREE_DESC *desc)
{
	CT_DATA		*tree = desc->dyn_tree;
	CT_DATA		*stree = desc->static_tree;
	int			elems = desc->elems;
	int			n, m;				// iterate over heap elements
	int			max_code = -1;		// largest code with non zero frequency
	int			node = elems;		// next internal node of the tree

	// Construct the initial heap, with least frequent element in
	// heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
	// heap[0] is not used.
	state->ts.heap_len = 0;
	state->ts.heap_max = HEAP_SIZE;

	for (n = 0; n < elems; n++)
	{
		if (tree[n].fc.freq)
		{
			state->ts.heap[++state->ts.heap_len] = max_code = n;
			state->ts.depth[n] = 0;
		}
		else
			tree[n].dl.len = 0;
	}

	// The pkzip format requires that at least one distance code exists,
	// and that at least one bit should be sent even if there is only one
	// possible code. So to avoid special checks later on we force at least
	// two codes of non zero frequency
	while (state->ts.heap_len < 2)
	{
		int newcp = state->ts.heap[++state->ts.heap_len] = (max_code < 2 ? ++max_code : 0);
		tree[newcp].fc.freq = 1;
		state->ts.depth[newcp] = 0;
		state->ts.opt_len--;
		if (stree) state->ts.static_len -= stree[newcp].dl.len;
		// new is 0 or 1 so it does not have extra bits
	}
	desc->max_code = max_code;

	// The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
	// establish sub-heaps of increasing lengths:
	for (n = state->ts.heap_len/2; n >= 1; n--) pqdownheap(state,tree, n);

	// Construct the Huffman tree by repeatedly combining the least two
	// frequent nodes
	do
	{
		pqremove(tree, n);						// n = node of least frequency
		m = state->ts.heap[SMALLEST];			// m = node of next least frequency

		state->ts.heap[--state->ts.heap_max] = n; // keep the nodes sorted by frequency
		state->ts.heap[--state->ts.heap_max] = m;

		// Create a new node father of n and m
		tree[node].fc.freq = (USH)(tree[n].fc.freq + tree[m].fc.freq);
		state->ts.depth[node] = (UCH) (Max(state->ts.depth[n], state->ts.depth[m]) + 1);
		tree[n].dl.dad = tree[m].dl.dad = (USH)node;
		// and insert the new node in the heap
		state->ts.heap[SMALLEST] = node++;
		pqdownheap(state,tree, SMALLEST);

	} while (state->ts.heap_len >= 2);

	state->ts.heap[--state->ts.heap_max] = state->ts.heap[SMALLEST];

	// At this point, the fields freq and dad are set. We can now
	// generate the bit lengths.
	gen_bitlen(state, desc);

	// The field len is now set, we can generate the bit codes
	gen_codes(state, tree, max_code);
}

/************************** scan_tree() **********************
 * Scans a literal or distance tree to determine the frequencies
 * of the codes in the bit length tree. Updates opt_len to take
 * into account the repeat counts. (The contribution of the bit
 * length codes will be added later during the construction of
 * bl_tree.)
 */

void scan_tree(register TSTATE *state, CT_DATA *tree, int max_code)
{
	int				n;			// iterates over all tree elements
	int				prevlen;	// last emitted length
	int				curlen;		// length of current code
	int				nextlen;	// length of next code
	register BYTE	count;		// repeat count of the current code
	register BYTE	max_count;	// max repeat count
	register BYTE	min_count;	// min repeat count

	count = 0;
	nextlen = tree[0].dl.len;
	prevlen = -1;

	max_count = 7;
	min_count = 4;
	if (!nextlen)
	{
		max_count = 138;
		min_count = 3;
	}

	// Set to -1 as a sort of "guard marker" that shouldn't be crossed
	tree[max_code + 1].dl.len = (USH)-1;

	for (n = 0; n <= max_code; n++)
	{
		curlen = nextlen;
		nextlen = tree[n+1].dl.len;
		if (++count < max_count && curlen == nextlen) continue;
		if (count < min_count)
			state->ts.bl_tree[curlen].fc.freq = (USH)(state->ts.bl_tree[curlen].fc.freq + count);
        else if (curlen)
		{
			if (curlen != prevlen) ++state->ts.bl_tree[curlen].fc.freq;
			++state->ts.bl_tree[REP_3_6].fc.freq;
		}
		else if (count <= 10)
			++state->ts.bl_tree[REPZ_3_10].fc.freq;
		else
			++state->ts.bl_tree[REPZ_11_138].fc.freq;

		count = 0;
		prevlen = curlen;
		if (!nextlen)
		{
			max_count = 138;
			min_count = 3;
		}
		else if (curlen == nextlen)
		{
			max_count = 6;
			min_count = 3;
		}
		else
		{
			max_count = 7;
			min_count = 4;
		}
	}
}

/*********************** send_tree() ************************
 * Sends a literal or distance tree in compressed form, using
 * the codes in bl_tree().
 */

BOOL send_tree(register TSTATE *state, CT_DATA *tree, int max_code)
{
	int				n;			// iterates over all tree elements
	int				prevlen;	// last emitted length
	int				curlen;		// length of current code
	int				nextlen;	// length of next code
	register BYTE	count;		// repeat count of the current code
	register BYTE	max_count;	// max repeat count
	register BYTE	min_count;	// min repeat count

	count = 0;
	nextlen = tree[0].dl.len;
	prevlen = -1;

	max_count = 7;
	min_count = 4;
	if (!nextlen)
	{
		max_count = 138;
		min_count = 3;
	}

	for (n = 0; n <= max_code; n++)
	{
		curlen = nextlen;
		nextlen = tree[n+1].dl.len;
		if (++count < max_count && curlen == nextlen) continue;

		if (count < min_count)
		{
			do
			{
				if (!send_code(state, curlen, state->ts.bl_tree)) goto out;
			} while (--count);
		}
		else if (curlen)
		{
			if (curlen != prevlen)
			{
				if (!send_code(state, curlen, state->ts.bl_tree)) goto out;
				--count;
			}
/*#ifndef NDEBUG
			Assert(state, count >= 3 && count <= 6, " 3_6?");
#endif*/
			if (!send_code(state, REP_3_6, state->ts.bl_tree) || ! send_bits(state, count - 3, 2)) goto out;

		}
		else if (count <= 10)
		{
			if (!send_code(state, REPZ_3_10, state->ts.bl_tree) || !send_bits(state, count - 3, 3)) goto out;
		}
		else
		{
			if (!send_code(state, REPZ_11_138, state->ts.bl_tree) || !send_bits(state, count - 11, 7))
out:			return(0);
		}

		count = 0;
		prevlen = curlen;
		if (!nextlen)
		{
			max_count = 138;
			min_count = 3;
		}
		else if (curlen == nextlen)
		{
			max_count = 6;
			min_count = 3;
		}
		else
		{
			max_count = 7;
			min_count = 4;
		}
	}

	return(1);
}

/************************* build_bl_tree() *******************
 * Constructs the Huffman tree for the bit lengths and returns
 * the index in BL_order[] of the last bit length code to send.
 */

int build_bl_tree(register TSTATE *state)
{
	int		max_blindex;		// index of last bit length code of non zero freq

	// Determine the bit length frequencies for literal and distance trees
	scan_tree(state, state->ts.dyn_ltree, state->ts.l_desc.max_code);
	scan_tree(state, state->ts.dyn_dtree, state->ts.d_desc.max_code);

	// Build the bit length tree:
	build_tree(state,(TREE_DESC *)(&state->ts.bl_desc));
	// opt_len now includes the length of the tree representations, except
	// the lengths of the bit lengths codes and the 5+5+4 bits for the counts.

	// Determine the number of bit length codes to send. The pkzip format
	// requires that at least 4 bit length codes be sent. (appnote.txt says
	// 3 but the actual value used is 4.)
	for (max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex--)
	{
		if (state->ts.bl_tree[BL_order[max_blindex]].dl.len != 0) break;
	}

	// Update opt_len to include the bit length tree and counts
	state->ts.opt_len += 3*(max_blindex+1) + 5+5+4;
/*#ifndef NDEBUG
	Trace("\ndyn trees: dyn %ld, stat %ld", state->ts.opt_len, state->ts.static_len);
#endif*/
	return(max_blindex);
}





/************************* send_all_trees() *******************
 * Sends the header for a block using dynamic Huffman trees:
 * the counts, the lengths of the bit length codes, the literal
 * tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
BOOL send_all_trees(register TSTATE *state, int lcodes, int dcodes, int blcodes)
{
	int		rank;	// index into BL_order[]

/*#ifndef NDEBUG
	Assert(state,lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
	Assert(state,lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES, "too many codes");
	Trace("\nbl counts: ");
#endif*/

	if (send_bits(state, lcodes - 257, 5) &&	// not +255 as stated in appnote.txt 1.93a or -256 in 2.04c
		send_bits(state, dcodes - 1, 5) &&
		send_bits(state, blcodes - 4,  4))		// not -3 as stated in appnote.txt
	{
		for (rank = 0; rank < blcodes; rank++)
		{
/*#ifndef NDEBUG
			Trace("\nbl code %2d ", BL_order[rank]);
#endif*/
			if (!send_bits(state,state->ts.bl_tree[BL_order[rank]].dl.len, 3)) goto out;
		}
/*#ifndef NDEBUG
		Trace("\nbl tree: sent %ld", state->bs.bits_sent);
#endif*/

		// Send the literal tree
		if (send_tree(state, (CT_DATA *)state->ts.dyn_ltree, lcodes - 1))
		{
/*#ifndef NDEBUG
			Trace("\nlit tree: sent %ld", state->bs.bits_sent);
#endif*/
			// Send the distance tree
			if (send_tree(state, state->ts.dyn_dtree, dcodes - 1))
			{
/*#ifndef NDEBUG
				Trace("\ndist tree: sent %ld", state->bs.bits_sent);
#endif*/
				return(1);
			}
		}
	}
out:
	return(0);
}





/********************* set_file_type() ********************
 * Sets the file type to ASCII or BINARY, using a crude
 * approximation: binary if more than 20% of the bytes are
 * <= 6 or >= 128, ascii otherwise.
 *
 * IN assertion: the fields freq of dyn_ltree are set and
 * the total of all frequencies does not exceed 64K (to fit
 * in an int on 16 bit machines).
 */

void set_file_type(register TSTATE *state)
{
	unsigned	n;
	unsigned	ascii_freq;
	unsigned	bin_freq;

	n = ascii_freq = bin_freq = 0;

	while (n < 7)        bin_freq += state->ts.dyn_ltree[n++].fc.freq;
	while (n < 128)    ascii_freq += state->ts.dyn_ltree[n++].fc.freq;
	while (n < LITERALS) bin_freq += state->ts.dyn_ltree[n++].fc.freq;
	*state->ts.file_type = (USH)(bin_freq > (ascii_freq >> 2) ? BINARY : ASCII);
}





/************************* flush_block() ********************
 * Determines the best encoding for the current block: dynamic
 * trees, static trees or store, and outputs the encoded block
 * to the zip file.
 *
 * RETURNS: The total compressed length (in bytes) for the
 * file so far.
 */

void flush_block(register TSTATE *state, char *buf, ULG stored_len, DWORD eof)
{
	ULG		opt_lenb, static_lenb;	// opt_len and static_len in bytes
	int		max_blindex;			// index of last bit length code of non zero freq

	state->ts.flag_buf[state->ts.last_flags] = state->ts.flags; // Save the flags for the last 8 items

	// Check if the file is ascii or binary
	if (*state->ts.file_type == (USH)UNKNOWN) set_file_type(state);

	// Construct the literal and distance trees
	build_tree(state, (TREE_DESC *)(&state->ts.l_desc));
/*#ifndef NDEBUG
	Trace("\nlit data: dyn %ld, stat %ld", state->ts.opt_len, state->ts.static_len);
#endif*/

	build_tree(state,(TREE_DESC *)(&state->ts.d_desc));
/*#ifndef NDEBUG
	Trace("\ndist data: dyn %ld, stat %ld", state->ts.opt_len, state->ts.static_len);
#endif*/
	// At this point, opt_len and static_len are the total bit lengths of
	// the compressed block data, excluding the tree representations.

	// Build the bit length tree for the above two trees, and get the index
	// in BL_order[] of the last bit length code to send.
	max_blindex = build_bl_tree(state);

	// Determine the best encoding. Compute first the block length in bytes
	opt_lenb = (state->ts.opt_len+3+7)>>3;
	static_lenb = (state->ts.static_len+3+7)>>3;

/*#ifndef NDEBUG
	state->ts.input_len += stored_len;
	Trace("\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ", opt_lenb, state->ts.opt_len, static_lenb, state->ts.static_len, stored_len, state->ts.last_lit, state->ts.last_dist);
#endif*/

	if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

	if (stored_len + 4 <= opt_lenb && buf)
	{
		// two words for the lengths
		// The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
		// Otherwise we can't have processed more than WSIZE input bytes since
		// the last block flush, because compression would have been
		// successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
		// transform a block into a stored block.

		// Send block type
		send_bits(state, (STORED_BLOCK << 1) + eof, 3);
		state->ts.cmpr_bytelen += ((state->ts.cmpr_len_bits + 3 + 7) >> 3) + stored_len + 4;
		state->ts.cmpr_len_bits = 0;
		copy_block(state, buf, (unsigned)stored_len, 1); // with header
	}
	else if (static_lenb == opt_lenb)
	{
		send_bits(state, (STATIC_TREES << 1) + eof, 3);
		compress_block(state,(CT_DATA *)state->ts.static_ltree, (CT_DATA *)state->ts.static_dtree);
		state->ts.cmpr_len_bits += 3 + state->ts.static_len;
		goto upd;
	}
	else
	{
		send_bits(state, (DYN_TREES << 1) + eof, 3);
		send_all_trees(state,state->ts.l_desc.max_code + 1, state->ts.d_desc.max_code + 1, max_blindex + 1);
		compress_block(state, state->ts.dyn_ltree, state->ts.dyn_dtree);
		state->ts.cmpr_len_bits += 3 + state->ts.opt_len;
upd:	state->ts.cmpr_bytelen += state->ts.cmpr_len_bits >> 3;
		state->ts.cmpr_len_bits &= 7;
	}
/*#ifndef NDEBUG
	Assert(state,((state->ts.cmpr_bytelen << 3) + state->ts.cmpr_len_bits) == state->bs.bits_sent, "bad compressed size");
#endif*/

	if (!state->tzip->lasterr)
	{
		init_block(state);

		if (eof)
		{
			bi_windup(state);
			state->ts.cmpr_len_bits += 7;	// align on byte boundary
		}
/*#ifndef NDEBUG
		Trace("\n");
#endif*/
		// Set compressed size so far
		state->tzip->csize = state->ts.cmpr_bytelen + (state->ts.cmpr_len_bits >> 3);
	}
}




/********************* ct_tally() **********************
 * Saves the match info and tallies the frequency counts.
 *
 * RETURNS: TRUE if the current block must be flushed.
 */

unsigned char ct_tally(register TSTATE *state, int dist, int lc)
{
	state->ts.l_buf[state->ts.last_lit++] = (UCH)lc;
	if (!dist)
	{
		// lc is the unmatched char
		state->ts.dyn_ltree[lc].fc.freq++;
	}
	else
	{
		// Here, lc is the match length - MIN_MATCH
		--dist;				// dist = match distance - 1
/*#ifndef NDEBUG
		Assert(state,(USH)dist < (USH)MAX_DIST && (USH)lc <= (USH)(MAX_MATCH-MIN_MATCH) && (USH)d_code(dist) < (USH)D_CODES,  "ct_tally: bad match");
#endif*/
		state->ts.dyn_ltree[state->ts.length_code[lc] + LITERALS + 1].fc.freq++;
		state->ts.dyn_dtree[d_code(dist)].fc.freq++;

		state->ts.d_buf[state->ts.last_dist++] = (USH)dist;
		state->ts.flags |= state->ts.flag_bit;
	}
	state->ts.flag_bit <<= 1;

	// Store the flags if they fill a byte
	if ((state->ts.last_lit & 7) == 0)
	{
		state->ts.flag_buf[state->ts.last_flags++] = state->ts.flags;
		state->ts.flags = 0, state->ts.flag_bit = 1;
	}

	// Try to guess if it is profitable to stop the current block here
	if (state->level > 2 && (state->ts.last_lit & 0xfff) == 0)
	{
		DWORD	dcode;
		ULG		out_length;
		ULG		in_length;

		// Compute an upper bound for the compressed length
		out_length = (ULG)state->ts.last_lit*8L;
		in_length = (ULG)state->ds.strstart - state->ds.block_start;
		dcode = D_CODES;
		while (dcode--) out_length += (ULG)state->ts.dyn_dtree[dcode].fc.freq * (5L + Extra_dbits[dcode]);
		out_length >>= 3;

/*#ifndef NDEBUG
		Trace("\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ", state->ts.last_lit, state->ts.last_dist, in_length, out_length, 100L - out_length*100L/in_length);
#endif*/
		// Should we stop the block here?
		if (state->ts.last_dist < state->ts.last_lit/2 && out_length < in_length/2) return(1);
	}

	// NOTE: We avoid equality with LIT_BUFSIZE because of wraparound at 64K
	// on 16 bit machines and because stored blocks are restricted to
	// 64K-1 bytes
	return(state->ts.last_lit == LIT_BUFSIZE-1 || state->ts.last_dist == DIST_BUFSIZE);
}





/********************* compress_block() ********************
 * Sends the block data compressed using the given Huffman
 * trees.
 */

void compress_block(register TSTATE *state, CT_DATA *ltree, CT_DATA *dtree)
{
	unsigned	dist;		// distance of matched string
	int			lc;			// match length or unmatched char (if dist == 0)
	DWORD		lx;			// running index in l_buf
	DWORD		dx;			// running index in d_buf
	DWORD		fx;			// running index in flag_buf
	UCH			flag;		// current flags
	unsigned	code;		// the code to send
	int			extra;		// number of extra bits to send

	lx = dx = fx = flag = 0;

	if (state->ts.last_lit)
	{
		do
		{
			if ((lx & 7) == 0) flag = state->ts.flag_buf[fx++];
			lc = state->ts.l_buf[lx++];

			if ((flag & 1) == 0)
			{
				// send a literal byte
				if (!send_code(state, lc, ltree)) goto out;
			}			// bug in VC4.0 if these {} are removed!!!
			else
			{
				// Here, lc is the match length - MIN_MATCH

				code = state->ts.length_code[lc];

				// send the length code
				if (!send_code(state, code + LITERALS + 1, ltree))
out:				return;

				// send the extra length bits
				extra = Extra_lbits[code];
				if (extra)
				{
					lc -= state->ts.base_length[code];
					if (!send_bits(state,lc, extra)) goto out;
				}

				dist = state->ts.d_buf[dx++];
				// Here, dist is the match distance - 1

				// send the distance code
				code = d_code(dist);
/*#ifndef NDEBUG
				Assert(state, code < D_CODES, "bad d_code");
#endif*/
				if (!send_code(state, code, dtree)) goto out;

				// send the extra distance bits
				extra = Extra_dbits[code];
				if (extra)
				{
					dist -= state->ts.base_dist[code];
					if (!send_bits(state, dist, extra)) goto out;
				}
			} // literal or match pair ?

			flag >>= 1;

		} while (lx < state->ts.last_lit);
	}

	send_code(state, END_BLOCK, ltree);
}






/*********************** send_bits() **********************
 * Sends a value on a given number of bits.
 *
 * IN assertion: length <= 16 and value fits in length bits.
 */

BOOL send_bits(register TSTATE *state, int value, int length)
{
/*#ifndef NDEBUG
	Assert(state, length > 0 && length <= 15, "invalid length");
	state->bs.bits_sent += (ULG)length;
#endif*/
	// If not enough room in bi_buf, use (bi_valid) bits from bi_buf and
	// (ZIP_BUF_SIZE - bi_valid) bits from value to flush the filled bi_buf,
	// then fill in the rest of (value), leaving (length - (ZIP_BUF_SIZE-bi_valid))
	// unused bits in bi_buf.
	state->bs.bi_buf |= (value << state->bs.bi_valid);
	state->bs.bi_valid += length;
	if (state->bs.bi_valid > ZIP_BUF_SIZE)
	{
		// If we've filled the output buffer, flush it
		if (state->bs.out_offset + 1 >= state->bs.out_size)
		{
			writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
			if (state->tzip->lasterr) return(0);
			state->bs.out_offset = 0;
		}

		// Store the short in Intel-order
		state->bs.out_buf[state->bs.out_offset++] = (char)((state->bs.bi_buf) & 0xff);
		state->bs.out_buf[state->bs.out_offset++] = (char)((USH)(state->bs.bi_buf) >> 8);

		state->bs.bi_valid -= ZIP_BUF_SIZE;
		state->bs.bi_buf = (unsigned)value >> (length - state->bs.bi_valid);
	}

	return(1);
}





/********************** bi_reverse() *********************
 * Reverses the first "len" bits of a code.
 *
 * code =	The number whose bits are to be shifted.
 * len =	The number of bits to reverse (1 to 15).
 */

unsigned bi_reverse(register unsigned code, register unsigned char len)
{
	register unsigned res;

	res = 0;
	goto rev;
	do
	{
		res <<= 1;
		code >>= 1;
rev:	res |= code & 1;
	} while (--len);
	return(res);
}





/*********************** bi_windup() ********************
 * Writes out any remaining bits in an incomplete byte.
 */

void bi_windup(register TSTATE *state)
{
	if (state->bs.bi_valid > 8)
	{
		// If we've filled the output buffer, flush it
		if (state->bs.out_offset + 1 >= state->bs.out_size)
		{
			writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
			state->bs.out_offset = 0;
		}

		// Store the short in Intel-order
		state->bs.out_buf[state->bs.out_offset++] = (char)((state->bs.bi_buf) & 0xff);
		state->bs.out_buf[state->bs.out_offset++] = (char)((USH)(state->bs.bi_buf) >> 8);
	}
	else if (state->bs.bi_valid > 0)
	{
		// If we've filled the output buffer, flush it
		if (state->bs.out_offset >= state->bs.out_size)
		{
			writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
			state->bs.out_offset = 0;
		}

		// Store the byte
		state->bs.out_buf[state->bs.out_offset++] = (char)state->bs.bi_buf;
	}

	// Flush the buffer to the ZIP archive
	writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
	state->bs.bi_buf = state->bs.out_offset = state->bs.bi_valid = 0;
/*#ifndef NDEBUG
	state->bs.bits_sent = (state->bs.bits_sent+7) & ~7;
#endif*/
}





/************************ copy_block() ********************
 * Copies a stored block to the zip file, storing first the
 * length and its one's complement if requested.
 */

void copy_block(register TSTATE *state, char *block, DWORD len, DWORD header)
{
	// Align on a byte boundary by writing out any previous, uncompleted bytes
	bi_windup(state);

	// If a header, write the length and one's complement
	if (header)
	{
		// If we don't have room for 2 shorts, flush the output buffer
		if (state->bs.out_offset + 3 >= state->bs.out_size)
		{
			writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
			state->bs.out_offset = 0;
		}

		// Store the short in Intel-order
		state->bs.out_buf[state->bs.out_offset++] = (char)((len) & 0xff);
		state->bs.out_buf[state->bs.out_offset++] = (char)((USH)(len) >> 8);

		// Store one's complement
		state->bs.out_buf[state->bs.out_offset++] = (char)((~len) & 0xff);
		state->bs.out_buf[state->bs.out_offset++] = (char)((USH)(~len) >> 8);

		// Flush the 2 shorts now (because we're going to flush the block
		// which is in a different memory buffer)
		writeDestination(state->tzip, state->bs.out_buf, state->bs.out_offset);
		state->bs.out_offset = 0;


/*#ifndef NDEBUG
		state->bs.bits_sent += (2*16);
#endif*/
	}

	// Write out the block
	writeDestination(state->tzip, block, len);

/*#ifndef NDEBUG
	state->bs.bits_sent += (ULG)len<<3;
#endif*/
}






/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)





/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(s, match_head) \
   (UPDATE_HASH(state->ds.ins_h, state->ds.window[(s) + (MIN_MATCH-1)]), \
    state->ds.prev[(s) & WMASK] = match_head = state->ds.head[state->ds.ins_h], \
    state->ds.head[state->ds.ins_h] = (s))





/************************ lm_init() ***********************
 * Initializes the "longest match" routines in preparation of
 * writing a new zip file.
 *
 * NOTE: state->ds.window_size is > 0 if the source file in
 * its entirety is already read into the state->ds.window[]
 * array, 0 otherwise. In the first case, window_size is
 * sufficient to contain the whole input file plus MIN_LOOKAHEAD
 * bytes (to avoid referencing memory beyond the end
 * of window[] when looking for matches towards the end).
 */

void lm_init(register TSTATE *state, DWORD pack_level, USH *flags)
{
	register unsigned j;

	// Do not slide the window if the whole input is already in memory (window_size > 0)
//	state->ds.sliding = 0;
//	if (!state->ds.window_size)
//	{
		state->ds.sliding = 1;
		state->ds.window_size = (ULG)(2L * WSIZE);
//	}

	// Initialize the hash table (avoiding 64K overflow for 16 bit systems).
	// prev[] will be initialized on the fly
	state->ds.head[HASH_SIZE - 1] = 0;
	ZeroMemory(state->ds.head, (HASH_SIZE - 1) * sizeof(*state->ds.head));

	// Set the default configuration parameters:
	state->ds.max_lazy_match = ConfigurationTable[pack_level].max_lazy;
	state->ds.good_match = ConfigurationTable[pack_level].good_length;
	state->ds.nice_match = ConfigurationTable[pack_level].nice_length;
	state->ds.max_chain_length = ConfigurationTable[pack_level].max_chain;
	if (pack_level <= 2) *flags |= FAST;
	else if (pack_level >= 8) *flags |= SLOW;

	// ??? reduce max_chain_length for binary files


	// Fill the state->ds.window[] buffer with source bytes
	if (!(state->ds.lookahead = readFromSource(state->tzip, state->ds.window, WSIZE * 2))) return;

	// At start of input buffer, and haven't reached the end of the source yet
	state->ds.strstart = state->ds.block_start = state->ds.eofile = 0;

	// Make sure that we always have enough lookahead
	if (state->ds.lookahead < MIN_LOOKAHEAD) fill_window(state);

	// If lookahead < MIN_MATCH, ins_h is garbage, but this is
	// not important since only literal bytes will be emitted
	state->ds.ins_h = 0;
	for (j=0; j < MIN_MATCH - 1; j++) UPDATE_HASH(state->ds.ins_h, state->ds.window[j]);
}





/********************** longest_match() *******************
 * Sets match_start to the longest match starting at the
 * given string and return its length. Any match shorter or
 * equal to prev_length ia discarded, in which case the
 * result is equal to prev_length and match_start is
 * garbage.
 *
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */

int longest_match(register TSTATE *state, unsigned cur_match)
{
	unsigned chain_length = state->ds.max_chain_length;   // max hash chain length
	register UCH *scan = state->ds.window + state->ds.strstart; // current string
	register UCH *match;                    // matched string
	register int len;                           // length of current match
	int best_len = state->ds.prev_length;                 // best match length so far
	unsigned limit = state->ds.strstart > (unsigned)MAX_DIST ? state->ds.strstart - (unsigned)MAX_DIST : 0;
	register UCH *strend;
	register UCH scan_end1;
	register UCH scan_end;

	// Stop when cur_match becomes <= limit. To simplify the code,
	// we prevent matches with the string of window index 0.

	// The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
	// It is easy to get rid of this optimization if necessary.
/*#ifndef NDEBUG
	Assert(state,HASH_BITS>=8 && MAX_MATCH==258,"Code too clever");
#endif*/

	strend = state->ds.window + state->ds.strstart + MAX_MATCH;
	scan_end1 = scan[best_len-1];
	scan_end = scan[best_len];

	// Do not waste too much time if we already have a good match:
	if (state->ds.prev_length >= state->ds.good_match) chain_length >>= 2;

/*#ifndef NDEBUG
	Assert(state,state->ds.strstart <= state->ds.window_size - MIN_LOOKAHEAD, "insufficient lookahead");
#endif*/

	do
	{
/*#ifndef NDEBUG
		Assert(state,cur_match < state->ds.strstart, "no future");
#endif*/
		match = state->ds.window + cur_match;

		// Skip to next match if the match length cannot increase
		// or if the match length is less than 2:
		if (match[best_len]   != scan_end  ||
			match[best_len-1] != scan_end1 ||
			*match            != *scan     ||
			*++match          != scan[1])      continue;

		// The check at best_len-1 can be removed because it will be made
		// again later. (This heuristic is not always a win.)
		// It is not necessary to compare scan[2] and match[2] since they
		// are always equal when the other bytes match, given that
		// the hash keys are equal and that HASH_BITS >= 8.
		scan += 2, match++;

		// We check for insufficient lookahead only every 8th comparison;
		// the 256th check will be made at strstart+258.
		do
		{
		} while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

/*#ifndef NDEBUG
		Assert(state,scan <= state->ds.window+(unsigned)(state->ds.window_size - 1), "wild scan");
#endif*/

		len = MAX_MATCH - (int)(strend - scan);
		scan = strend - MAX_MATCH;

		if (len > best_len)
		{
			state->ds.match_start = cur_match;
			best_len = len;
			if (len >= state->ds.nice_match) break;
			scan_end1  = scan[best_len-1];
			scan_end   = scan[best_len];
		}
	} while ((cur_match = state->ds.prev[cur_match & WMASK]) > limit && --chain_length != 0);

	return best_len;
}





#define check_match(state,start, match, length)





/*********************** fill_window() **********************
 * Fills the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of
 * input file.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or eofile is set; file reads are
 *    performed for at least two bytes (required for the translate_eol option).
 */

void fill_window(register TSTATE *state)
{
	register unsigned	n, m;
	unsigned			more;	// Amount of free space at the end of the window

	do
	{
		more = (unsigned)(state->ds.window_size - (ULG)state->ds.lookahead - (ULG)state->ds.strstart);

		// If the window is almost full and there is insufficient lookahead,
		// move the upper half to the lower one to make room in the upper half.
		if (more == (unsigned)-1)
		{
			// Very unlikely, but possible on 16 bit machine if strstart == 0
			// and lookahead == 1 (input done one byte at time)
			--more;
		}

		// For a memory-block source, the whole source is already in memory so
		// we don't need to perform sliding. We must however call readFromSource() in
		// order to compute the crc, update lookahead, and possibly set eofile
		else if (state->ds.strstart >= WSIZE + MAX_DIST && state->ds.sliding)
		{
			// By the IN assertion, the window is not empty so we can't confuse
			// more == 0 with more == 64K on a 16 bit machine
			CopyMemory(state->ds.window, (char *)state->ds.window + WSIZE, WSIZE);
			state->ds.match_start -= WSIZE;
			state->ds.strstart -= WSIZE;		// we now have strstart >= MAX_DIST

			state->ds.block_start -= WSIZE;

			for (n = 0; n < HASH_SIZE; n++)
			{
				m = state->ds.head[n];
				state->ds.head[n] = (unsigned)(m >= WSIZE ? m-WSIZE : 0);
			}
			for (n = 0; n < WSIZE; n++)
			{
				m = state->ds.prev[n];
				state->ds.prev[n] = (unsigned)(m >= WSIZE ? m-WSIZE : 0);
				// If n is not on any hash chain, prev[n] is garbage but
				// its value will never be used.
			}
			more += WSIZE;
		}

		if (state->ds.eofile) break;

		// If there was no sliding:
		//    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
		//    more == window_size - lookahead - strstart
		// => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
		// => more >= window_size - 2*WSIZE + 2
		// In the memory-block case (not yet supported in gzip),
		//   window_size == input_size + MIN_LOOKAHEAD  &&
		//   strstart + lookahead <= input_size => more >= MIN_LOOKAHEAD.
		// Otherwise, window_size == 2*WSIZE so more >= 2.
		// If there was sliding, more >= WSIZE. So in all cases, more >= 2.
/*#ifndef NDEBUG
		Assert(state, more >= 2, "more < 2");
#endif*/
		if (!(n = readFromSource(state->tzip, state->ds.window + state->ds.strstart + state->ds.lookahead, more)))
		{
			state->ds.eofile = 1;
			break;
		}

		state->ds.lookahead += n;
	} while (state->ds.lookahead < MIN_LOOKAHEAD);
}






/*************************** deflate_fast() *******************
 * Processes a new input file and return its compressed length.
 * Does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or
 * for short matches. It is used only for the fast compression
 * options.
 */

#if 0

void deflate_fast(register TSTATE *state)
{
	unsigned		hash_head;		// head of the hash chain
	unsigned		match_length;	// length of best match
	unsigned char	flush;			// set if current block must be flushed

	hash_head = match_length = 0;
	state->ds.prev_length = MIN_MATCH - 1;
	while (state->ds.lookahead)
	{
		// Insert the string window[strstart .. strstart+2] in the
		// dictionary, and set hash_head to the head of the hash chain:
		if (state->ds.lookahead >= MIN_MATCH)
		INSERT_STRING(state->ds.strstart, hash_head);

		// Find the longest match, discarding those <= prev_length.
		// At this point we have always match_length < MIN_MATCH
		if (hash_head != 0 && state->ds.strstart - hash_head <= MAX_DIST)
		{
			// To simplify the code, we prevent matches with the string
			// of window index 0 (in particular we have to avoid a match
			// of the string with itself at the start of the input file).

			// Do not look for matches beyond the end of the input.
			// This is necessary to make deflate deterministic.
			if ((unsigned)state->ds.nice_match > state->ds.lookahead) state->ds.nice_match = (int)state->ds.lookahead;
			match_length = longest_match (state,hash_head);
			// longest_match() sets match_start
			if (match_length > state->ds.lookahead) match_length = state->ds.lookahead;
		}
		if (match_length >= MIN_MATCH)
		{
			check_match(state,state->ds.strstart, state->ds.match_start, match_length);

			flush = ct_tally(state,state->ds.strstart-state->ds.match_start, match_length - MIN_MATCH);

			state->ds.lookahead -= match_length;

			// Insert new strings in the hash table only if the match length
			// is not too large. This saves time but degrades compression.
			if (match_length <= state->ds.max_lazy_match && state->ds.lookahead >= MIN_MATCH)
			{
				--match_length;		// string at strstart already in hash table
				do
				{
					++state->ds.strstart;
					INSERT_STRING(state->ds.strstart, hash_head);
					// strstart never exceeds WSIZE-MAX_MATCH, so there are
					// always MIN_MATCH bytes ahead.
				} while (--match_length);
				++state->ds.strstart;
			}
			else
			{
				state->ds.strstart += match_length;
				match_length = 0;
				state->ds.ins_h = state->ds.window[state->ds.strstart];
				UPDATE_HASH(state->ds.ins_h, state->ds.window[state->ds.strstart + 1]);
/*#ifndef NDEBUG
				Assert(state,MIN_MATCH==3,"Call UPDATE_HASH() MIN_MATCH-3 more times");
#endif*/
			}
		}
		else
		{
			// No match, output a literal byte
			flush = ct_tally (state,0, state->ds.window[state->ds.strstart]);
			--state->ds.lookahead;
			++state->ds.strstart;
		}

		if (flush)
		{
			flush_block(state, state->ds.block_start >= 0 ? &state->ds.window[(unsigned)state->ds.block_start] :
						0, (long)state->ds.strstart - state->ds.block_start, 0)
			if (state->tzip->lasterr) goto bad;

			state->ds.block_start = state->ds.strstart;
		}

		// Make sure that we always have enough lookahead, except
		// at the end of the input file. We need MAX_MATCH bytes
		// for the next match, plus MIN_MATCH bytes to insert the
		// string following the next match
		if (state->ds.lookahead < MIN_LOOKAHEAD)
		{
			fill_window(state);
			if (state->tzip->lasterr)
bad:			return(state->tzip->lasterr);
		}
	}

	// EOF
	flush_block(state, state->ds.block_start >= 0 ? &state->ds.window[(unsigned)state->ds.block_start] :
				0, (long)state->ds.strstart - state->ds.block_start, 1));
}

#endif




/************************* deflate() ***********************
 * Same as above, but achieves better compression. We use a
 * lazy evaluation for matches: a match is finally adopted
 * only if there is no better match at the next window position.
 */

void deflate(register TSTATE *state)
{
	unsigned			hash_head;				// head of hash chain
	unsigned			prev_match;				// previous match
	unsigned char		flush;					// set if current block must be flushed
	unsigned char		match_available;		// set if previous match exists
	register unsigned	match_length;			// length of best match

	hash_head = match_available = 0;
	match_length = MIN_MATCH - 1;

	// Process the input bloc.
	while (state->ds.lookahead)
	{
		// Insert the string window[strstart .. strstart+2] in the
		// dictionary, and set hash_head to the head of the hash chain:
		if (state->ds.lookahead >= MIN_MATCH)
		INSERT_STRING(state->ds.strstart, hash_head);

		// Find the longest match, discarding those <= prev_length.
		state->ds.prev_length = match_length, prev_match = state->ds.match_start;
		match_length = MIN_MATCH-1;

		if (hash_head != 0 && state->ds.prev_length < state->ds.max_lazy_match && state->ds.strstart - hash_head <= MAX_DIST)
		{
			// To simplify the code, we prevent matches with the string
			// of window index 0 (in particular we have to avoid a match
			// of the string with itself at the start of the input file).

			// Do not look for matches beyond the end of the source.
			// This is necessary to make deflate deterministic
			if ((unsigned)state->ds.nice_match > state->ds.lookahead) state->ds.nice_match = (int)state->ds.lookahead;
			match_length = longest_match(state, hash_head);
			// longest_match() sets match_start
			if (match_length > state->ds.lookahead) match_length = state->ds.lookahead;

			// Ignore a length 3 match if it is too distant
			if (match_length == MIN_MATCH && state->ds.strstart - state->ds.match_start > TOO_FAR)
			{
				// If prev_match is also MIN_MATCH, match_start is garbage
				// but we will ignore the current match anyway
				match_length = MIN_MATCH-1;
			}
		}

		// If there was a match at the previous step and the current
		// match is not better, output the previous match
		if (state->ds.prev_length >= MIN_MATCH && match_length <= state->ds.prev_length)
		{
			unsigned	max_insert;

			max_insert = state->ds.strstart + state->ds.lookahead - MIN_MATCH;
			check_match(state,state->ds.strstart - 1, prev_match, state->ds.prev_length);
			flush = ct_tally(state,state->ds.strstart - 1 - prev_match, state->ds.prev_length - MIN_MATCH);

			// Insert in hash table all strings up to the end of the match.
			// strstart-1 and strstart are already inserted
			state->ds.lookahead -= state->ds.prev_length - 1;
			state->ds.prev_length -= 2;
			do
			{
				if (++state->ds.strstart <= max_insert)
				{
					INSERT_STRING(state->ds.strstart, hash_head);
					// strstart never exceeds WSIZE-MAX_MATCH, so there are always MIN_MATCH bytes ahead
				}
			} while (--state->ds.prev_length);
			++state->ds.strstart;
			match_available = 0;
			match_length = MIN_MATCH - 1;

			if (flush)
			{
				flush_block(state, state->ds.block_start >= 0 ? &state->ds.window[(unsigned)state->ds.block_start] :
							0, (long)state->ds.strstart - state->ds.block_start, 0);
				if (state->tzip->lasterr) goto bad;
				state->ds.block_start = state->ds.strstart;
			}
		}
		else if (match_available)
		{
			// If there was no match at the previous position, output a
			// single literal. If there was a match but the current match
			// is longer, truncate the previous match to a single literal
			if (ct_tally(state, 0, state->ds.window[state->ds.strstart - 1]))
			{
				flush_block(state, state->ds.block_start >= 0 ? &state->ds.window[(unsigned)state->ds.block_start] :
							0, (long)state->ds.strstart - state->ds.block_start, 0);
				if (state->tzip->lasterr) goto bad;
				state->ds.block_start = state->ds.strstart;
			}

			++state->ds.strstart;
			--state->ds.lookahead;
		}
		else
		{
			// There is no previous match to compare with, wait for the next step to decide
			match_available = 1;
			++state->ds.strstart;
			--state->ds.lookahead;
		}
//		Assert(state,strstart <= isize && lookahead <= isize, "a bit too far");

		// Make sure that we always have enough lookahead, except
		// at the end of the input file. We need MAX_MATCH bytes
		// for the next match, plus MIN_MATCH bytes to insert the
		// string following the next match.
		if (state->ds.lookahead < MIN_LOOKAHEAD)
		{
			fill_window(state);
			if (state->tzip->lasterr)
bad:			return;
		}
	}

	if (match_available) ct_tally(state, 0, state->ds.window[state->ds.strstart - 1]);

	// EOF
	flush_block(state, state->ds.block_start >= 0 ? &state->ds.window[(unsigned)state->ds.block_start] :
				0, (long)state->ds.strstart - state->ds.block_start, 1);

}











// ========================== Headers ========================

// Writes an extended local header to destination zip. Returns a ZE_ code

void putextended(TZIPFILEINFO *z, TZIP *tzip)
{
	writeDestShort(tzip, EXTLOCSIG);
	writeDestShort(tzip, EXTLOCSIG >> 16);
	writeDestShort(tzip, z->crc);
	writeDestShort(tzip, z->crc >> 16);
	writeDestShort(tzip, z->siz);
	writeDestShort(tzip, z->siz >> 16);
	writeDestShort(tzip, z->len);
	writeDestShort(tzip, z->len >> 16);
}

void putpartial(TZIPFILEINFO *z, TZIP *tzip)
{
	writeDestShort(tzip, z->tim);
	writeDestShort(tzip, z->tim >> 16);
	writeDestShort(tzip, z->crc);
	writeDestShort(tzip, z->crc >> 16);
	writeDestShort(tzip, z->siz);
	writeDestShort(tzip, z->siz >> 16);
	writeDestShort(tzip, z->len);
	writeDestShort(tzip, z->len >> 16);
	writeDestShort(tzip, z->nam);
}

// Writes a local header to destination zip. Return a ZE_ error code.

void putlocal(TZIPFILEINFO *z, TZIP *tzip)
{
	// GZIP format?
	if (tzip->flags & TZIP_OPTION_GZIP)
	{
		writeDestShort(tzip, 0x8B1F);
		writeDestShort(tzip, 0x0808);
		writeDestShort(tzip, z->tim);
		writeDestShort(tzip, z->tim >> 16);
		writeDestShort(tzip, 0xFF02);
		writeDestination(tzip, z->iname, z->nam + 1);
	}

	// PkZip format
	else
	{
		writeDestShort(tzip, LOCSIG);
		writeDestShort(tzip, LOCSIG >> 16);
		writeDestShort(tzip, (USH)20);		// Needs PKUNZIP 2.0 to unzip it
		writeDestShort(tzip, z->lflg);
		writeDestShort(tzip, z->how);
		putpartial(z, tzip);
		writeDestShort(tzip, z->ext);
		writeDestination(tzip, z->iname, z->nam);
		if (z->ext) writeDestination(tzip, z->extra, z->ext);
	}
}

/********************* addCentral() ********************
 * Writes the ZIP file's central directory.
 *
 * NOTE: Once the central directory is written, no
 * further files can be added.
 */

static void addCentral(register TZIP *tzip)
{
	// If there was an error adding files, then don't write the Central directory
	if (!tzip->lasterr && !(tzip->flags & TZIP_DONECENTRALDIR))
	{
		DWORD		numentries;
		ULG			pos_at_start_of_central;

		numentries = 0;

		{
		register TZIPFILEINFO	*zfi;

		pos_at_start_of_central = tzip->writ;
		for (zfi = tzip->zfis; zfi; )
		{
			TZIPFILEINFO	*zfinext;

			// Write this TZIPFILEINFO entry to the Central directory
			writeDestShort(tzip, CENSIG);
			writeDestShort(tzip, CENSIG >> 16);
			writeDestShort(tzip, (USH)0x0B17);		// 0xB00 is win32 os-code. 0x17 is 23 in decimal: zip 2.3
			writeDestShort(tzip, (USH)20);			// Needs PKUNZIP 2.0 to unzip it
			writeDestShort(tzip, zfi->flg);
			writeDestShort(tzip, zfi->how);
			putpartial(zfi, tzip);
			writeDestShort(tzip, zfi->cext);
			writeDestShort(tzip, 0);
			writeDestShort(tzip, zfi->dsk);
			writeDestShort(tzip, zfi->att);
			writeDestShort(tzip, zfi->atx);
			writeDestShort(tzip, zfi->atx >> 16);
			writeDestShort(tzip, zfi->off);
			writeDestShort(tzip, zfi->off >> 16);
			writeDestination(tzip, zfi->iname, zfi->nam);
			if (zfi->cext) writeDestination(tzip, zfi->cextra, zfi->cext);

			// Update count of bytes written
			tzip->writ += 4 + CENHEAD + (unsigned int)zfi->nam + (unsigned int)zfi->cext;

			// Another entry added
			++numentries;

			// Free the TZIPFILEINFO now that we've written it out to the Central directory
			zfinext = zfi->nxt;
			GlobalFree(zfi);
			zfi = zfinext;
		}
		}

		// Write the end of the central-directory-data to the zip.
		writeDestShort(tzip, ENDSIG);
		writeDestShort(tzip, ENDSIG >> 16);
		writeDestShort(tzip, 0);
		writeDestShort(tzip, 0);
		writeDestShort(tzip, numentries);
		writeDestShort(tzip, numentries);
		numentries = tzip->writ - pos_at_start_of_central;
		writeDestShort(tzip, numentries);
		writeDestShort(tzip, numentries >> 16);
		pos_at_start_of_central += tzip->ooffset;
		writeDestShort(tzip, pos_at_start_of_central);
		writeDestShort(tzip, pos_at_start_of_central >> 16);
		writeDestShort(tzip, 0);
		tzip->writ += 4 + ENDHEAD + 0;

		// Done writing the central dir. Flag this so that another call here does not
		// write another central dir
		tzip->flags |= TZIP_DONECENTRALDIR;
	}
}






















// ========================== Encryption ========================

#define CRC32(c, b) (CrcTable[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
#define DO1(buf)  crc = CRC32(crc, *buf++)
#define DO2(buf)  DO1(buf); DO1(buf)
#define DO4(buf)  DO2(buf); DO2(buf)
#define DO8(buf)  DO4(buf); DO4(buf)

static ULG crc32(ULG crc, const UCH *buf, DWORD len)
{
	if (!buf) return(0);
	crc = crc ^ 0xffffffffL;
	while (len >= 8) {DO8(buf); len -= 8;}
	if (len) do {DO1(buf);} while (--len);
	return crc ^ 0xffffffffL;  // (instead of ~c for 64-bit machines)
}

static void update_keys(unsigned long *keys, char c)
{
	keys[0] = CRC32(keys[0],c);
	keys[1] += keys[0] & 0xFF;
	keys[1] = keys[1]*134775813L +1;
	keys[2] = CRC32(keys[2], keys[1] >> 24);
}

static char decrypt_byte(unsigned long *keys)
{
	unsigned temp = ((unsigned)keys[2] & 0xffff) | 2;
	return((char)(((temp * (temp ^ 1)) >> 8) & 0xff));
}

static char zencode(unsigned long *keys, char c)
{
	int t = decrypt_byte(keys);
	update_keys(keys,c);
	return((char)(t^c));
}




























// =============== Source and Destination "file" handling ==============


/****************** writeDestShort() *******************
 * Writes the specified short (from the memory buffer) to
 * the current position of the ZIP destination (ZIP output
 * file). Writes in Intel format.
 *
 * NOTE: If encryption is enabled, this encrypts the output
 * data without altering the contents of the memory buffer.
 *
 * NOTE: If an error, then tzip->lasterr holds the
 * (non-zero) error #. It is assumed that tzip->lasterr is
 * cleared before any write to the destination. Otherwise
 * writeDestination() does nothing.
 */

static void writeDestShort(TZIP *tzip, DWORD data)
{
	unsigned char	bytes[2];

	// If a previous error, do not write anything more
	if (!tzip->lasterr)
	{
		bytes[0] = (unsigned char)(data & 0xff);
		bytes[1] = (unsigned char)(data >> 8);

		// Encrypting data?
		if (tzip->flags & TZIP_ENCRYPT)
		{
			bytes[0] = zencode(tzip->keys, bytes[0]);
			bytes[1] = zencode(tzip->keys, bytes[1]);
		}

		// If writing to a destination memory buffer, make sure it is large enough, and then
		// copy data to it
		if (tzip->flags & TZIP_DESTMEMORY)
		{
			if (tzip->opos + 2 >= tzip->mapsize)
			{
				tzip->lasterr = ZR_MEMSIZE;
				goto out;
			}

			CopyMemory((unsigned char *)tzip->destination + tzip->opos, &bytes[0], 2);
			tzip->opos += 2;
		}

		// If writing to a handle, write out the data
		else
		{
			DWORD	written;

			if (!WriteFile(tzip->destination, &bytes[0], 2, &written, 0) || written != 2)
				tzip->lasterr = ZR_WRITE;
		}
	}
out:
	return;
}





/****************** writeDestination() *******************
 * Writes the specified bytes (from the memory buffer) to
 * the current position of the ZIP destination (ZIP output
 * file).
 *
 * NOTE: If encryption is enabled, this encrypts the output
 * data without altering the contents of the memory buffer.
 *
 * NOTE: If an error, then tzip->lasterr holds the
 * (non-zero) error #. It is assumed that tzip->lasterr is
 * cleared before any write to the destination. Otherwise
 * writeDestination() does nothing.
 */

static void writeDestination(TZIP *tzip, const char *buf, DWORD size)
{
	// If a previous error, do not write anything more
	if (size && !tzip->lasterr)
	{
		// Encrypting data? If so, make sure the encrpytion buffer is
		// big enough, then encrpyt to that buffer and write out that
		if (tzip->flags & TZIP_ENCRYPT)
		{
			unsigned int i;

			if (tzip->encbuf && tzip->encbufsize < size)
			{
				GlobalFree(tzip->encbuf);
				tzip->encbuf = 0;
			}

			if (!tzip->encbuf)
			{
				tzip->encbufsize = size << 1;
				if (!(tzip->encbuf = (char *)GlobalAlloc(GMEM_FIXED, tzip->encbufsize)))
				{
					tzip->lasterr = ZR_NOALLOC;
					goto bad;
				}
			}

			CopyMemory(tzip->encbuf, buf, size);
			for (i=0; i < size; i++) tzip->encbuf[i] = zencode(tzip->keys, tzip->encbuf[i]);
			buf = tzip->encbuf;
		}

		// If writing to a destination memory buffer, make sure it is large enough, and then
		// copy data to it
		if (tzip->flags & TZIP_DESTMEMORY)
		{
			if (tzip->opos + size >= tzip->mapsize)
			{
				tzip->lasterr = ZR_MEMSIZE;
				goto bad;
			}
			CopyMemory((unsigned char *)tzip->destination + tzip->opos, buf, size);
			tzip->opos += size;
		}

		// If writing to a handle, write out the data
		else
		{
			DWORD	written;

			if (!WriteFile(tzip->destination, buf, size, &written, 0) || written != size) tzip->lasterr = ZR_WRITE;
		}
	}
bad:
	return;
}





/****************** seekDestination() *******************
 * Seeks to the specified position from the start of the
 * ZIP destination (ZIP output file).
 */

static BOOL seekDestination(TZIP *tzip, unsigned int pos)
{
	if (!(tzip->flags & TZIP_CANSEEK))
	{
seekerr:
		tzip->lasterr = ZR_SEEK;
bad:	return(FALSE);
	}

	if (tzip->flags & TZIP_DESTMEMORY)
	{
		if (pos >= tzip->mapsize)
		{
			tzip->lasterr = ZR_MEMSIZE;
			goto bad;
		}
		tzip->opos = pos;
	}
	else if (SetFilePointer(tzip->destination, pos + tzip->ooffset, 0, FILE_BEGIN) == 0xFFFFFFFF) goto seekerr;
	return(TRUE);
}





/****************** srcHandleInfo() *******************
 * Fills in the TZIP with some information about an
 * open source file handle.
 */

static DWORD srcHandleInfo(TZIP *tzip, DWORD len, IZTIMES *times)
{
	DWORD	res;

	if ((tzip->ooffset = SetFilePointer(tzip->source, 0, 0, FILE_CURRENT)) != 0xFFFFFFFF)
	{
		tzip->flags |= TZIP_SRCCANSEEK;
		if ((res = getFileInfo(tzip, times)) != ZR_OK) return(res);
	}
	else
	{
		tzip->ooffset = 0;
		if (!len) len = (DWORD)-1;		// Can't know size until at the end unless we were told explicitly!
		tzip->isize = len;
	}

	return(ZR_OK);
}





/****************** readFromSource() *******************
 * Reads the specified bytes (from the current position of
 * the source) and copies them to the current position of
 * the specified memory buffer.
 *
 * RETURNS: The number of bytes read, or 0 if the end of
 * the source.
 */

static unsigned readFromSource(register TZIP *tzip, char *buf, unsigned size)
{
	DWORD	read;

	// Memory buffer
	if (tzip->flags & TZIP_SRCMEMORY)
	{
		if (tzip->posin >= tzip->lenin) goto bad;	// end of input
		read = tzip->lenin - tzip->posin;
		if (read > size) read = size;
		CopyMemory(buf, (unsigned char *)tzip->source + tzip->posin, read);
		tzip->posin += read;
	}
	else
	{
		if (!ReadFile(tzip->source, buf, size, &read, 0))
		{
			tzip->lasterr = ZR_READ;
bad:		return(0);
		}
	}

	tzip->totalRead += read;
	tzip->crc = crc32(tzip->crc, (UCH *)buf, read);
	return(read);
}





/****************** closeSource() *******************
 * Closes the source (that we added to the ZIP file).
 */

static DWORD closeSource(register TZIP *tzip)
{
	register DWORD	ret;

	ret = ZR_OK;

	// See if we need to close the source
	if (tzip->flags & TZIP_SRCCLOSEFH) CloseHandle(tzip->source);
	tzip->flags &= ~TZIP_SRCCLOSEFH;

	// Check that we've accounted for all the source bytes (assuming we knew the size initially)
	if (tzip->isize != (DWORD)-1 && tzip->isize != tzip->totalRead) ret = ZR_MISSIZE;

	// Update the count. The CRC has been done anyway, so we may as well
	tzip->isize = tzip->totalRead;

	return(ret);
}





/********************* ideflate() *********************
 * Adds the current source to the ZIP file, using the
 * deflate method.
 */

static void ideflate(TZIP *tzip, TZIPFILEINFO *zfi)
{
	register TSTATE		*state;

	// Make sure we have a TSTATE struct. It's a very big object -- 500k!
	// We allocate it on the heap, because PocketPC's stack breaks if
	// we try to put it all on the stack. It will be deleted when the
	// TZIP is deleted
	if ((state = tzip->state)) goto skip;

	if ((tzip->state = state = (TSTATE *)GlobalAlloc(GMEM_FIXED, sizeof(TSTATE))))
	{
		// Once-only initialization
		state->tzip = tzip;
		state->bs.out_buf = tzip->buf;
		state->bs.out_size = sizeof(tzip->buf);	// it used to be just 1024 bytes, not 16384 as here
		state->ts.static_dtree[0].dl.len = 0;	// this will make ct_init() realise it has to perform the init

		state->ts.l_desc.extra_bits = Extra_lbits;
		state->ts.d_desc.extra_bits = Extra_dbits;
		state->ts.bl_desc.extra_bits = Extra_blbits;
		state->ts.l_desc.dyn_tree = state->ts.dyn_ltree;
		state->ts.l_desc.static_tree = state->ts.static_ltree;
		state->ts.d_desc.dyn_tree = state->ts.dyn_dtree;
		state->ts.d_desc.static_tree = state->ts.static_dtree;
		state->ts.l_desc.extra_base = LITERALS+1;
		state->ts.l_desc.elems = L_CODES;
		state->ts.d_desc.elems = D_CODES;
		state->ts.l_desc.max_length = state->ts.d_desc.max_length = MAX_BITS;
		state->ts.bl_desc.dyn_tree = state->ts.bl_tree;
		state->ts.bl_desc.static_tree = 0;
		state->ts.d_desc.extra_base = state->ts.bl_desc.extra_base = 0;
		state->ts.bl_desc.elems = BL_CODES;
		state->ts.bl_desc.max_length = MAX_BL_BITS;
		state->ts.l_desc.max_code = state->ts.d_desc.max_code = state->ts.bl_desc.max_code = 0;

skip:	state->level = 8;
//		state->seekable = tzip->flags & TZIP_SRCCANSEEK ? 1 : 0;

//		state->ds.window_size =
		state->ts.last_lit = state->ts.last_dist = state->ts.last_flags =
		state->bs.out_offset = state->bs.bi_buf = state->bs.bi_valid = 0;
/*#ifndef NDEBUG
		state->bs.bits_sent = 0;
#endif*/
		ct_init(state, &zfi->att);

		lm_init(state, state->level, &zfi->flg);
		if (!tzip->lasterr)
		{
			// Compress the source into the zip
//			if (state->level <= 3) deflate_fast(state);
			deflate(state);
		}
	}
	else
		tzip->lasterr = ZR_NOALLOC;
}





/********************** istore() *********************
 * Adds the current source to the ZIP file, using the
 * store method.
 */

static void istore(register TZIP *tzip)
{
	register DWORD		cin;

	// If a memory buffer, we can write out those bytes all at once
	if (tzip->flags & TZIP_SRCMEMORY)
	{
		writeDestination(tzip, tzip->destination, tzip->lenin);
		tzip->csize = tzip->lenin;
	}

	// Otherwise, we read in the source in 16384 byte chunks, and write it out.

	// Read upto the next 16384 bytes from the source. If no more, we're done
	else while ((cin = readFromSource(tzip, tzip->buf, 16384)) && !tzip->lasterr)
	{
		// Write out those bytes
		writeDestination(tzip, tzip->buf, cin);

		// Increment total amount of bytes written (ie, the total size of the source)
		tzip->csize += cin;
	}
}






/******************** checkSuffix() *******************
 * Checks if a filename has a ZIP extension.
 */

static BOOL checkSuffix(const char *fn)
{
	const char *ext;

	// Locate the extension
	ext = fn + lstrlenA(fn);
	while (ext > fn && *ext != '.') ext--;
	if (ext != fn || *ext == '.')
	{
		LPCTSTR		strs;

		// Check for a ZIP extension
		strs = &ZipSuffixes[0];
		while (*strs)
		{
			if (!lstrcmpiA(ext, strs)) return(TRUE);
			strs += lstrlenA(strs) + 1;
		}
	}
	return(FALSE);
}





/************************* addSrc() ***********************
 * Compresses a source to the ZIP file.
 *
 * tzip =	Handle to TZIP gotten via one of the
 *			ZipCreate*() functions.
 *
 * destname = Desired name for the source when it is
 *			added to the ZIP.
 *
 * src =	Handle to the source to be added to the ZIP. This
 *			could be a pointer to a filename, a handle to an
 *			open file, a pointer to a memory buffer
 *			containing the contents to ZIP, or 0 if destname
 *			is a directory.
 *
 * flags =	ZIP_HANDLE, ZIP_FILENAME, ZIP_MEMORY, or ZIP_FOLDER.
 *			Also ZIP_UNICODE may be set.
 */

#define ZIP_HANDLE		0x00000001
#define ZIP_FILENAME	0x00000002
#define ZIP_MEMORY		0x00000004
#define ZIP_FOLDER		0x00000008
#define ZIP_UNICODE		0x00000010

DWORD addSrc(register TZIP *tzip, const void *destname, const void *src, DWORD len, DWORD flags)
{
	DWORD			passex;
	TZIPFILEINFO	*zfi;
	unsigned char	method;
	IZTIMES			times;

	if (IsBadReadPtr(tzip, 1)) goto badargs;

	// Can't add any more if the app did a ZipGetMemory
	if (tzip->flags & TZIP_DONECENTRALDIR) return(ZR_ENDED);

	// Re-init some stuff potentially left over from a previous addSrc()
	tzip->ooffset = tzip->crc = tzip->csize = tzip->totalRead = 0;
	tzip->flags &= ~(TZIP_SRCCANSEEK|TZIP_SRCCLOSEFH|TZIP_SRCMEMORY|TZIP_ENCRYPT);

	// ==================== Get the source (to compress to the ZIP) ===================

	switch (flags & ~ZIP_UNICODE)
	{
		// Zipping a file by name?
		case ZIP_FILENAME:
		{
			if (!src) goto badargs;
			if (flags & ZIP_UNICODE)
				tzip->source = CreateFileW((const WCHAR *)src, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			else
				tzip->source = CreateFileA((const char *)src, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (tzip->source == INVALID_HANDLE_VALUE)
				passex = ZR_NOFILE;
			else
			{
				if ((passex = srcHandleInfo(tzip, 0, &times)) == ZR_OK)
				{
					tzip->flags |= TZIP_SRCCLOSEFH;

					// If he didn't supply the destname, then use the same name as the source
					if (!destname) destname = src;

					goto chktime;
				}

				CloseHandle(tzip->source);
			}

badopen:	return(passex);
		}

		// Zipping a file by its open handle?
		case ZIP_HANDLE:
		{
			if (!(tzip->source = (HANDLE)src) || (HANDLE)src == INVALID_HANDLE_VALUE) goto badargs;
			if ((passex = srcHandleInfo(tzip, len, &times)) != ZR_OK) goto badopen;
chktime:	if (!(tzip->flags & TZIP_SRCCANSEEK)) goto gettime2;
			break;
		}

		// Zipping a folder name?
		case ZIP_FOLDER:
		{
			tzip->source = 0;
			tzip->isize = 0;
			times.attributes = 0x41C00010; // a readable writable directory, and again directory
			goto gettime;
		}

		// Zipping a memory buffer?
		default:
//		case ZIP_MEMORY:
		{
			WORD	dosdate, dostime;

			if (!src || !len) goto badargs;

			// Set the TZIP_SRCMEMORY flag because this source is from a memory buffer
			tzip->source = (HANDLE)src;
			tzip->isize = tzip->lenin = len;
			tzip->flags |= (TZIP_SRCMEMORY|TZIP_SRCCANSEEK);
			tzip->posin = 0;

gettime2:	times.attributes = 0x80000000;	// just a normal file

			// Set the time stamps to the current time
gettime:	getNow(&times.atime, &dosdate, &dostime);
			times.mtime = times.atime;
			times.ctime = times.atime;
			times.timestamp = (WORD)dostime | (((DWORD)dosdate) << 16);
		}
	}

	// ==================== Initialize the local header ===================
	// A zip "entry" consists of a local header (which includes the file name),
	// then the compressed data, and possibly an extended local header.

	// We need to allocate a TZIPFILEINFO + sizeof(EB_C_UT_SIZE)
	if (!(zfi = (TZIPFILEINFO *)GlobalAlloc(GMEM_FIXED, sizeof(TZIPFILEINFO) + EB_C_UT_SIZE)))
	{
		passex = ZR_NOALLOC;
		goto badout2;
	}
	ZeroMemory(zfi, sizeof(TZIPFILEINFO));

	// zip has its own notion of what filenames should look like, so we have to reformat
	// the name. First of all, ZIP does not support UNICODE names. Must be ANSI
	if (flags & ZIP_UNICODE)
		zfi->nam = WideCharToMultiByte(CP_UTF8, 0, (const WCHAR *)destname, -1, zfi->iname, MAX_PATH, 0, 0);
	else
	{
		lstrcpyA(zfi->iname, (const char *)destname);
		zfi->nam = lstrlenA((const char *)destname);
	}
	if (!zfi->nam)
	{
		GlobalFree(zfi);
badargs:
		passex = ZR_ARGS;
		goto badout2;
	}

	// Next we need to replace '\' with '/' chars
	{
	register char	*d;

	d = zfi->iname;
	while (*d)
	{
		if (*d == '\\') *d = '/';
		++d;
	}
	}

	// Determine whether app wants encryption, and whether we should use DEFLATE or STORE compression method
	passex = 0;
	zfi->flg = 8;		// 8 means 'there is an extra header'. Assume for the moment that we need it.
	method = STORE;
	zfi->tim = times.timestamp;
	zfi->atx = times.attributes;

	// Stuff the 'times' struct into zfi->extra. Apparently there's a problem with PocketPC
	// CE(zip)->CE(unzip) fails. Removing the following block fixes it up
	{
	char xloc[EB_L_UT_SIZE];

	zfi->extra = xloc;
	zfi->ext = EB_L_UT_SIZE;
	zfi->cextra = (unsigned char *)zfi + sizeof(TZIPFILEINFO);
	zfi->cext = EB_C_UT_SIZE;
	xloc[0]  = 'U';
	xloc[1]  = 'T';
	xloc[2]  = EB_UT_LEN(3);       // length of data part of e.f.
	xloc[3]  = 0;
	xloc[4]  = EB_UT_FL_MTIME | EB_UT_FL_ATIME | EB_UT_FL_CTIME;
	xloc[5]  = (char)(times.mtime);
	xloc[6]  = (char)(times.mtime >> 8);
	xloc[7]  = (char)(times.mtime >> 16);
	xloc[8]  = (char)(times.mtime >> 24);
	xloc[9]  = (char)(times.atime);
	xloc[10] = (char)(times.atime >> 8);
	xloc[11] = (char)(times.atime >> 16);
	xloc[12] = (char)(times.atime >> 24);
	xloc[13] = (char)(times.ctime);
	xloc[14] = (char)(times.ctime >> 8);
	xloc[15] = (char)(times.ctime >> 16);
	xloc[16] = (char)(times.ctime >> 24);
	CopyMemory(zfi->cextra, zfi->extra, EB_C_UT_SIZE);
	zfi->cextra[EB_LEN] = EB_UT_LEN(1);
	}

	if (tzip->flags & TZIP_OPTION_GZIP)
	{
		method = DEFLATE;
		zfi->ext = 0;
	}
	else if (flags & ZIP_FOLDER)
	{
		// If a folder, make sure its name ends with a '/'. Also we'll use STORE
		// method, and no encryption
		if (zfi->iname[zfi->nam - 1] != '/') zfi->iname[zfi->nam++] = '/';
	}
	else
	{
		// If password encryption, then every tzip->isize and tzip->csize is 12 bytes bigger
		if (tzip->password)
		{
			passex = 12;
			zfi->flg = 9;	// 8 + 1 = 'password-encrypted', with extra header
		}

		// If zipping another ZIP file, then use STORE (since it's already compressed). Otherwise, DEFLATE
		if (!checkSuffix(zfi->iname)) method = DEFLATE;
	}

	// Initialize other info for writing the local header. For some of this information,
	// we have to make assumptions since we won't know the real information until after
	// compression is done
	zfi->lflg = zfi->flg;
	zfi->how = (USH)method;
	// If STORE method, we know the "compressed" size right now, so set it. DEFLATE method, we'll get it later
	if (method == STORE && tzip->isize != (DWORD)-1) zfi->siz = tzip->isize + passex;
	zfi->len = (ULG)tzip->isize;

	// Set the (byte) offset within the ZIP archive where this local record starts
	zfi->off = tzip->writ + tzip->ooffset;

	// ============ Compress the source to the ZIP archive ================

	// Assume no error
	tzip->lasterr = ZR_OK;

	// Write the local header. Later, it will have to be rewritten, since we don't
	// know compressed size or crc yet. We get that info only after the compression
	// is done
	putlocal(zfi, tzip);
	if (tzip->lasterr != ZR_OK)
	{
reterr:	passex = tzip->lasterr;
badout:	GlobalFree(zfi);
badout2:
		if (tzip->flags & TZIP_SRCCLOSEFH) CloseHandle(tzip->source);
		return(passex);
	}

	// Increment the amount of bytes written
	if (tzip->flags & TZIP_OPTION_GZIP)
		tzip->writ = 11 + (unsigned int)zfi->nam;
	else
	{
		tzip->writ += 4 + LOCHEAD + (unsigned int)zfi->nam + (unsigned int)zfi->ext;

		// If needed, write the encryption header
		if (passex)
		{
			char		encbuf[12];
			int			i;
			const char	*cp;

			tzip->keys[0]=305419896L;
			tzip->keys[1]=591751049L;
			tzip->keys[2]=878082192L;
			for (cp = tzip->password; cp && *cp; cp++) update_keys(tzip->keys, *cp);
			// generate some random bytes
			for (i=0; i < 12; i++) encbuf[i] = (char)((rand() >> 7) & 0xff);
			encbuf[11] = (char)((zfi->tim >> 8) & 0xff);
			for (i=0; i < 12; i++) encbuf[i] = zencode(tzip->keys, encbuf[i]);
			{
				writeDestination(tzip, encbuf, 12);
				if (tzip->lasterr != ZR_OK) goto reterr;
				tzip->writ += 12;
			}

			// Enable encryption for below
			tzip->flags |= TZIP_ENCRYPT;
		}
	}

	// Compress the source contents to the zip file
	if (tzip->source)
	{
		if (method == DEFLATE)
			ideflate(tzip, zfi);
		else
			istore(tzip);

		// Check for an error
		if (tzip->lasterr != ZR_OK) goto reterr;

		// No more encryption below
		tzip->flags &= ~TZIP_ENCRYPT;

		// Done with the source, so close it
		closeSource(tzip);

		tzip->writ += tzip->csize;
	}

	// For GZIP format, we allow only 1 file in the archive and no central directory
	if (tzip->flags & TZIP_OPTION_GZIP)
	{
		tzip->flags |= TZIP_DONECENTRALDIR;

		// Write out the CRC and uncompressed size
		writeDestShort(tzip, tzip->crc);
		writeDestShort(tzip, tzip->crc >> 16);
		writeDestShort(tzip, tzip->totalRead);
		writeDestShort(tzip, tzip->totalRead >> 16);
		if (tzip->lasterr != ZR_OK) goto reterr;
	}
	else
	{
		{
		// Update the local header now that we have some final information about the compression
		unsigned char	first_header_has_size_right;

		first_header_has_size_right = (zfi->siz == tzip->csize + passex);

		// Update the CRC, compressed size, original size. Also save them for when we write the central directory
		zfi->crc = tzip->crc;
		zfi->siz = tzip->csize + passex;
		zfi->len = tzip->isize;

		// If we can seek in the source, seek to the local header and rewrite it with correct information
		if ((tzip->flags & TZIP_CANSEEK) && !passex)
		{
			// Update what compression method we used
			zfi->how = (USH)method;

			// Clear the extended local header flag and update the local header's flags
			if (!(zfi->flg & 1)) zfi->flg &= ~8;
			zfi->lflg = zfi->flg;

			// Rewrite the local header
			if (!seekDestination(tzip, zfi->off - tzip->ooffset))
			{
badseek:		passex = ZR_SEEK;
				goto badout;
			}
			putlocal(zfi, tzip);
			if (tzip->lasterr != ZR_OK) goto reterr;
			if (!seekDestination(tzip, tzip->writ)) goto badseek;
		}

		// Otherwise, we put an updated (extended) header at the end
		else
		{
			// We can't change the compression method from our initial assumption
			if (zfi->how != (USH)method || (method == STORE && !first_header_has_size_right))
			{
				passex = ZR_NOCHANGE;
				goto badout;
			}
			putextended(zfi, tzip);
			if (tzip->lasterr != ZR_OK) goto reterr;

			tzip->writ += 16;

			// Store final flg for writing the central directory, just in case it was modified by inflate()
			zfi->flg = zfi->lflg;
		}
		}

		// ============ Book-keeping for Central Directory ================

		// Keep the ZIPFILEINFO, for when we write our end-of-zip directory later.
		// Link it at the end of the list
		if (!tzip->zfis) tzip->zfis = zfi;
		else
		{
			register TZIPFILEINFO *z;

			z = tzip->zfis;
			while (z->nxt) z = z->nxt;
			z->nxt = zfi;
		}
	}

	return(ZR_OK);
}














// ======================= Callable API ========================
/*
DWORD WINAPI ZipAddFileA(HZIP tzip, const char *destname, const char *fn)
{
	return(addSrc((TZIP *)tzip, (void *)destname, (void *)fn, 0, ZIP_FILENAME));
}

DWORD WINAPI ZipAddFileW(HZIP tzip, const WCHAR *destname, const WCHAR *fn)
{
	return(addSrc((TZIP *)tzip, (void *)destname, (void *)fn, 0, ZIP_FILENAME|ZIP_UNICODE));
}

DWORD WINAPI ZipAddBufferA(HZIP tzip, const char *destname, const void *src, DWORD len)
{
	return(addSrc((TZIP *)tzip, (void *)destname, src, len, ZIP_MEMORY));
}

DWORD WINAPI ZipAddBufferW(HZIP tzip, const WCHAR *destname, const void *src, DWORD len)
{
	return(addSrc((TZIP *)tzip, (void *)destname, src, len, ZIP_MEMORY|ZIP_UNICODE));
}

DWORD WINAPI ZipAddHandleA(HZIP tzip, const char *destname, HANDLE h)
{
	return(addSrc((TZIP *)tzip, (void *)destname, h, 0, ZIP_HANDLE));
}

DWORD WINAPI ZipAddHandleW(HZIP tzip, const WCHAR *destname, HANDLE h)
{
	return(addSrc((TZIP *)tzip, (void *)destname, h, 0, ZIP_HANDLE|ZIP_UNICODE));
}

DWORD WINAPI ZipAddPipeA(HZIP tzip, const char *destname, HANDLE h, DWORD len)
{
	return(addSrc((TZIP *)tzip, (void *)destname, h, len, ZIP_HANDLE));
}

DWORD WINAPI ZipAddPipeW(HZIP tzip, const WCHAR *destname, HANDLE h, DWORD len)
{
	return(addSrc((TZIP *)tzip, (void *)destname, h, len, ZIP_HANDLE|ZIP_UNICODE));
}

DWORD WINAPI ZipAddFolderA(HZIP tzip, const char *destname)
{
	return(addSrc((TZIP *)tzip, (void *)destname, 0, 0, ZIP_FOLDER));
}

DWORD WINAPI ZipAddFolderW(HZIP tzip, const WCHAR *destname)
{
	return(addSrc((TZIP *)tzip, (void *)destname, 0, 0, ZIP_FOLDER|ZIP_UNICODE));
}









DWORD WINAPI ZipFormatMessageA(DWORD code, char *buf, DWORD len)
{
	if (code == ZR_OK) code = IDS_OK;
	if (!(code = LoadStringA(ThisInstance, code, buf, len)))
		code = LoadStringA(ThisInstance, IDS_UNKNOWN, buf, len);
	return(code);
}

DWORD WINAPI ZipFormatMessageW(DWORD code, WCHAR *buf, DWORD len)
{
	if (code == ZR_OK) code = IDS_OK;
	if (!(code = LoadStringW(ThisInstance, code, buf, len)))
		code = LoadStringW(ThisInstance, IDS_UNKNOWN, buf, len);
	return(code);
}

*/




static void free_tzip(TZIP *tzip)
{
	// Free various buffers
	if (tzip->state) GlobalFree(tzip->state);
	if (tzip->encbuf) GlobalFree(tzip->encbuf);
	if (tzip->password) GlobalFree(tzip->password);

	// Free the TZIP itself
	GlobalFree(tzip);
}

/********************* ZipClose() **********************
 * Closes the ZIP file that was created/opened with one
 * of the ZipCreate* functions.
 */

DWORD WINAPI ZipClose(HZIP tzip)
{
	DWORD	result;

	// Make sure TZIP if valid
	if (IsBadReadPtr(tzip, 1))
		result = ZR_ARGS;
	else
	{
		result = ZR_OK;
		if (((TZIP *)tzip)->destination)
		{
			// If the directory wasn't already added via a call to ZipGetMemory, then we do it now
			addCentral((TZIP *)tzip);
			result = ((TZIP *)tzip)->lasterr;

			// If we created a memory-mapped file, free it now
			if (((TZIP *)tzip)->flags & TZIP_DESTMEMORY)
			{
				UnmapViewOfFile(((TZIP *)tzip)->destination);
				CloseHandle(((TZIP *)tzip)->memorymap);
			}

			// If we opened the handle, close it now
			if (((TZIP *)tzip)->flags & TZIP_DESTCLOSEFH) CloseHandle(((TZIP *)tzip)->destination);
		}

		// Free various buffers and the TZIP
		free_tzip((TZIP *)tzip);
	}

	return(result);
}








/********************* createZip() **********************
 * Does all the real work for the ZipCreate* functions.
 */

/*static*/ DWORD createZip(HZIP *zipHandle, void *z, DWORD len, DWORD flags, const char *pwd)
{
	register TZIP	*tzip;
	DWORD			result;

	// Get a TZIP struct
	if (!(tzip = (TZIP *)GlobalAlloc(GMEM_FIXED, sizeof(TZIP))))
	{
		result = ZR_NOALLOC;
		goto done;
	}

	// Clear out all fields except the temporary buffer at the end
	ZeroMemory(tzip, sizeof(TZIP) - 16384);

	// If password is passed, make a copy of it
	if (pwd && *pwd)
	{
		if (!(tzip->password = (char *)GlobalAlloc(GMEM_FIXED, (lstrlenA(pwd) + 1)))) goto badalloc;
		lstrcpyA(tzip->password, pwd);
	}

	switch (flags & ~ZIP_UNICODE)
	{
		// Passed a file handle?
		case ZIP_HANDLE:
		{
			// Try to duplicate the handle. If successful, flag that we have to close it later
			if (DuplicateHandle(GetCurrentProcess(), z, GetCurrentProcess(), &tzip->destination, 0, 0, DUPLICATE_SAME_ACCESS))
				tzip->flags |= TZIP_DESTCLOSEFH;
			else
				tzip->destination = z;

			// See if we can seek on it. If so, get the current position
			if ((tzip->ooffset = getFilePos(tzip->destination)) != 0xFFFFFFFF)
				tzip->flags |= TZIP_CANSEEK;
			else
				tzip->ooffset = 0;

			goto good;
		}

		// Passed a filename?
		case ZIP_FILENAME:
		{
			// Open the file and store the handle
			if (flags & ZIP_UNICODE)
				tzip->destination = CreateFileW((const WCHAR *)z, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			else
				tzip->destination = CreateFileA((const char *)z, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (tzip->destination == INVALID_HANDLE_VALUE)
			{
				result = ZR_NOFILE;
				goto freeit;
			}

			tzip->flags |= TZIP_CANSEEK|TZIP_DESTCLOSEFH;
			goto good;
		}

		// Passed a memory buffer where we store the zip file?
		case ZIP_MEMORY:
		{
			// Make sure he passed a non-zero length
			if (!len)
			{
				result = ZR_MEMSIZE;
				goto freeit;
			}

			// If he supplied the buffer, store it. Otherwise, we need to create some
			// memory-mapped file of the requested size
			if (!(tzip->destination = z))
			{
				if (!(tzip->memorymap = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, len, 0))) goto badalloc;
				if (!(tzip->destination = (HANDLE)MapViewOfFile(tzip->memorymap, FILE_MAP_ALL_ACCESS, 0, 0, len)))
				{
					CloseHandle(tzip->memorymap);
badalloc:			result = ZR_NOALLOC;
					goto freeit;
				}
			}
			else if (IsBadReadPtr(z, len))
				goto argerr;

			tzip->flags |= (TZIP_CANSEEK|TZIP_DESTMEMORY);
			tzip->mapsize = len;

			// Success
good:		result = ZR_OK;
			break;
		}

		default:
		{
argerr:		result = ZR_ARGS;
freeit:		ZipClose((HZIP)tzip);
			tzip = 0;
		}
	}
done:
	// Return TZIP * to the caller
	*zipHandle = (HZIP)tzip;

	// Return error code to caller
	return(result);
}

/******************** ZipCreate*() ********************
 * Creates/Opens a ZIP archive, in preparation of
 * compressing files into it. The archive can be
 * created on disk, or in memory, or to a pipe or
 * an already open file.
 */

DWORD WINAPI ZipCreateHandle(HZIP *zipHandle, HANDLE h, const char *password)
{
	return(createZip(zipHandle, h, 0, ZIP_HANDLE, password));
}

DWORD WINAPI ZipCreateFileA(HZIP *zipHandle, const char *fn, const char *password)
{
	return(createZip(zipHandle, (void *)fn, 0, ZIP_FILENAME, password));
}

DWORD WINAPI ZipCreateFileW(HZIP *zipHandle, const WCHAR *fn, const char *password)
{
	return(createZip(zipHandle, (void *)fn, 0, ZIP_FILENAME|ZIP_UNICODE, password));
}

DWORD WINAPI ZipCreateBuffer(HZIP *zipHandle, void *z, DWORD len, const char *password)
{
	return(createZip(zipHandle, z, len, ZIP_MEMORY, password));
}








/*********************** ZipGetMemory() ***********************
 * Called by an application to get the memory-mapped address
 * created by ZipCreateBuffer() (after the app has called the
 * ZipAdd* functions to zip up some contents into it, and before
 * the app ZipClose()'s it).
 *
 */

DWORD WINAPI ZipGetMemory(HZIP tzip, void **pbuf, DWORD *plen, HANDLE *base)
{
	DWORD	result;

	if (IsBadReadPtr(tzip, 1))
	{
		result = ZR_ARGS;
zero:	*pbuf = 0;
		*plen = 0;
		if (base)
		{
			*base = 0;
			if (result != ZR_ARGS) free_tzip((TZIP *)tzip);
		}
	}
	else
	{
		// When the app calls ZipGetMemory, it has presumably finished
		// adding all of files to the ZIP. In any case, we have to write
		// the central directory now, otherwise the memory we return won't
		// be a complete ZIP file
		addCentral((TZIP *)tzip);
		result = ((TZIP *)tzip)->lasterr;

		if (!((TZIP *)tzip)->memorymap)
		{
			if (!result) result = ZR_NOTMMAP;
			goto zero;
		}
		else
		{
			// Return the memory buffer and size
			*pbuf = (void *)((TZIP *)tzip)->destination;
			*plen = ((TZIP *)tzip)->writ;
		}

		// Does caller want everything freed except for the memory?
		if (base)
		{
			*base = ((TZIP *)tzip)->memorymap;
			free_tzip((TZIP *)tzip);
		}
	}

	return(result);
}





/*********************** ZipResetMemory() ***********************
 * Called by an application to reset a memory-mapped HZIP for
 * compressing a new ZIP file within it.
 */

DWORD WINAPI ZipResetMemory(HZIP tzip)
{
	register HANDLE		memorymap;

	if (IsBadReadPtr(tzip, 1) || !(((TZIP *)tzip)->flags & TZIP_DESTMEMORY)) return(ZR_ARGS);

	// Did we create the memory?
	if ((memorymap = ((TZIP *)tzip)->memorymap))
	{
		UnmapViewOfFile(((TZIP *)tzip)->destination);
		if (!(((TZIP *)tzip)->destination = (HANDLE)MapViewOfFile(memorymap, FILE_MAP_ALL_ACCESS, 0, 0, ((TZIP *)tzip)->mapsize)))
		{
			CloseHandle(memorymap);
			free_tzip((TZIP *)tzip);
			return(ZR_NOALLOC);
		}
	}

	// Reset certain fields of the TZIP
	((TZIP *)tzip)->lasterr = ((TZIP *)tzip)->opos = ((TZIP *)tzip)->writ = 0;
	((TZIP *)tzip)->state->ts.static_dtree[0].dl.len = 0;
	return(ZR_OK);
}





/*********************** ZipOptions() ***********************
 * Called by an application to reset a memory-mapped HZIP for
 * compressing a new ZIP file within it.
 */

DWORD WINAPI ZipOptions(HZIP tzip, DWORD flags)
{
	if (IsBadReadPtr(tzip, 1) || (flags & ~(TZIP_OPTION_GZIP))) return(ZR_ARGS);
	((TZIP *)tzip)->flags |= flags;
	return(ZR_OK);
}





/************************* DLLMain() ************************
 * Automatically called by Win32 when the DLL is loaded or
 * unloaded.
 */
/*
BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// Save the module handle. It will be the same for all instances of the DLL
	ThisInstance = hinstDLL;

    switch(fdwReason)
	{
		// ==============================================================
		case DLL_PROCESS_ATTACH:
		{
			// Seed random generator
			srand(GetTickCount() ^ (unsigned long)GetDesktopWindow());
			break;
		}

		// ==============================================================
		case DLL_THREAD_ATTACH:
		{
			DisableThreadLibraryCalls(hinstDLL);
			break;
		}

//		case DLL_THREAD_DETACH:
//			break;

		// ==============================================================
//		case DLL_PROCESS_DETACH:
	}

	/* Success *//*
	return(1);
}*/
