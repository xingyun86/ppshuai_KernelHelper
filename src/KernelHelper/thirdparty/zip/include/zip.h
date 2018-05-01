#ifndef __ZIP_H
#define __ZIP_H

#include "zdef.h"

HZIP CreateZip(const ZTCHAR *fn, const char *password);
HZIP CreateZip(void *buf,unsigned int len, const char *password);
HZIP CreateZipHandle(ZHANDLE h, const char *password);
// CreateZip - call this to start the creation of a zip file.
// As the zip is being created, it will be stored somewhere:
// to a pipe:              CreateZipHandle(hpipe_write);
// in a file (by handle):  CreateZipHandle(hfile);
// in a file (by name):    CreateZip("c:\\test.zip");
// in memory:              CreateZip(buf, len);
// or in pagefile memory:  CreateZip(0, len);
// The final case stores it in memory backed by the system paging file,
// where the zip may not exceed len bytes. This is a bit friendlier than
// allocating memory with new[]: it won't lead to fragmentation, and the
// memory won't be touched unless needed. That means you can give very
// large estimates of the maximum-size without too much worry.
// As for the password, it lets you encrypt every file in the archive.
// (This api doesn't support per-file encryption.)
// Note: because pipes don't allow random access, the structure of a zipfile
// created into a pipe is slightly different from that created into a file
// or memory. In particular, the compressed-size of the item cannot be
// stored in the zipfile until after the item itself. (Also, for an item added
// itself via a pipe, the uncompressed-size might not either be known until
// after.) This is not normally a problem. But if you try to unzip via a pipe
// as well, then the unzipper will not know these things about the item until
// after it has been unzipped. Therefore: for unzippers which don't just write
// each item to disk or to a pipe, but instead pre-allocate memory space into
// which to unzip them, then either you have to create the zip not to a pipe,
// or you have to add items not from a pipe, or at least when adding items
// from a pipe you have to specify the length.
// Note: for windows-ce, you cannot close the handle until after CloseZip.
// but for real windows, the zip makes its own copy of your handle, so you
// can close yours anytime.


ZZRESULT ZipAdd(HZIP hz,const ZTCHAR *dstzn, const ZTCHAR *fn);
ZZRESULT ZipAdd(HZIP hz,const ZTCHAR *dstzn, void *src,unsigned int len);
ZZRESULT ZipAddHandle(HZIP hz,const ZTCHAR *dstzn, ZHANDLE h);
ZZRESULT ZipAddHandle(HZIP hz,const ZTCHAR *dstzn, ZHANDLE h, unsigned int len);
ZZRESULT ZipAddFolder(HZIP hz,const ZTCHAR *dstzn);
// ZipAdd - call this for each file to be added to the zip.
// dstzn is the name that the file will be stored as in the zip file.
// The file to be added to the zip can come
// from a pipe:  ZipAddHandle(hz,"file.dat", hpipe_read);
// from a file:  ZipAddHandle(hz,"file.dat", hfile);
// from a filen: ZipAdd(hz,"file.dat", "c:\\docs\\origfile.dat");
// from memory:  ZipAdd(hz,"subdir\\file.dat", buf,len);
// (folder):     ZipAddFolder(hz,"subdir");
// Note: if adding an item from a pipe, and if also creating the zip file itself
// to a pipe, then you might wish to pass a non-zero length to the ZipAddHandle
// function. This will let the zipfile store the item's size ahead of the
// compressed item itself, which in turn makes it easier when unzipping the
// zipfile from a pipe.

ZZRESULT ZipGetMemory(HZIP hz, void **buf, unsigned long *len);
// ZipGetMemory - If the zip was created in memory, via ZipCreate(0,len),
// then this function will return information about that memory block.
// buf will receive a pointer to its start, and len its length.
// Note: you can't add any more after calling this.


// e.g.
//
// (1) Traditional use, creating a zipfile from existing files
//     HZIP hz = CreateZip("c:\\simple1.zip",0);
//     ZipAdd(hz,"znsimple.bmp", "c:\\simple.bmp");
//     ZipAdd(hz,"znsimple.txt", "c:\\simple.txt");
//     CloseZip(hz);
//
// (2) Memory use, creating an auto-allocated mem-based zip file from various sources
//     HZIP hz = CreateZip(0,100000, 0);
//     // adding a conventional file...
//     ZipAdd(hz,"src1.txt",  "c:\\src1.txt");
//     // adding something from memory...
//     char buf[1000]; for (int i=0; i<1000; i++) buf[i]=(char)(i&0x7F);
//     ZipAdd(hz,"file.dat",  buf,1000);
//     // adding something from a pipe...
//     ZHANDLE hread,hwrite; CreatePipe(&hread,&hwrite,NULL,0);
//     ZHANDLE hthread = CreateThread(0,0,ThreadFunc,(void*)hwrite,0,0);
//     ZipAdd(hz,"unz3.dat",  hread,1000);  // the '1000' is optional.
//     WaitForSingleObject(hthread,INFINITE);
//     CloseHandle(hthread); CloseHandle(hread);
//     ... meanwhile ZDWORD WINAPI ThreadFunc(void *dat)
//                   { ZHANDLE hwrite = (ZHANDLE)dat;
//                     char buf[1000]={17};
//                     ZDWORD writ; WriteFile(hwrite,buf,1000,&writ,NULL);
//                     CloseHandle(hwrite);
//                     return 0;
//                   }
//     // and now that the zip is created, let's do something with it:
//     void *zbuf; unsigned long zlen; ZipGetMemory(hz,&zbuf,&zlen);
//     ZHANDLE hfz = CreateFile("test2.zip",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
//     ZDWORD writ; WriteFile(hfz,zbuf,zlen,&writ,NULL);
//     CloseHandle(hfz);
//     CloseZip(hz);
//
// (3) Handle use, for file handles and pipes
//     ZHANDLE hzread,hzwrite; CreatePipe(&hzread,&hzwrite,0,0);
//     ZHANDLE hthread = CreateThread(0,0,ZipReceiverThread,(void*)hzread,0,0);
//     HZIP hz = CreateZipHandle(hzwrite,0);
//     // ... add to it
//     CloseZip(hz);
//     CloseHandle(hzwrite);
//     WaitForSingleObject(hthread,INFINITE);
//     CloseHandle(hthread);
//     ... meanwhile ZDWORD WINAPI ZipReceiverThread(void *dat)
//                   { ZHANDLE hread = (ZHANDLE)dat;
//                     char buf[1000];
//                     while (true)
//                     { ZDWORD red; ReadFile(hread,buf,1000,&red,NULL);
//                       // ... and do something with this zip data we're receiving
//                       if (red==0) break;
//                     }
//                     CloseHandle(hread);
//                     return 0;
//                   }



// Now we indulge in a little skullduggery so that the code works whether
// the user has included just zip or both zip and unzip.
// Idea: if header files for both zip and unzip are present, then presumably
// the cpp files for zip and unzip are both present, so we will call
// one or the other of them based on a dynamic choice. If the header file
// for only one is present, then we will bind to that particular one.
ZZRESULT CloseZipZ(HZIP hz);
unsigned int ZFormatZipMessageZ(ZZRESULT code, ZTCHAR *buf,unsigned int len);
bool IsZipHandleZ(HZIP hz);
#ifdef __UNZIP_H
#undef CloseZip
#define CloseZip(hz) (IsZipHandleZ(hz)?CloseZipZ(hz):CloseZipU(hz))
#else
#define CloseZip CloseZipZ
#define FormatZipMessage FormatZipMessageZ
#endif


#ifdef ZIP_STD
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include <ctype.h>

#else
#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>

#endif


// THIS FILE is almost entirely based upon code by info-zip.
// It has been modified by Lucian Wischik. The modifications
// were a complete rewrite of the bit of code that generates the
// layout of the zipfile, and support for zipping to/from memory
// or handles or pipes or pagefile or diskfiles, encryption, unicode.
// The original code may be found at http://www.info-zip.org
// The original copyright text follows.
//
//
//
// This is version 1999-Oct-05 of the Info-ZIP copyright and license.
// The definitive version of this document should be available at
// ftp://ftp.cdrom.com/pub/infozip/license.html indefinitely.
//
// Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.
//
// For the purposes of this copyright and license, "Info-ZIP" is defined as
// the following set of individuals:
//
//   Mark Adler, John Bush, Karl Davis, Harald Denker, Jean-Michel Dubois,
//   Jean-loup Gailly, Hunter Goatley, Ian Gorman, Chris Herborth, Dirk Haase,
//   Greg Hartwig, Robert Heath, Jonathan Hudson, Paul Kienitz, David Kirschbaum,
//   Johnny Lee, Onno van der Linden, Igor Mandrichenko, Steve P. Miller,
//   Sergio Monesi, Keith Owens, George Petrov, Greg Roelofs, Kai Uwe Rommel,
//   Steve Salisbury, Dave Smith, Christian Spieler, Antoine Verheijen,
//   Paul von Behren, Rich Wales, Mike White
//
// This software is provided "as is," without warranty of any kind, express
// or implied.  In no event shall Info-ZIP or its contributors be held liable
// for any direct, indirect, incidental, special or consequential damages
// arising out of the use of or inability to use this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       definition, disclaimer, and this list of conditions.
//
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, definition, disclaimer, and this list of conditions in
//       documentation and/or other materials provided with the distribution.
//
//    3. Altered versions--including, but not limited to, ports to new operating
//       systems, existing ports with new graphical interfaces, and dynamic,
//       shared, or static library versions--must be plainly marked as such
//       and must not be misrepresented as being the original source.  Such
//       altered versions also must not be misrepresented as being Info-ZIP
//       releases--including, but not limited to, labeling of the altered
//       versions with the names "Info-ZIP" (or any variation thereof, including,
//       but not limited to, different capitalizations), "Pocket UnZip," "WiZ"
//       or "MacZip" without the explicit permission of Info-ZIP.  Such altered
//       versions are further prohibited from misrepresentative use of the
//       Zip-Bugs or Info-ZIP e-mail addresses or of the Info-ZIP URL(s).
//
//    4. Info-ZIP retains the right to use the names "Info-ZIP," "Zip," "UnZip,"
//       "WiZ," "Pocket UnZip," "Pocket Zip," and "MacZip" for its own source and
//       binary releases.
//

// Error return values.  The values 0..4 and 12..18 follow the conventions
// of PKZIP.   The values 4..10 are all assigned to "insufficient memory"
// by PKZIP, so the codes 5..10 are used here for other purposes.
#define ZZE_MISS         -1      // used by procname(), zipbare()
#define ZZE_OK           0       // success
#define ZZE_EOF          2       // unexpected end of zip file
#define ZZE_FORM         3       // zip file structure error
#define ZZE_MEM          4       // out of memory
#define ZZE_LOGIC        5       // internal logic error
#define ZZE_BIG          6       // entry too large to split
#define ZZE_NOTE         7       // invalid comment format
#define ZZE_TEST         8       // zip test (-T) failed or out of memory
#define ZZE_ABORT        9       // user interrupt or termination
#define ZZE_TEMP         10      // error using a temp file
#define ZZE_READ         11      // read or seek error
#define ZZE_NONE         12      // nothing to do
#define ZZE_NAME         13      // missing or empty zip file
#define ZZE_WRITE        14      // error writing to a file
#define ZZE_CREAT        15      // couldn't open to write
#define ZZE_PARMS        16      // bad command line
#define ZZE_OPEN         18      // could not open a specified file to read
#define ZZE_MAXERR       18      // the highest error number


// internal file attribute
#define ZUNKNOWN (-1)
#define ZBINARY  0
#define ZASCII   1

#define ZBEST -1                 // Use best method (deflation or store)
#define ZSTORE 0                 // Store method
#define ZDEFLATE 8               // Deflation method

#define ZCRCVAL_INITIAL  0L

// MSDOS file or directory attributes
#define ZMSDOS_HIDDEN_ATTR 0x02
#define ZMSDOS_DIR_ATTR 0x10

// Lengths of headers after signatures in bytes
#define ZLOCHEAD 26
#define ZCENHEAD 42
#define ZENDHEAD 18

// Definitions for extra field handling:
#define ZEB_HEADSIZE       4     /* length of a extra field block header */
#define ZEB_LEN            2     /* offset of data length field in header */
#define ZEB_UT_MINLEN      1     /* minimal UT field contains Flags byte */
#define ZEB_UT_FLAGS       0     /* byte offset of Flags field */
#define ZEB_UT_TIME1       1     /* byte offset of 1st time value */
#define ZEB_UT_FL_MTIME    (1 << 0)      /* mtime present */
#define ZEB_UT_FL_ATIME    (1 << 1)      /* atime present */
#define ZEB_UT_FL_CTIME    (1 << 2)      /* ctime present */
#define ZEB_UT_LEN(n)      (ZEB_UT_MINLEN + 4 * (n))
#define ZEB_L_UT_SIZE    (ZEB_HEADSIZE + ZEB_UT_LEN(3))
#define ZEB_C_UT_SIZE    (ZEB_HEADSIZE + ZEB_UT_LEN(1))


// Macros for writing machine integers to little-endian format
#define ZPUTSH(a,f) {char _putsh_c=(char)((a)&0xff); wfunc(param,&_putsh_c,1); _putsh_c=(char)((a)>>8); wfunc(param,&_putsh_c,1);}
#define ZPUTLG(a,f) {ZPUTSH((a) & 0xffff,(f)) ZPUTSH((a) >> 16,(f))}


// -- Structure of a ZIP file --
// Signatures for zip file information headers
#define ZLOCSIG     0x04034b50L
#define ZCENSIG     0x02014b50L
#define ZENDSIG     0x06054b50L
#define ZEXTLOCSIG  0x08074b50L


#define ZMIN_MATCH  3
#define ZMAX_MATCH  258
// The minimum and maximum match lengths


#define ZWSIZE  (0x8000)
// Maximum window size = 32K. If you are really short of memory, compile
// with a smaller WSIZE but this reduces the compression ratio for files
// of size > WSIZE. WSIZE must be a power of two in the current implementation.
//

#define ZMIN_LOOKAHEAD (ZMAX_MATCH+ZMIN_MATCH+1)
// Minimum amount of lookahead, except at the end of the input file.
// See deflate.c for comments about the MIN_MATCH+1.
//

#define ZMAX_DIST  (ZWSIZE-ZMIN_LOOKAHEAD)
// In order to simplify the code, particularly on 16 bit machines, match
// distances are limited to MAX_DIST instead of WSIZE.
//


#define ZZIP_HANDLE   1
#define ZZIP_FILENAME 2
#define ZZIP_MEMORY   3
#define ZZIP_FOLDER   4



// ===========================================================================
// Constants
//

#define ZMAX_BITS 15
// All codes must not exceed MAX_BITS bits

#define ZMAX_BL_BITS 7
// Bit length codes must not exceed MAX_BL_BITS bits

#define ZLENGTH_CODES 29
// number of length codes, not counting the special END_BLOCK code

#define ZLITERALS  256
// number of literal bytes 0..255

#define ZEND_BLOCK 256
// end of block literal code

#define ZL_CODES (ZLITERALS+1+ZLENGTH_CODES)
// number of Literal or Length codes, including the END_BLOCK code

#define ZD_CODES   30
// number of distance codes

#define ZBL_CODES  19
// number of codes used to transfer the bit lengths


#define ZSTORED_BLOCK 0
#define ZSTATIC_TREES 1
#define ZDYN_TREES    2
// The three kinds of block type

#define ZLIT_BUFSIZE  0x8000
#define ZDIST_BUFSIZE  ZLIT_BUFSIZE
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
//

#define ZREP_3_6      16
// repeat previous bit length 3-6 times (2 bits of repeat count)

#define ZREPZ_3_10    17
// repeat a zero length 3-10 times  (3 bits of repeat count)

#define ZREPZ_11_138  18
// repeat a zero length 11-138 times  (7 bits of repeat count)

#define ZHEAP_SIZE (2*ZL_CODES+1)
// maximum heap size


// ===========================================================================
// Local data used by the "bit string" routines.
//

#define ZBuf_size (8 * 2*sizeof(char))
// Number of bits used within bi_buf. (bi_buf may be implemented on
// more than 16 bits on some systems.)

// Output a 16 bit value to the bit stream, lower (oldest) byte first
#define ZPUTSHORT(state,w) \
{ if (state.bs.out_offset >= state.bs.out_size-1) \
    state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset); \
  state.bs.out_buf[state.bs.out_offset++] = (char) ((w) & 0xff); \
  state.bs.out_buf[state.bs.out_offset++] = (char) ((uShort)(w) >> 8); \
}

#define ZPUTBYTE(state,b) \
{ if (state.bs.out_offset >= state.bs.out_size) \
    state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset); \
  state.bs.out_buf[state.bs.out_offset++] = (char) (b); \
}

// DEFLATE.CPP HEADER

#define ZHASH_BITS  15
// For portability to 16 bit machines, do not use values above 15.

#define ZHASH_SIZE (unsigned)(1<<ZHASH_BITS)
#define ZHASH_MASK (ZHASH_SIZE-1)
#define ZWMASK     (ZWSIZE-1)
// ZHASH_SIZE and ZWSIZE must be powers of two

#define ZNIL 0
// Tail of hash chains

#define ZFAST 4
#define ZSLOW 2
// speed options for the general purpose bit flag

#define ZTOO_FAR 4096
// Matches of length 3 are discarded if their distance exceeds TOO_FAR



#define ZEQUAL 0
// result of memcmp for equal strings


// ===========================================================================
// Local data used by the "longest match" routines.

#define ZH_SHIFT  ((ZHASH_BITS+ZMIN_MATCH-1)/ZMIN_MATCH)
// Number of bits by which ins_h and del_h must be shifted at each
// input step. It must be such that after MIN_MATCH steps, the oldest
// byte no longer takes part in the hash key, that is:
//   H_SHIFT * MIN_MATCH >= HASH_BITS

#define zmax_insert_length  max_lazy_match
// Insert new strings in the hash table only if the match length
// is not greater than this length. This saves time but degrades compression.
// max_insert_length is used only for compression levels <= 3.



const int zextra_lbits[ZLENGTH_CODES] // extra bits for each length code
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

const int zextra_dbits[ZD_CODES] // extra bits for each distance code
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

const int zextra_blbits[ZBL_CODES]// extra bits for each bit length code
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

const uChar zbl_order[ZBL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
// The lengths of the bit length codes are sent in order of decreasing
// probability, to avoid transmitting the lengths for unused bit length codes.


typedef struct zconfig {
   uShort good_length; // reduce lazy search above this match length
   uShort max_lazy;    // do not perform lazy search above this match length
   uShort nice_length; // quit search above this match length
   uShort max_chain;
} zconfig;

// Values for max_lazy_match, good_match, nice_match and max_chain_length,
// depending on the desired pack level (0..9). The values given below have
// been tuned to exclude worst case performance for pathological files.
// Better values may be found for specific files.
//

const zconfig zconfiguration_table[10] = {
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







// Data structure describing a single value and its code string.
typedef struct zct_data {
    union {
        uShort  freq;       // frequency count
        uShort  code;       // bit string
    } fc;
    union {
        uShort  dad;        // father node in Huffman tree
        uShort  len;        // length of bit string
    } dl;
} zct_data;

typedef struct ztree_desc {
    zct_data *dyn_tree;      // the dynamic tree
    zct_data *static_tree;   // corresponding static tree or NULL
    const int *extra_bits;  // extra bits for each code or NULL
    int     extra_base;     // base index for extra_bits
    int     elems;          // max number of elements in the tree
    int     max_length;     // max bit length for the codes
    int     max_code;       // largest code with non zero frequency
} ztree_desc;




class ZTTreeState
{ public:
  ZTTreeState();

  zct_data dyn_ltree[ZHEAP_SIZE];    // literal and length tree
  zct_data dyn_dtree[2*ZD_CODES+1];  // distance tree
  zct_data static_ltree[ZL_CODES+2]; // the static literal tree...
  // ... Since the bit lengths are imposed, there is no need for the L_CODES
  // extra codes used during heap construction. However the codes 286 and 287
  // are needed to build a canonical tree (see ct_init below).
  zct_data static_dtree[ZD_CODES]; // the static distance tree...
  // ... (Actually a trivial tree since all codes use 5 bits.)
  zct_data bl_tree[2*ZBL_CODES+1];  // Huffman tree for the bit lengths

  ztree_desc l_desc;
  ztree_desc d_desc;
  ztree_desc bl_desc;

  uShort bl_count[ZMAX_BITS+1];  // number of codes at each bit length for an optimal tree

  int heap[2*ZL_CODES+1]; // heap used to build the Huffman trees
  int heap_len;               // number of elements in the heap
  int heap_max;               // element of largest frequency
  // The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
  // The same heap array is used to build all trees.

  uChar depth[2*ZL_CODES+1];
  // Depth of each subtree used as tie breaker for trees of equal frequency

  uChar length_code[ZMAX_MATCH-ZMIN_MATCH+1];
  // length code for each normalized match length (0 == MIN_MATCH)

  uChar dist_code[512];
  // distance codes. The first 256 values correspond to the distances
  // 3 .. 258, the last 256 values correspond to the top 8 bits of
  // the 15 bit distances.

  int base_length[ZLENGTH_CODES];
  // First normalized length for each code (0 = MIN_MATCH)

  int base_dist[ZD_CODES];
  // First normalized distance for each code (0 = distance of 1)

  uChar l_buf[ZLIT_BUFSIZE];  // buffer for literals/lengths
  uShort d_buf[ZDIST_BUFSIZE]; // buffer for distances

  uChar flag_buf[(ZLIT_BUFSIZE/8)];
  // flag_buf is a bit array distinguishing literals from lengths in
  // l_buf, and thus indicating the presence or absence of a distance.

  unsigned last_lit;    // running index in l_buf
  unsigned last_dist;   // running index in d_buf
  unsigned last_flags;  // running index in flag_buf
  uChar flags;            // current flags not yet saved in flag_buf
  uChar flag_bit;         // current bit used in flags
  // bits are filled in flags starting at bit 0 (least significant).
  // Note: these flags are overkill in the current code since we don't
  // take advantage of DIST_BUFSIZE == LIT_BUFSIZE.

  uLong opt_len;          // bit length of current block with optimal trees
  uLong static_len;       // bit length of current block with static trees

  uLong cmpr_bytelen;     // total byte length of compressed file
  uLong cmpr_len_bits;    // number of bits past 'cmpr_bytelen'

  uLong input_len;        // total byte length of input file
  // input_len is for debugging only since we can get it by other means.

  uShort *file_type;       // pointer to UNKNOWN, BINARY or ASCII
//  int *file_method;     // pointer to DEFLATE or STORE
};

ZTTreeState::ZTTreeState()
{ ztree_desc a = {dyn_ltree, static_ltree, zextra_lbits, ZLITERALS+1, ZL_CODES, ZMAX_BITS, 0};  l_desc = a;
  ztree_desc b = {dyn_dtree, static_dtree, zextra_dbits, 0,          ZD_CODES, ZMAX_BITS, 0};  d_desc = b;
  ztree_desc c = {bl_tree, NULL,       zextra_blbits, 0,         ZBL_CODES, ZMAX_BL_BITS, 0};  bl_desc = c;
  last_lit=0;
  last_dist=0;
  last_flags=0;
}



class ZTBitState
{ public:

  int flush_flg;
  //
  unsigned bi_buf;
  // Output buffer. bits are inserted starting at the bottom (least significant
  // bits). The width of bi_buf must be at least 16 bits.
  int bi_valid;
  // Number of valid bits in bi_buf.  All bits above the last valid bit
  // are always zero.
  char *out_buf;
  // Current output buffer.
  unsigned out_offset;
  // Current offset in output buffer.
  // On 16 bit machines, the buffer is limited to 64K.
  unsigned out_size;
  // Size of current output buffer
  uLong bits_sent;   // bit length of the compressed data  only needed for debugging???
};

class ZTDeflateState
{ public:
  ZTDeflateState() {window_size=0;}

  uChar    window[2L*ZWSIZE];
  // Sliding window. Input bytes are read into the second half of the window,
  // and move to the first half later to keep a dictionary of at least WSIZE
  // bytes. With this organization, matches are limited to a distance of
  // WSIZE-MAX_MATCH bytes, but this ensures that IO is always
  // performed with a length multiple of the block size. Also, it limits
  // the window size to 64K, which is quite useful on MSDOS.
  // To do: limit the window size to WSIZE+CBSZ if SMALL_MEM (the code would
  // be less efficient since the data would have to be copied WSIZE/CBSZ times)
  Pos    prev[ZWSIZE];
  // Link to older string with same hash index. To limit the size of this
  // array to 64K, this link is maintained only for the last 32K strings.
  // An index in this array is thus a window index modulo 32K.
  Pos    head[ZHASH_SIZE];
  // Heads of the hash chains or NIL. If your compiler thinks that
  // HASH_SIZE is a dynamic value, recompile with -DDYN_ALLOC.

  uLong window_size;
  // window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
  // input file length plus MIN_LOOKAHEAD.

  long block_start;
  // window position at the beginning of the current output block. Gets
  // negative when the window is moved backwards.

  int sliding;
  // Set to false when the input file is already in memory

  unsigned ins_h;  // hash index of string to be inserted

  unsigned int prev_length;
  // Length of the best match at previous step. Matches not greater than this
  // are discarded. This is used in the lazy match evaluation.

  unsigned strstart;         // start of string to insert
  unsigned match_start; // start of matching string
  int      eofile;           // flag set at end of input file
  unsigned lookahead;        // number of valid bytes ahead in window

  unsigned max_chain_length;
  // To speed up deflation, hash chains are never searched beyond this length.
  // A higher limit improves compression ratio but degrades the speed.

  unsigned int max_lazy_match;
  // Attempt to find a better match only when the current match is strictly
  // smaller than this value. This mechanism is used only for compression
  // levels >= 4.

  unsigned good_match;
  // Use a faster search when the previous match is longer than this

  int nice_match; // Stop searching when current match exceeds this
};

typedef struct ziztimes {
  z_l_utime_t atime,mtime,ctime;
} ziztimes; // access, modify, create times

typedef struct zzlist {
  uShort vem, ver, flg, how;       // See central header in zipfile.c for what vem..off are
  uLong tim, crc, siz, len;
  fSize nam, ext, cext, com;   // offset of ext must be >= LOCHEAD
  uShort dsk, att, lflg;           // offset of lflg must be >= LOCHEAD
  uLong atx, off;
  char name[_Z_MAX_PATH];          // File name in zip file
  char *extra;                  // Extra field (set only if ext != 0)
  char *cextra;                 // Extra in central (set only if cext != 0)
  char *comment;                // Comment (set only if com != 0)
  char iname[_Z_MAX_PATH];         // Internal file name after cleanup
  char zname[_Z_MAX_PATH];         // External version of internal name
  int mark;                     // Marker for files to operate on
  int trash;                    // Marker for files to delete
  int dosflag;                  // Set to force MSDOS file attributes
  struct zzlist *nxt;        // Pointer to next header in list
} ZTZipFileInfo;


struct ZTState;
typedef unsigned (*ZREADFUNC)(ZTState &state, char *buf,unsigned size);
typedef unsigned (*ZFLUSHFUNC)(void *param, const char *buf, unsigned *size);
typedef unsigned (*ZWRITEFUNC)(void *param, const char *buf, unsigned size);
struct ZTState
{ void *param;
  int level; bool seekable;
  ZREADFUNC readfunc; ZFLUSHFUNC flush_outbuf;
  ZTTreeState ts; ZTBitState bs; ZTDeflateState ds;
  const char *err;
};




// ----------------------------------------------------------------------
// some windows<->linux portability things
#ifdef ZIP_STD
void zfiletime2dosdatetime(const ZFILETIME ft, ZWORD *dosdate, ZWORD *dostime)
{ struct tm *st=gmtime(&ft);
  *dosdate = (uShort)(((st->tm_year+1900 -1980)&0x7f) << 9);
  *dosdate |= (uShort)((st->tm_mon&0xf) << 5);
  *dosdate |= (uShort)((st->tm_mday&0x1f));
  *dostime = (uShort)((st->tm_hour&0x1f) << 11);
  *dostime |= (uShort)((st->tm_min&0x3f) << 5);
  *dostime |= (uShort)((st->tm_sec*2)&0x1f);
}

void ZGetNow(z_l_utime_t *ft, ZWORD *dosdate, ZWORD *dostime)
{ time_t tm = time(0);
  zfiletime2dosdatetime(tm,dosdate,dostime);
  *ft = (z_l_utime_t)tm;
}

ZDWORD ZGetFilePosZ(ZHANDLE hfout)
{ struct stat st; fstat(fileno(hfout),&st);
  if ((st.st_mode&S_IFREG)==0) return 0xFFFFFFFF;
  return ftell(hfout);
}

ZZRESULT ZGetFileInfo(FILE *hf, uLong *attr, long *size, ziztimes *times, uLong *timestamp)
{ // The handle must be a handle to a file
  // The date and time is returned in a long with the date most significant to allow
  // unsigned integer comparison of absolute times. The attributes have two
  // high bytes unix attr, and two low bytes a mapping of that to DOS attr.
  struct stat bhi; int res=fstat(fileno(hf),&bhi); if (res==-1) return ZZR_NOFILE;
  uLong fa=bhi.st_mode; uLong a=0;
  // Zip uses the lower word for its interpretation of windows stuff
  if ((fa&S_IWUSR)==0) a|=0x01;
  if (S_ISDIR(fa)) a|=0x10;
  // It uses the upper word for standard unix attr
  a |= ((fa&0xFFFF)<<16);
  //
  if (attr!=NULL) *attr = a;
  if (size!=NULL) *size = bhi.st_size;
  if (times!=NULL)
  { times->atime = (z_l_utime_t)bhi.st_atime;
    times->mtime = (z_l_utime_t)bhi.st_mtime;
    times->ctime = (z_l_utime_t)bhi.st_ctime;
  }
  if (timestamp!=NULL)
  { uShort dosdate,dostime;
    zfiletime2dosdatetime(bhi.st_mtime,&dosdate,&dostime);
    *timestamp = (uShort)dostime | (((uLong)dosdate)<<16);
  }
  return ZZR_OK;
}


// ----------------------------------------------------------------------
#else
void zfiletime2dosdatetime(const ZFILETIME ft, ZWORD *dosdate,ZWORD *dostime)
{ // date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
  // time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
  SYSTEMTIME st; FileTimeToSystemTime(&ft,&st);
  *dosdate = (ZWORD)(((st.wYear-1980)&0x7f) << 9);
  *dosdate |= (ZWORD)((st.wMonth&0xf) << 5);
  *dosdate |= (ZWORD)((st.wDay&0x1f));
  *dostime = (ZWORD)((st.wHour&0x1f) << 11);
  *dostime |= (ZWORD)((st.wMinute&0x3f) << 5);
  *dostime |= (ZWORD)((st.wSecond*2)&0x1f);
}

z_l_utime_t zfiletime2timet(const ZFILETIME ft)
{ LONGLONG i = *(LONGLONG*)&ft;
  return (z_l_utime_t)((i-116444736000000000LL)/10000000LL);
}

void ZGetNow(z_l_utime_t *pft, ZWORD *dosdate, ZWORD *dostime)
{ SYSTEMTIME st; GetLocalTime(&st);
  ZFILETIME ft;   SystemTimeToFileTime(&st,&ft);
  zfiletime2dosdatetime(ft,dosdate,dostime);
  *pft = zfiletime2timet(ft);
}

ZDWORD ZGetFilePosZ(ZHANDLE hfout)
{ return SetFilePointer(hfout,0,0,FILE_CURRENT);
}


ZZRESULT ZGetFileInfo(ZHANDLE hf, uLong *attrs, long *lsize, ziztimes *times, uLong *timestamp)
{ // The handle must be a handle to a file
  // The date and time is returned in a long with the date most significant to allow
  // unsigned integer comparison of absolute times. The attributes have two
  // high bytes unix attr, and two low bytes a mapping of that to DOS attr.
  //struct stat s; int res=stat(fn,&s); if (res!=0) return false;
  // translate windows file attributes into zip ones.
  BY_HANDLE_FILE_INFORMATION bhi; BOOL res=GetFileInformationByHandle(hf,&bhi);
  if (!res) return ZZR_NOFILE;
  ZDWORD fa=bhi.dwFileAttributes; uLong a=0;
  // Zip uses the lower word for its interpretation of windows stuff
  if (fa&FILE_ATTRIBUTE_READONLY) a|=0x01;
  if (fa&FILE_ATTRIBUTE_HIDDEN)   a|=0x02;
  if (fa&FILE_ATTRIBUTE_SYSTEM)   a|=0x04;
  if (fa&FILE_ATTRIBUTE_DIRECTORY)a|=0x10;
  if (fa&FILE_ATTRIBUTE_ARCHIVE)  a|=0x20;
  // It uses the upper word for standard unix attr, which we manually construct
  if (fa&FILE_ATTRIBUTE_DIRECTORY)a|=0x40000000;  // directory
  else a|=0x80000000;  // normal file
  a|=0x01000000;      // readable
  if (fa&FILE_ATTRIBUTE_READONLY) {} else a|=0x00800000; // writeable
  // now just a small heuristic to check if it's an executable:
  ZDWORD red, hsize=GetFileSize(hf,NULL); if (hsize>40)
  { SetFilePointer(hf,0,NULL,FILE_BEGIN); unsigned short magic; ReadFile(hf,&magic,sizeof(magic),&red,NULL);
    SetFilePointer(hf,36,NULL,FILE_BEGIN); unsigned long hpos;  ReadFile(hf,&hpos,sizeof(hpos),&red,NULL);
    if (magic==0x54AD && hsize>hpos+4+20+28)
    { SetFilePointer(hf,hpos,NULL,FILE_BEGIN); unsigned long signature; ReadFile(hf,&signature,sizeof(signature),&red,NULL);
      if (signature==IMAGE_DOS_SIGNATURE || signature==IMAGE_OS2_SIGNATURE
         || signature==IMAGE_OS2_SIGNATURE_LE || signature==IMAGE_NT_SIGNATURE)
      { a |= 0x00400000; // executable
      }
    }
  }
  //
  if (attrs!=NULL) *attrs = a;
  if (lsize!=NULL) *lsize = hsize;
  if (times!=NULL)
  { // lutime_t is 32bit number of seconds elapsed since 0:0:0GMT, Jan1, 1970.
    // but FILETIME is 64bit number of 100-nanosecs since Jan1, 1601
    times->atime = zfiletime2timet(bhi.ftLastAccessTime);
    times->mtime = zfiletime2timet(bhi.ftLastWriteTime);
    times->ctime = zfiletime2timet(bhi.ftCreationTime);
  }
  if (timestamp!=NULL)
  { ZWORD dosdate,dostime;
    zfiletime2dosdatetime(bhi.ftLastWriteTime,&dosdate,&dostime);
    *timestamp = (ZWORD)dostime | (((ZDWORD)dosdate)<<16);
  }
  return ZZR_OK;
}
#endif
// ----------------------------------------------------------------------





void ZAssert(ZTState &state,bool cond, const char *msg)
{ if (cond) return;
  state.err=msg;
}
void ZTrace(const char *x, ...) {va_list paramList; va_start(paramList, x); paramList; va_end(paramList);}
void ZTracec(bool ,const char *x, ...) {va_list paramList; va_start(paramList, x); paramList; va_end(paramList);}



// ===========================================================================
// Local (static) routines in this file.
//

void zinit_block     (ZTState &);
void zpqdownheap     (ZTState &,zct_data *tree, int k);
void zgen_bitlen     (ZTState &,ztree_desc *desc);
void zgen_codes      (ZTState &state,zct_data *tree, int max_code);
void zbuild_tree     (ZTState &,ztree_desc *desc);
void zscan_tree      (ZTState &,zct_data *tree, int max_code);
void zsend_tree      (ZTState &state,zct_data *tree, int max_code);
int  zbuild_bl_tree  (ZTState &);
void zsend_all_trees (ZTState &state,int lcodes, int dcodes, int blcodes);
void zcompress_block (ZTState &state,zct_data *ltree, zct_data *dtree);
void zset_file_type  (ZTState &);
void zsend_bits      (ZTState &state, int value, int length);
unsigned zbi_reverse (unsigned code, int len);
void zbi_windup      (ZTState &state);
void zcopy_block     (ZTState &state,char *buf, unsigned len, int header);


#define zsend_code(state, c, tree) zsend_bits(state, tree[c].fc.code, tree[c].dl.len)
// Send a code of the given tree. c and tree must not have side effects

// alternatively...
//#define send_code(state, c, tree)
//     { if (state.verbose>1) fprintf(stderr,"\ncd %3d ",(c));
//       send_bits(state, tree[c].fc.code, tree[c].dl.len); }

#define zd_code(dist) ((dist) < 256 ? state.ts.dist_code[dist] : state.ts.dist_code[256+((dist)>>7)])
// Mapping from a distance to a distance code. dist is the distance - 1 and
// must not have side effects. dist_code[256] and dist_code[257] are never used.

#define ZMax(a,b) (a >= b ? a : b)
/* the arguments must not have side effects */

/* ===========================================================================
 * Allocate the match buffer, initialize the various tables and save the
 * location of the internal file attribute (ascii/binary) and method
 * (DEFLATE/STORE).
 */
void zct_init(ZTState &state, uShort *attr)
{
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */

    state.ts.file_type = attr;
    //state.ts.file_method = method;
    state.ts.cmpr_bytelen = state.ts.cmpr_len_bits = 0L;
    state.ts.input_len = 0L;

    if (state.ts.static_dtree[0].dl.len != 0) return; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < ZLENGTH_CODES-1; code++) {
        state.ts.base_length[code] = length;
        for (n = 0; n < (1<<zextra_lbits[code]); n++) {
            state.ts.length_code[length++] = (uChar)code;
        }
    }
    ZAssert(state,length == 256, "ct_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    state.ts.length_code[length-1] = (uChar)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
        state.ts.base_dist[code] = dist;
        for (n = 0; n < (1<<zextra_dbits[code]); n++) {
            state.ts.dist_code[dist++] = (uChar)code;
        }
    }
    ZAssert(state,dist == 256, "ct_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < ZD_CODES; code++) {
        state.ts.base_dist[code] = dist << 7;
        for (n = 0; n < (1<<(zextra_dbits[code]-7)); n++) {
            state.ts.dist_code[256 + dist++] = (uChar)code;
        }
    }
    ZAssert(state,dist == 256, "ct_init: 256+dist != 512");
	
    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= ZMAX_BITS; bits++) state.ts.bl_count[bits] = 0;
    n = 0;
    while (n <= 143) state.ts.static_ltree[n++].dl.len = 8, state.ts.bl_count[8]++;
    while (n <= 255) state.ts.static_ltree[n++].dl.len = 9, state.ts.bl_count[9]++;
    while (n <= 279) state.ts.static_ltree[n++].dl.len = 7, state.ts.bl_count[7]++;
    while (n <= 287) state.ts.static_ltree[n++].dl.len = 8, state.ts.bl_count[8]++;
    /* fc.codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    zgen_codes(state,(zct_data *)state.ts.static_ltree, ZL_CODES+1);

    /* The static distance tree is trivial: */
    for (n = 0; n < ZD_CODES; n++) {
        state.ts.static_dtree[n].dl.len = 5;
        state.ts.static_dtree[n].fc.code = (uShort)zbi_reverse(n, 5);
    }

    /* Initialize the first block of the first file: */
    zinit_block(state);
}

/* ===========================================================================
 * Initialize a new block.
 */
void zinit_block(ZTState &state)
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < ZL_CODES;  n++) state.ts.dyn_ltree[n].fc.freq = 0;
    for (n = 0; n < ZD_CODES;  n++) state.ts.dyn_dtree[n].fc.freq = 0;
    for (n = 0; n < ZBL_CODES; n++) state.ts.bl_tree[n].fc.freq = 0;

    state.ts.dyn_ltree[ZEND_BLOCK].fc.freq = 1;
    state.ts.opt_len = state.ts.static_len = 0L;
    state.ts.last_lit = state.ts.last_dist = state.ts.last_flags = 0;
    state.ts.flags = 0; state.ts.flag_bit = 1;
}

#define ZSMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define zpqremove(tree, top) \
{\
    top = state.ts.heap[ZSMALLEST]; \
    state.ts.heap[ZSMALLEST] = state.ts.heap[state.ts.heap_len--]; \
    zpqdownheap(state,tree, ZSMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define zsmaller(tree, n, m) \
   (tree[n].fc.freq < tree[m].fc.freq || \
   (tree[n].fc.freq == tree[m].fc.freq && state.ts.depth[n] <= state.ts.depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
void zpqdownheap(ZTState &state,zct_data *tree, int k)
{
    int v = state.ts.heap[k];
    int j = k << 1;  /* left son of k */
    int htemp;       /* required because of bug in SASC compiler */

    while (j <= state.ts.heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < state.ts.heap_len && zsmaller(tree, state.ts.heap[j+1], state.ts.heap[j])) j++;

        /* Exit if v is smaller than both sons */
        htemp = state.ts.heap[j];
        if (zsmaller(tree, v, htemp)) break;

        /* Exchange v with the smallest son */
        state.ts.heap[k] = htemp;
        k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    state.ts.heap[k] = v;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
void zgen_bitlen(ZTState &state,ztree_desc *desc)
{
    zct_data *tree  = desc->dyn_tree;
    const int *extra     = desc->extra_bits;
    int base            = desc->extra_base;
    int max_code        = desc->max_code;
    int max_length      = desc->max_length;
    zct_data *stree = desc->static_tree;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    uShort f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= ZMAX_BITS; bits++) state.ts.bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[state.ts.heap[state.ts.heap_max]].dl.len = 0; /* root of the heap */

    for (h = state.ts.heap_max+1; h < ZHEAP_SIZE; h++) {
        n = state.ts.heap[h];
        bits = tree[tree[n].dl.dad].dl.len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].dl.len = (uShort)bits;
        /* We overwrite tree[n].dl.dad which is no longer needed */

        if (n > max_code) continue; /* not a leaf node */

        state.ts.bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n-base];
        f = tree[n].fc.freq;
        state.ts.opt_len += (uLong)f * (bits + xbits);
        if (stree) state.ts.static_len += (uLong)f * (stree[n].dl.len + xbits);
    }
    if (overflow == 0) return;

    ZTrace("\nbit length overflow\n");
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (state.ts.bl_count[bits] == 0) bits--;
        state.ts.bl_count[bits]--;           /* move one leaf down the tree */
        state.ts.bl_count[bits+1] += (uShort)2; /* move one overflow item as its brother */
        state.ts.bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = state.ts.bl_count[bits];
        while (n != 0) {
            m = state.ts.heap[--h];
            if (m > max_code) continue;
            if (tree[m].dl.len != (uShort)bits) {
                ZTrace("code %d bits %d->%d\n", m, tree[m].dl.len, bits);
                state.ts.opt_len += ((long)bits-(long)tree[m].dl.len)*(long)tree[m].fc.freq;
                tree[m].dl.len = (uShort)bits;
            }
            n--;
        }
    }
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
void zgen_codes (ZTState &state, zct_data *tree, int max_code)
{
    uShort next_code[ZMAX_BITS+1]; /* next code value for each bit length */
    uShort code = 0;              /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= ZMAX_BITS; bits++) {
        next_code[bits] = code = (uShort)((code + state.ts.bl_count[bits-1]) << 1);
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    ZAssert(state,code + state.ts.bl_count[ZMAX_BITS]-1 == (1<< ((uShort) ZMAX_BITS)) - 1,
            "inconsistent bit counts");
    ZTrace("\ngen_codes: max_code %d ", max_code);

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].dl.len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].fc.code = (uShort)zbi_reverse(next_code[len]++, len);

        //Tracec(tree != state.ts.static_ltree, "\nn %3d %c l %2d c %4x (%x) ", n, (isgraph(n) ? n : ' '), len, tree[n].fc.code, next_code[len]-1);
    }
}

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
void zbuild_tree(ZTState &state,ztree_desc *desc)
{
    zct_data *tree   = desc->dyn_tree;
    zct_data *stree  = desc->static_tree;
    int elems            = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    state.ts.heap_len = 0, state.ts.heap_max = ZHEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].fc.freq != 0) {
            state.ts.heap[++state.ts.heap_len] = max_code = n;
            state.ts.depth[n] = 0;
        } else {
            tree[n].dl.len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (state.ts.heap_len < 2) {
        int newcp = state.ts.heap[++state.ts.heap_len] = (max_code < 2 ? ++max_code : 0);
        tree[newcp].fc.freq = 1;
        state.ts.depth[newcp] = 0;
        state.ts.opt_len--; if (stree) state.ts.static_len -= stree[newcp].dl.len;
        /* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = state.ts.heap_len/2; n >= 1; n--) zpqdownheap(state,tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
        zpqremove(tree, n);   /* n = node of least frequency */
        m = state.ts.heap[ZSMALLEST];  /* m = node of next least frequency */

        state.ts.heap[--state.ts.heap_max] = n; /* keep the nodes sorted by frequency */
        state.ts.heap[--state.ts.heap_max] = m;

        /* Create a new node father of n and m */
        tree[node].fc.freq = (uShort)(tree[n].fc.freq + tree[m].fc.freq);
        state.ts.depth[node] = (uChar) (ZMax(state.ts.depth[n], state.ts.depth[m]) + 1);
        tree[n].dl.dad = tree[m].dl.dad = (uShort)node;
        /* and insert the new node in the heap */
        state.ts.heap[ZSMALLEST] = node++;
        zpqdownheap(state,tree, ZSMALLEST);

    } while (state.ts.heap_len >= 2);

    state.ts.heap[--state.ts.heap_max] = state.ts.heap[ZSMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    zgen_bitlen(state,(ztree_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    zgen_codes (state,(zct_data *)tree, max_code);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
void zscan_tree (ZTState &state,zct_data *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].dl.len = (uShort)-1; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].dl.len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            state.ts.bl_tree[curlen].fc.freq = (uShort)(state.ts.bl_tree[curlen].fc.freq + count);
        } else if (curlen != 0) {
            if (curlen != prevlen) state.ts.bl_tree[curlen].fc.freq++;
            state.ts.bl_tree[ZREP_3_6].fc.freq++;
        } else if (count <= 10) {
            state.ts.bl_tree[ZREPZ_3_10].fc.freq++;
        } else {
            state.ts.bl_tree[ZREPZ_11_138].fc.freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
void zsend_tree (ZTState &state, zct_data *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].dl.len = -1; */  /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].dl.len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { zsend_code(state, curlen, state.ts.bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                zsend_code(state, curlen, state.ts.bl_tree); count--;
            }
            ZAssert(state,count >= 3 && count <= 6, " 3_6?");
            zsend_code(state,ZREP_3_6, state.ts.bl_tree); zsend_bits(state,count-3, 2);

        } else if (count <= 10) {
            zsend_code(state,ZREPZ_3_10, state.ts.bl_tree); zsend_bits(state,count-3, 3);

        } else {
            zsend_code(state,ZREPZ_11_138, state.ts.bl_tree); zsend_bits(state,count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
int build_bl_tree(ZTState &state)
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    zscan_tree(state,(zct_data *)state.ts.dyn_ltree, state.ts.l_desc.max_code);
    zscan_tree(state,(zct_data *)state.ts.dyn_dtree, state.ts.d_desc.max_code);

    /* Build the bit length tree: */
    zbuild_tree(state,(ztree_desc *)(&state.ts.bl_desc));
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = ZBL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (state.ts.bl_tree[zbl_order[max_blindex]].dl.len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    state.ts.opt_len += 3*(max_blindex+1) + 5+5+4;
    ZTrace("\ndyn trees: dyn %ld, stat %ld", state.ts.opt_len, state.ts.static_len);

    return max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
void zsend_all_trees(ZTState &state,int lcodes, int dcodes, int blcodes)
{
    int rank;                    /* index in bl_order */

    ZAssert(state,lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    ZAssert(state,lcodes <= ZL_CODES && dcodes <= ZD_CODES && blcodes <= ZBL_CODES,
            "too many codes");
    ZTrace("\nbl counts: ");
    zsend_bits(state,lcodes-257, 5);
    /* not +255 as stated in appnote.txt 1.93a or -256 in 2.04c */
    zsend_bits(state,dcodes-1,   5);
    zsend_bits(state,blcodes-4,  4); /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        ZTrace("\nbl code %2d ", zbl_order[rank]);
        zsend_bits(state,state.ts.bl_tree[zbl_order[rank]].dl.len, 3);
    }
    ZTrace("\nbl tree: sent %ld", state.bs.bits_sent);

    zsend_tree(state,(zct_data *)state.ts.dyn_ltree, lcodes-1); /* send the literal tree */
    ZTrace("\nlit tree: sent %ld", state.bs.bits_sent);

    zsend_tree(state,(zct_data *)state.ts.dyn_dtree, dcodes-1); /* send the distance tree */
    ZTrace("\ndist tree: sent %ld", state.bs.bits_sent);
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length (in bytes) for the file so far.
 */
uLong zflush_block(ZTState &state,char *buf, uLong stored_len, int eof)
{
    uLong opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

    state.ts.flag_buf[state.ts.last_flags] = state.ts.flags; /* Save the flags for the last 8 items */

     /* Check if the file is ascii or binary */
    if (*state.ts.file_type == (uShort)ZUNKNOWN) zset_file_type(state);

    /* Construct the literal and distance trees */
    zbuild_tree(state,(ztree_desc *)(&state.ts.l_desc));
    ZTrace("\nlit data: dyn %ld, stat %ld", state.ts.opt_len, state.ts.static_len);

    zbuild_tree(state,(ztree_desc *)(&state.ts.d_desc));
    ZTrace("\ndist data: dyn %ld, stat %ld", state.ts.opt_len, state.ts.static_len);
    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations.
     */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send.
     */
    max_blindex = build_bl_tree(state);

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (state.ts.opt_len+3+7)>>3;
    static_lenb = (state.ts.static_len+3+7)>>3;
    state.ts.input_len += stored_len; /* for debugging only */

    ZTrace("\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ",
            opt_lenb, state.ts.opt_len, static_lenb, state.ts.static_len, stored_len,
            state.ts.last_lit, state.ts.last_dist);

    if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

    // Originally, zip allowed the file to be transformed from a compressed
    // into a stored file in the case where compression failed, there
    // was only one block, and it was allowed to change. I've removed this
    // possibility since the code's cleaner if no changes are allowed.
    //if (stored_len <= opt_lenb && eof && state.ts.cmpr_bytelen == 0L
    //   && state.ts.cmpr_len_bits == 0L && state.seekable)
    //{   // && state.ts.file_method != NULL
    //    // Since LIT_BUFSIZE <= 2*WSIZE, the input data must be there:
    //    Assert(state,buf!=NULL,"block vanished");
    //    copy_block(state,buf, (unsigned)stored_len, 0); // without header
    //    state.ts.cmpr_bytelen = stored_len;
    //    Assert(state,false,"unimplemented *state.ts.file_method = STORE;");
    //    //*state.ts.file_method = STORE;
    //}
    //else
    if (stored_len+4 <= opt_lenb && buf != (char*)NULL) {
                       /* 4: two words for the lengths */
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        zsend_bits(state,(ZSTORED_BLOCK<<1)+eof, 3);  /* send block type */
        state.ts.cmpr_bytelen += ((state.ts.cmpr_len_bits + 3 + 7) >> 3) + stored_len + 4;
        state.ts.cmpr_len_bits = 0L;

        zcopy_block(state,buf, (unsigned)stored_len, 1); /* with header */
    }
    else if (static_lenb == opt_lenb) {
        zsend_bits(state,(ZSTATIC_TREES<<1)+eof, 3);
        zcompress_block(state,(zct_data *)state.ts.static_ltree, (zct_data *)state.ts.static_dtree);
        state.ts.cmpr_len_bits += 3 + state.ts.static_len;
        state.ts.cmpr_bytelen += state.ts.cmpr_len_bits >> 3;
        state.ts.cmpr_len_bits &= 7L;
    }
    else {
        zsend_bits(state,(ZDYN_TREES<<1)+eof, 3);
        zsend_all_trees(state,state.ts.l_desc.max_code+1, state.ts.d_desc.max_code+1, max_blindex+1);
        zcompress_block(state,(zct_data *)state.ts.dyn_ltree, (zct_data *)state.ts.dyn_dtree);
        state.ts.cmpr_len_bits += 3 + state.ts.opt_len;
        state.ts.cmpr_bytelen += state.ts.cmpr_len_bits >> 3;
        state.ts.cmpr_len_bits &= 7L;
    }
    ZAssert(state,((state.ts.cmpr_bytelen << 3) + state.ts.cmpr_len_bits) == state.bs.bits_sent, "bad compressed size");
    zinit_block(state);

    if (eof) {
        // Assert(state,input_len == isize, "bad input size");
        zbi_windup(state);
        state.ts.cmpr_len_bits += 7;  /* align on byte boundary */
    }
    ZTrace("\n");

    return state.ts.cmpr_bytelen + (state.ts.cmpr_len_bits >> 3);
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
int zct_tally (ZTState &state,int dist, int lc)
{
    state.ts.l_buf[state.ts.last_lit++] = (uChar)lc;
    if (dist == 0) {
        /* lc is the unmatched char */
        state.ts.dyn_ltree[lc].fc.freq++;
    } else {
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        ZAssert(state,(uShort)dist < (uShort)ZMAX_DIST &&
               (uShort)lc <= (uShort)(ZMAX_MATCH-ZMIN_MATCH) &&
               (uShort)zd_code(dist) < (uShort)ZD_CODES,  "ct_tally: bad match");

        state.ts.dyn_ltree[state.ts.length_code[lc]+ZLITERALS+1].fc.freq++;
        state.ts.dyn_dtree[zd_code(dist)].fc.freq++;

        state.ts.d_buf[state.ts.last_dist++] = (uShort)dist;
        state.ts.flags |= state.ts.flag_bit;
    }
    state.ts.flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((state.ts.last_lit & 7) == 0) {
        state.ts.flag_buf[state.ts.last_flags++] = state.ts.flags;
        state.ts.flags = 0, state.ts.flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if (state.level > 2 && (state.ts.last_lit & 0xfff) == 0) {
        /* Compute an upper bound for the compressed length */
        uLong out_length = (uLong)state.ts.last_lit*8L;
        uLong in_length = (uLong)state.ds.strstart-state.ds.block_start;
        int dcode;
        for (dcode = 0; dcode < ZD_CODES; dcode++) {
            out_length += (uLong)state.ts.dyn_dtree[dcode].fc.freq*(5L+zextra_dbits[dcode]);
        }
        out_length >>= 3;
        ZTrace("\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ",
               state.ts.last_lit, state.ts.last_dist, in_length, out_length,
               100L - out_length*100L/in_length);
        if (state.ts.last_dist < state.ts.last_lit/2 && out_length < in_length/2) return 1;
    }
    return (state.ts.last_lit == ZLIT_BUFSIZE-1 || state.ts.last_dist == ZDIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
void zcompress_block(ZTState &state,zct_data *ltree, zct_data *dtree)
{
    unsigned dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    unsigned lx = 0;    /* running index in l_buf */
    unsigned dx = 0;    /* running index in d_buf */
    unsigned fx = 0;    /* running index in flag_buf */
    uChar flag = 0;       /* current flags */
    unsigned code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (state.ts.last_lit != 0) do {
        if ((lx & 7) == 0) flag = state.ts.flag_buf[fx++];
        lc = state.ts.l_buf[lx++];
        if ((flag & 1) == 0) {
            zsend_code(state,lc, ltree); /* send a literal byte */
        } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = state.ts.length_code[lc];
            zsend_code(state,code+ZLITERALS+1, ltree); /* send the length code */
            extra = zextra_lbits[code];
            if (extra != 0) {
                lc -= state.ts.base_length[code];
                zsend_bits(state,lc, extra);        /* send the extra length bits */
            }
            dist = state.ts.d_buf[dx++];
            /* Here, dist is the match distance - 1 */
            code = zd_code(dist);
            ZAssert(state,code < ZD_CODES, "bad d_code");

            zsend_code(state,code, dtree);       /* send the distance code */
            extra = zextra_dbits[code];
            if (extra != 0) {
                dist -= state.ts.base_dist[code];
                zsend_bits(state,dist, extra);   /* send the extra distance bits */
            }
        } /* literal or match pair ? */
        flag >>= 1;
    } while (lx < state.ts.last_lit);

    zsend_code(state,ZEND_BLOCK, ltree);
}

/* ===========================================================================
 * Set the file type to ASCII or BINARY, using a crude approximation:
 * binary if more than 20% of the bytes are <= 6 or >= 128, ascii otherwise.
 * IN assertion: the fields freq of dyn_ltree are set and the total of all
 * frequencies does not exceed 64K (to fit in an int on 16 bit machines).
 */
void zset_file_type(ZTState &state)
{
    int n = 0;
    unsigned ascii_freq = 0;
    unsigned bin_freq = 0;
    while (n < 7)        bin_freq += state.ts.dyn_ltree[n++].fc.freq;
    while (n < 128)    ascii_freq += state.ts.dyn_ltree[n++].fc.freq;
    while (n < ZLITERALS) bin_freq += state.ts.dyn_ltree[n++].fc.freq;
    *state.ts.file_type = (uShort)(bin_freq > (ascii_freq >> 2) ? ZBINARY : ZASCII);
}


/* ===========================================================================
 * Initialize the bit string routines.
 */
void zbi_init (ZTState &state,char *tgt_buf, unsigned tgt_size, int flsh_allowed)
{
    state.bs.out_buf = tgt_buf;
    state.bs.out_size = tgt_size;
    state.bs.out_offset = 0;
    state.bs.flush_flg = flsh_allowed;

    state.bs.bi_buf = 0;
    state.bs.bi_valid = 0;
    state.bs.bits_sent = 0L;
}

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
void zsend_bits(ZTState &state,int value, int length)
{
    ZAssert(state,length > 0 && length <= 15, "invalid length");
    state.bs.bits_sent += (uLong)length;
    /* If not enough room in bi_buf, use (bi_valid) bits from bi_buf and
     * (Buf_size - bi_valid) bits from value to flush the filled bi_buf,
     * then fill in the rest of (value), leaving (length - (Buf_size-bi_valid))
     * unused bits in bi_buf.
     */
    state.bs.bi_buf |= (value << state.bs.bi_valid);
    state.bs.bi_valid += length;
    if (state.bs.bi_valid > (int)ZBuf_size) {
        ZPUTSHORT(state,state.bs.bi_buf);
        state.bs.bi_valid -= ZBuf_size;
        state.bs.bi_buf = (unsigned)value >> (length - state.bs.bi_valid);
    }
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
unsigned zbi_reverse(unsigned code, int len)
{
    register unsigned res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* ===========================================================================
 * Write out any remaining bits in an incomplete byte.
 */
void zbi_windup(ZTState &state)
{
    if (state.bs.bi_valid > 8) {
        ZPUTSHORT(state,state.bs.bi_buf);
    } else if (state.bs.bi_valid > 0) {
        ZPUTBYTE(state,state.bs.bi_buf);
    }
    if (state.bs.flush_flg) {
        state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset);
    }
    state.bs.bi_buf = 0;
    state.bs.bi_valid = 0;
    state.bs.bits_sent = (state.bs.bits_sent+7) & ~7;
}

/* ===========================================================================
 * Copy a stored block to the zip file, storing first the length and its
 * one's complement if requested.
 */
void zcopy_block(ZTState &state, char *block, unsigned len, int header)
{
    zbi_windup(state);              /* align on byte boundary */

    if (header) {
        ZPUTSHORT(state,(uShort)len);
        ZPUTSHORT(state,(uShort)~len);
        state.bs.bits_sent += 2*16;
    }
    if (state.bs.flush_flg) {
        state.flush_outbuf(state.param,state.bs.out_buf, &state.bs.out_offset);
        state.bs.out_offset = len;
        state.flush_outbuf(state.param,block, &state.bs.out_offset);
    } else if (state.bs.out_offset + len > state.bs.out_size) {
        ZAssert(state,false,"output buffer too small for in-memory compression");
    } else {
        memcpy(state.bs.out_buf + state.bs.out_offset, block, len);
        state.bs.out_offset += len;
    }
    state.bs.bits_sent += (uLong)len<<3;
}

/* ===========================================================================
 *  Prototypes for functions.
 */

void zfill_window  (ZTState &state);
uLong zdeflate_fast  (ZTState &state);

int  zlongest_match (ZTState &state,IPos cur_match);


/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define ZUPDATE_HASH(h,c) (h = (((h)<<ZH_SHIFT) ^ (c)) & ZHASH_MASK)

/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define ZINSERT_STRING(s, match_head) \
   (ZUPDATE_HASH(state.ds.ins_h, state.ds.window[(s) + (ZMIN_MATCH-1)]), \
    state.ds.prev[(s) & ZWMASK] = match_head = state.ds.head[state.ds.ins_h], \
    state.ds.head[state.ds.ins_h] = (s))

/* ===========================================================================
 * Initialize the "longest match" routines for a new file
 *
 * IN assertion: window_size is > 0 if the input file is already read or
 *    mmap'ed in the window[] array, 0 otherwise. In the first case,
 *    window_size is sufficient to contain the whole input file plus
 *    MIN_LOOKAHEAD bytes (to avoid referencing memory beyond the end
 *    of window[] when looking for matches towards the end).
 */
void zlm_init (ZTState &state, int pack_level, uShort *flags)
{
    register unsigned j;

    ZAssert(state,pack_level>=1 && pack_level<=8,"bad pack level");

    /* Do not slide the window if the whole input is already in memory
     * (window_size > 0)
     */
    state.ds.sliding = 0;
    if (state.ds.window_size == 0L) {
        state.ds.sliding = 1;
        state.ds.window_size = (uLong)2L*ZWSIZE;
    }

    /* Initialize the hash table (avoiding 64K overflow for 16 bit systems).
     * prev[] will be initialized on the fly.
     */
    state.ds.head[ZHASH_SIZE-1] = ZNIL;
    memset((char*)state.ds.head, ZNIL, (unsigned)(ZHASH_SIZE-1)*sizeof(*state.ds.head));

    /* Set the default configuration parameters:
     */
    state.ds.max_lazy_match   = zconfiguration_table[pack_level].max_lazy;
    state.ds.good_match       = zconfiguration_table[pack_level].good_length;
    state.ds.nice_match       = zconfiguration_table[pack_level].nice_length;
    state.ds.max_chain_length = zconfiguration_table[pack_level].max_chain;
    if (pack_level <= 2) {
       *flags |= ZFAST;
    } else if (pack_level >= 8) {
       *flags |= ZSLOW;
    }
    /* ??? reduce max_chain_length for binary files */

    state.ds.strstart = 0;
    state.ds.block_start = 0L;

    j = ZWSIZE;
    j <<= 1; // Can read 64K in one step
    state.ds.lookahead = state.readfunc(state, (char*)state.ds.window, j);

    if (state.ds.lookahead == 0 || state.ds.lookahead == (unsigned)EOF) {
       state.ds.eofile = 1, state.ds.lookahead = 0;
       return;
    }
    state.ds.eofile = 0;
    /* Make sure that we always have enough lookahead. This is important
     * if input comes from a device such as a tty.
     */
    if (state.ds.lookahead < ZMIN_LOOKAHEAD) zfill_window(state);

    state.ds.ins_h = 0;
    for (j=0; j<ZMIN_MATCH-1; j++) ZUPDATE_HASH(state.ds.ins_h, state.ds.window[j]);
    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
}


/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
// For 80x86 and 680x0 and ARM, an optimized version is in match.asm or
// match.S. The code is functionally equivalent, so you can use the C version
// if desired. Which I do so desire!
int zlongest_match(ZTState &state,IPos cur_match)
{
    unsigned chain_length = state.ds.max_chain_length;   /* max hash chain length */
    register uChar *scan = state.ds.window + state.ds.strstart; /* current string */
    register uChar *match;                    /* matched string */
    register int len;                           /* length of current match */
    int best_len = state.ds.prev_length;                 /* best match length so far */
    IPos limit = state.ds.strstart > (IPos)ZMAX_DIST ? state.ds.strstart - (IPos)ZMAX_DIST : ZNIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */

  // The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
  // It is easy to get rid of this optimization if necessary.
    ZAssert(state,ZHASH_BITS>=8 && ZMAX_MATCH==258,"Code too clever");



    register uChar *strend = state.ds.window + state.ds.strstart + ZMAX_MATCH;
    register uChar scan_end1  = scan[best_len-1];
    register uChar scan_end   = scan[best_len];

    /* Do not waste too much time if we already have a good match: */
    if (state.ds.prev_length >= state.ds.good_match) {
        chain_length >>= 2;
    }

    ZAssert(state,state.ds.strstart <= state.ds.window_size-ZMIN_LOOKAHEAD, "insufficient lookahead");

    do {
        ZAssert(state,cur_match < state.ds.strstart, "no future");
        match = state.ds.window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */
        if (match[best_len]   != scan_end  ||
            match[best_len-1] != scan_end1 ||
            *match            != *scan     ||
            *++match          != scan[1])      continue;

        /* The check at best_len-1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        scan += 2, match++;

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart+258.
         */
        do {
        } while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        ZAssert(state,scan <= state.ds.window+(unsigned)(state.ds.window_size-1), "wild scan");

        len = ZMAX_MATCH - (int)(strend - scan);
        scan = strend - ZMAX_MATCH;


        if (len > best_len) {
            state.ds.match_start = cur_match;
            best_len = len;
            if (len >= state.ds.nice_match) break;
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
        }
    } while ((cur_match = state.ds.prev[cur_match & ZWMASK]) > limit
             && --chain_length != 0);

    return best_len;
}

#define zcheck_match(state,start, match, length)
// or alternatively...
//void check_match(TState &state,IPos start, IPos match, int length)
//{ // check that the match is indeed a match
//    if (memcmp((char*)state.ds.window + match,
//                (char*)state.ds.window + start, length) != EQUAL) {
//        fprintf(stderr,
//            " start %d, match %d, length %d\n",
//            start, match, length);
//        error("invalid match");
//    }
//    if (state.verbose > 1) {
//        fprintf(stderr,"\\[%d,%d]", start-match, length);
//        do { fprintf(stdout,"%c",state.ds.window[start++]); } while (--length != 0);
//    }
//}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of input file.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or eofile is set; file reads are
 *    performed for at least two bytes (required for the translate_eol option).
 */
void zfill_window(ZTState &state)
{
    register unsigned n, m;
    unsigned more;    /* Amount of free space at the end of the window. */

    do {
        more = (unsigned)(state.ds.window_size - (uLong)state.ds.lookahead - (uLong)state.ds.strstart);

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        if (more == (unsigned)EOF) {
            /* Very unlikely, but possible on 16 bit machine if strstart == 0
             * and lookahead == 1 (input done one byte at time)
             */
            more--;

        /* For MMAP or BIG_MEM, the whole input file is already in memory so
         * we must not perform sliding. We must however call (*read_buf)() in
         * order to compute the crc, update lookahead and possibly set eofile.
         */
        } else if (state.ds.strstart >= ZWSIZE+ZMAX_DIST && state.ds.sliding) {

            /* By the IN assertion, the window is not empty so we can't confuse
             * more == 0 with more == 64K on a 16 bit machine.
             */
            memcpy((char*)state.ds.window, (char*)state.ds.window+ZWSIZE, (unsigned)ZWSIZE);
            state.ds.match_start -= ZWSIZE;
            state.ds.strstart    -= ZWSIZE; /* we now have strstart >= MAX_DIST: */

            state.ds.block_start -= (long) ZWSIZE;

            for (n = 0; n < ZHASH_SIZE; n++) {
                m = state.ds.head[n];
                state.ds.head[n] = (Pos)(m >= ZWSIZE ? m-ZWSIZE : ZNIL);
            }
            for (n = 0; n < ZWSIZE; n++) {
                m = state.ds.prev[n];
                state.ds.prev[n] = (Pos)(m >= ZWSIZE ? m-ZWSIZE : ZNIL);
                /* If n is not on any hash chain, prev[n] is garbage but
                 * its value will never be used.
                 */
            }
            more += ZWSIZE;
        }
        if (state.ds.eofile) return;

        /* If there was no sliding:
         *    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
         *    more == window_size - lookahead - strstart
         * => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
         * => more >= window_size - 2*WSIZE + 2
         * In the MMAP or BIG_MEM case (not yet supported in gzip),
         *   window_size == input_size + MIN_LOOKAHEAD  &&
         *   strstart + lookahead <= input_size => more >= MIN_LOOKAHEAD.
         * Otherwise, window_size == 2*WSIZE so more >= 2.
         * If there was sliding, more >= WSIZE. So in all cases, more >= 2.
         */
        ZAssert(state,more >= 2, "more < 2");

        n = state.readfunc(state, (char*)state.ds.window+state.ds.strstart+state.ds.lookahead, more);

        if (n == 0 || n == (unsigned)EOF) {
            state.ds.eofile = 1;
        } else {
            state.ds.lookahead += n;
        }
    } while (state.ds.lookahead < ZMIN_LOOKAHEAD && !state.ds.eofile);
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define ZFLUSH_BLOCK(state,eof) \
   zflush_block(state,state.ds.block_start >= 0L ? (char*)&state.ds.window[(unsigned)state.ds.block_start] : \
                (char*)NULL, (long)state.ds.strstart - state.ds.block_start, (eof))

/* ===========================================================================
 * Processes a new input file and return its compressed length. This
 * function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
uLong zdeflate_fast(ZTState &state)
{
    IPos hash_head = ZNIL;       /* head of the hash chain */
    int flush;                  /* set if current block must be flushed */
    unsigned match_length = 0;  /* length of best match */

    state.ds.prev_length = ZMIN_MATCH-1;
    while (state.ds.lookahead != 0) {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (state.ds.lookahead >= ZMIN_MATCH)
        ZINSERT_STRING(state.ds.strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        if (hash_head != ZNIL && state.ds.strstart - hash_head <= ZMAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            /* Do not look for matches beyond the end of the input.
             * This is necessary to make deflate deterministic.
             */
            if ((unsigned)state.ds.nice_match > state.ds.lookahead) state.ds.nice_match = (int)state.ds.lookahead;
            match_length = zlongest_match (state,hash_head);
            /* longest_match() sets match_start */
            if (match_length > state.ds.lookahead) match_length = state.ds.lookahead;
        }
        if (match_length >= ZMIN_MATCH) {
            zcheck_match(state,state.ds.strstart, state.ds.match_start, match_length);

            flush = zct_tally(state,state.ds.strstart-state.ds.match_start, match_length - ZMIN_MATCH);

            state.ds.lookahead -= match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
            if (match_length <= state.ds.zmax_insert_length
                && state.ds.lookahead >= ZMIN_MATCH) {
                match_length--; /* string at strstart already in hash table */
                do {
                    state.ds.strstart++;
                    ZINSERT_STRING(state.ds.strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } while (--match_length != 0);
                state.ds.strstart++;
            } else {
                state.ds.strstart += match_length;
                match_length = 0;
                state.ds.ins_h = state.ds.window[state.ds.strstart];
                ZUPDATE_HASH(state.ds.ins_h, state.ds.window[state.ds.strstart+1]);
                ZAssert(state,ZMIN_MATCH==3,"Call UPDATE_HASH() MIN_MATCH-3 more times");
            }
        } else {
            /* No match, output a literal byte */
            flush = zct_tally (state,0, state.ds.window[state.ds.strstart]);
            state.ds.lookahead--;
            state.ds.strstart++;
        }
        if (flush) ZFLUSH_BLOCK(state,0), state.ds.block_start = state.ds.strstart;

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (state.ds.lookahead < ZMIN_LOOKAHEAD) zfill_window(state);
    }
    return ZFLUSH_BLOCK(state,1); /* eof */
}

/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
uLong zdeflate(ZTState &state)
{
    IPos hash_head = ZNIL;       /* head of hash chain */
    IPos prev_match;            /* previous match */
    int flush;                  /* set if current block must be flushed */
    int match_available = 0;    /* set if previous match exists */
    register unsigned match_length = ZMIN_MATCH-1; /* length of best match */

    if (state.level <= 3) return zdeflate_fast(state); /* optimized for speed */

    /* Process the input block. */
    while (state.ds.lookahead != 0) {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (state.ds.lookahead >= ZMIN_MATCH)
        ZINSERT_STRING(state.ds.strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         */
        state.ds.prev_length = match_length, prev_match = state.ds.match_start;
        match_length = ZMIN_MATCH-1;

        if (hash_head != ZNIL && state.ds.prev_length < state.ds.max_lazy_match &&
            state.ds.strstart - hash_head <= ZMAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            /* Do not look for matches beyond the end of the input.
             * This is necessary to make deflate deterministic.
             */
            if ((unsigned)state.ds.nice_match > state.ds.lookahead) state.ds.nice_match = (int)state.ds.lookahead;
            match_length = zlongest_match (state,hash_head);
            /* longest_match() sets match_start */
            if (match_length > state.ds.lookahead) match_length = state.ds.lookahead;

            /* Ignore a length 3 match if it is too distant: */
            if (match_length == ZMIN_MATCH && state.ds.strstart-state.ds.match_start > ZTOO_FAR){
                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                match_length = ZMIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (state.ds.prev_length >= ZMIN_MATCH && match_length <= state.ds.prev_length) {
            unsigned max_insert = state.ds.strstart + state.ds.lookahead - ZMIN_MATCH;
            zcheck_match(state,state.ds.strstart-1, prev_match, state.ds.prev_length);
            flush = zct_tally(state,state.ds.strstart-1-prev_match, state.ds.prev_length - ZMIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */
            state.ds.lookahead -= state.ds.prev_length-1;
            state.ds.prev_length -= 2;
            do {
                if (++state.ds.strstart <= max_insert) {
                    ZINSERT_STRING(state.ds.strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                }
            } while (--state.ds.prev_length != 0);
            state.ds.strstart++;
            match_available = 0;
            match_length = ZMIN_MATCH-1;

            if (flush) ZFLUSH_BLOCK(state,0), state.ds.block_start = state.ds.strstart;

        } else if (match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            if (zct_tally (state,0, state.ds.window[state.ds.strstart-1])) {
                ZFLUSH_BLOCK(state,0), state.ds.block_start = state.ds.strstart;
            }
            state.ds.strstart++;
            state.ds.lookahead--;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            match_available = 1;
            state.ds.strstart++;
            state.ds.lookahead--;
        }
//        Assert(state,strstart <= isize && lookahead <= isize, "a bit too far");

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (state.ds.lookahead < ZMIN_LOOKAHEAD) zfill_window(state);
    }
    if (match_available) zct_tally (state,0, state.ds.window[state.ds.strstart-1]);

    return ZFLUSH_BLOCK(state,1); /* eof */
}

int zputlocal(struct zzlist *z, ZWRITEFUNC wfunc,void *param)
{ // Write a local header described by *z to file *f.  Return a ZE_ error code.
  ZPUTLG(ZLOCSIG, f);
  ZPUTSH(z->ver, f);
  ZPUTSH(z->lflg, f);
  ZPUTSH(z->how, f);
  ZPUTLG(z->tim, f);
  ZPUTLG(z->crc, f);
  ZPUTLG(z->siz, f);
  ZPUTLG(z->len, f);
  ZPUTSH(z->nam, f);
  ZPUTSH(z->ext, f);
  size_t res = (size_t)wfunc(param, z->iname, (unsigned int)z->nam);
  if (res!=z->nam) return ZZE_TEMP;
  if (z->ext)
  { res = (size_t)wfunc(param, z->extra, (unsigned int)z->ext);
    if (res!=z->ext) return ZZE_TEMP;
  }
  return ZZE_OK;
}

int zputextended(struct zzlist *z, ZWRITEFUNC wfunc, void *param)
{ // Write an extended local header described by *z to file *f. Returns a ZE_ code
  ZPUTLG(ZEXTLOCSIG, f);
  ZPUTLG(z->crc, f);
  ZPUTLG(z->siz, f);
  ZPUTLG(z->len, f);
  return ZZE_OK;
}

int zputcentral(struct zzlist *z, ZWRITEFUNC wfunc, void *param)
{ // Write a central header entry of *z to file *f. Returns a ZE_ code.
  ZPUTLG(ZCENSIG, f);
  ZPUTSH(z->vem, f);
  ZPUTSH(z->ver, f);
  ZPUTSH(z->flg, f);
  ZPUTSH(z->how, f);
  ZPUTLG(z->tim, f);
  ZPUTLG(z->crc, f);
  ZPUTLG(z->siz, f);
  ZPUTLG(z->len, f);
  ZPUTSH(z->nam, f);
  ZPUTSH(z->cext, f);
  ZPUTSH(z->com, f);
  ZPUTSH(z->dsk, f);
  ZPUTSH(z->att, f);
  ZPUTLG(z->atx, f);
  ZPUTLG(z->off, f);
  if ((size_t)wfunc(param, z->iname, (unsigned int)z->nam) != z->nam ||
      (z->cext && (size_t)wfunc(param, z->cextra, (unsigned int)z->cext) != z->cext) ||
      (z->com && (size_t)wfunc(param, z->comment, (unsigned int)z->com) != z->com))
    return ZZE_TEMP;
  return ZZE_OK;
}


int zputend(int n, uLong s, uLong c, fSize m, char *z, ZWRITEFUNC wfunc, void *param)
{ // write the end of the central-directory-data to file *f.
  ZPUTLG(ZENDSIG, f);
  ZPUTSH(0, f);
  ZPUTSH(0, f);
  ZPUTSH(n, f);
  ZPUTSH(n, f);
  ZPUTLG(s, f);
  ZPUTLG(c, f);
  ZPUTSH(m, f);
  // Write the comment, if any
  if (m && wfunc(param, z, (unsigned int)m) != m) return ZZE_TEMP;
  return ZZE_OK;
}

char zencode(unsigned long *keys, char c)
{ int t=zdecrypt_byte(keys);
  zupdate_keys(keys,c);
  return (char)(t^c);
}

int zlustricmp(const ZTCHAR *sa, const ZTCHAR *sb)
{ for (const ZTCHAR *ca=sa, *cb=sb; ; ca++, cb++)
  { int ia=tolower(*ca), ib=tolower(*cb);
    if (ia==ib && ia==0) return 0;
    if (ia==ib) continue;
    if (ia<ib) return -1;
    if (ia>ib) return 1;
  }
}


bool ZHasZipSuffix(const ZTCHAR *fn)
{ const ZTCHAR *ext = fn+z_tcslen(fn);
  while (ext>fn && *ext!=Z_T('.')) ext--;
  if (ext==fn && *ext!=Z_T('.')) return false;
  if (zlustricmp(ext,Z_T(".Z"))==0) return true;
  if (zlustricmp(ext,Z_T(".zip"))==0) return true;
  if (zlustricmp(ext,Z_T(".zoo"))==0) return true;
  if (zlustricmp(ext,Z_T(".arc"))==0) return true;
  if (zlustricmp(ext,Z_T(".lzh"))==0) return true;
  if (zlustricmp(ext,Z_T(".arj"))==0) return true;
  if (zlustricmp(ext,Z_T(".gz"))==0) return true;
  if (zlustricmp(ext,Z_T(".tgz"))==0) return true;
  return false;
}

class TZip
{ public:
  TZip(const char *pwd) : hfout(0),mustclosehfout(false),hmapout(0),zfis(0),obuf(0),hfin(0),writ(0),oerr(false),hasputcen(false),ooffset(0),encwriting(false),encbuf(0),password(0), state(0) {if (pwd!=0 && *pwd!=0) {password=new char[strlen(pwd)+1]; strcpy(password,pwd);}}
  ~TZip() {if (state!=0) delete state; state=0; if (encbuf!=0) delete[] encbuf; encbuf=0; if (password!=0) delete[] password; password=0;}

  // These variables say about the file we're writing into
  // We can write to pipe, file-by-handle, file-by-name, memory-to-memmapfile
  char *password;           // keep a copy of the password
  ZHANDLE hfout;             // if valid, we'll write here (for files or pipes)
  bool mustclosehfout;      // if true, we are responsible for closing hfout
  ZHANDLE hmapout;           // otherwise, we'll write here (for memmap)
  unsigned ooffset;         // for hfout, this is where the pointer was initially
  ZZRESULT oerr;             // did a write operation give rise to an error?
  unsigned writ;            // how far have we written. This is maintained by Add, not write(), to avoid confusion over seeks
  bool ocanseek;            // can we seek?
  char *obuf;               // this is where we've locked mmap to view.
  unsigned int opos;        // current pos in the mmap
  unsigned int mapsize;     // the size of the map we created
  bool hasputcen;           // have we yet placed the central directory?
  bool encwriting;          // if true, then we'll encrypt stuff using 'keys' before we write it to disk
  unsigned long keys[3];    // keys are initialised inside Add()
  char *encbuf;             // if encrypting, then this is a temporary workspace for encrypting the data
  unsigned int encbufsize;  // (to be used and resized inside write(), and deleted in the destructor)
  //
  ZTZipFileInfo *zfis;       // each file gets added onto this list, for writing the table at the end
  ZTState *state;            // we use just one state object per zip, because it's big (500k)

  ZZRESULT Create(void *z,unsigned int len,ZDWORD flags);
  static unsigned sflush(void *param,const char *buf, unsigned *size);
  static unsigned swrite(void *param,const char *buf, unsigned size);
  unsigned int write(const char *buf,unsigned int size);
  bool oseek(unsigned int pos);
  ZZRESULT GetMemory(void **pbuf, unsigned long *plen);
  ZZRESULT Close();

  // some variables to do with the file currently being read:
  // I haven't done it object-orientedly here, just put them all
  // together, since OO didn't seem to make the design any clearer.
  uLong attr; ziztimes times; uLong timestamp;  // all open_* methods set these
  bool iseekable; long isize,ired;         // size is not set until close() on pips
  uLong crc;                                 // crc is not set until close(). iwrit is cumulative
  ZHANDLE hfin; bool selfclosehf;           // for input files and pipes
  const char *bufin; unsigned int lenin,posin; // for memory
  // and a variable for what we've done with the input: (i.e. compressed it!)
  uLong csize;                               // compressed size, set by the compression routines
  // and this is used by some of the compression routines
  char buf[16384];


  ZZRESULT open_file(const ZTCHAR *fn);
  ZZRESULT open_handle(ZHANDLE hf,unsigned int len);
  ZZRESULT open_mem(void *src,unsigned int len);
  ZZRESULT open_dir();
  static unsigned sread(ZTState &s,char *buf,unsigned size);
  unsigned read(char *buf, unsigned size);
  ZZRESULT iclose();

  ZZRESULT ideflate(ZTZipFileInfo *zfi);
  ZZRESULT istore();

  ZZRESULT Add(const ZTCHAR *odstzn, void *src,unsigned int len, ZDWORD flags);
  ZZRESULT AddCentral();

};



ZZRESULT TZip::Create(void *z,unsigned int len,ZDWORD flags)
{ if (hfout!=0 || hmapout!=0 || obuf!=0 || writ!=0 || oerr!=ZZR_OK || hasputcen) return ZZR_NOTINITED;
  //
  if (flags==ZZIP_HANDLE)
  { ZHANDLE hf = (ZHANDLE)z;
    hfout=hf; mustclosehfout=false;
#ifdef DuplicateHandle
    BOOL res = DuplicateHandle(GetCurrentProcess(),hf,GetCurrentProcess(),&hfout,0,FALSE,DUPLICATE_SAME_ACCESS);
    if (res) mustclosehandle=true;
#endif
    // now we have hfout. Either we duplicated the handle and we close it ourselves
    // (while the caller closes h themselves), or we couldn't duplicate it.
    ZDWORD res=ZGetFilePosZ(hfout);
    ocanseek = (res!=0xFFFFFFFF);
    ooffset = ocanseek ? res : 0;
    return ZZR_OK;
  }
  else if (flags==ZZIP_FILENAME)
  { const ZTCHAR *fn = (const ZTCHAR*)z;
#ifdef ZIP_STD
    hfout = fopen(fn,"wb");
    if (hfout==0) return ZZR_NOFILE;
#else
    hfout = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hfout==Z_INVALID_HANDLE_VALUE) {hfout=0; return ZZR_NOFILE;}
#endif
    ocanseek=true;
    ooffset=0;
    mustclosehfout=true;
    return ZZR_OK;
  }
  else if (flags==ZZIP_MEMORY)
  { unsigned int size = len;
    if (size==0) return ZZR_MEMSIZE;
#ifdef ZIP_STD
    if (z!=0) obuf=(char*)z;
    else return ZZR_ARGS;
#else
    if (z!=0) obuf=(char*)z;
    else
    { hmapout = CreateFileMapping(Z_INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,size,NULL);
      if (hmapout==NULL) return ZZR_NOALLOC;
      obuf = (char*)MapViewOfFile(hmapout,FILE_MAP_ALL_ACCESS,0,0,size);
      if (obuf==0) {CloseHandle(hmapout); hmapout=0; return ZZR_NOALLOC;}
    }
#endif
    ocanseek=true;
    opos=0; mapsize=size;
    return ZZR_OK;
  }
  else return ZZR_ARGS;
}

unsigned TZip::sflush(void *param,const char *buf, unsigned *size)
{ // static
  if (*size==0) return 0;
  TZip *zip = (TZip*)param;
  unsigned int writ = zip->write(buf,*size);
  if (writ!=0) *size=0;
  return writ;
}
unsigned TZip::swrite(void *param,const char *buf, unsigned size)
{ // static
  if (size==0) return 0;
  TZip *zip=(TZip*)param; return zip->write(buf,size);
}
unsigned int TZip::write(const char *buf,unsigned int size)
{ const char *srcbuf=buf;
  if (encwriting)
  { if (encbuf!=0 && encbufsize<size) {delete[] encbuf; encbuf=0;}
    if (encbuf==0) {encbuf=new char[size*2]; encbufsize=size;}
    memcpy(encbuf,buf,size);
    for (unsigned int i=0; i<size; i++) encbuf[i]=zencode(keys,encbuf[i]);
    srcbuf=encbuf;
  }
  if (obuf!=0)
  { if (opos+size>=mapsize) {oerr=ZZR_MEMSIZE; return 0;}
    memcpy(obuf+opos, srcbuf, size);
    opos+=size;
    return size;
  }
  else if (hfout!=0)
  {
#ifdef ZIP_STD
    ZDWORD writ=(ZDWORD)fwrite(srcbuf,1,size,hfout);
#else
    DWORD writ; WriteFile(hfout,srcbuf,size,&writ,NULL);
#endif
    return writ;
  }
  oerr=ZZR_NOTINITED; return 0;
}

bool TZip::oseek(unsigned int pos)
{ if (!ocanseek) {oerr=ZZR_SEEK; return false;}
  if (obuf!=0)
  { if (pos>=mapsize) {oerr=ZZR_MEMSIZE; return false;}
    opos=pos;
    return true;
  }
  else if (hfout!=0)
  {
#ifdef ZIP_STD
    fseek(hfout,pos+ooffset,SEEK_SET);
#else
    SetFilePointer(hfout,pos+ooffset,NULL,FILE_BEGIN);
#endif
    return true;
  }
  oerr=ZZR_NOTINITED; return 0;
}

ZZRESULT TZip::GetMemory(void **pbuf, unsigned long *plen)
{ // When the user calls GetMemory, they're presumably at the end
  // of all their adding. In any case, we have to add the central
  // directory now, otherwise the memory we tell them won't be complete.
  if (!hasputcen) AddCentral(); hasputcen=true;
  if (pbuf!=NULL) *pbuf=(void*)obuf;
  if (plen!=NULL) *plen=writ;
  if (obuf==NULL) return ZZR_NOTMMAP;
  return ZZR_OK;
}

ZZRESULT TZip::Close()
{ // if the directory hadn't already been added through a call to GetMemory,
  // then we do it now
  ZZRESULT res=ZZR_OK; if (!hasputcen) res=AddCentral(); hasputcen=true;
#ifdef ZIP_STD
  if (hfout!=0 && mustclosehfout) fclose(hfout); hfout=0; mustclosehfout=false;
#else
  if (obuf!=0 && hmapout!=0) UnmapViewOfFile(obuf); obuf=0;
  if (hmapout!=0) CloseHandle(hmapout); hmapout=0;
  if (hfout!=0 && mustclosehfout) CloseHandle(hfout); hfout=0; mustclosehfout=false;
#endif
  return res;
}

ZZRESULT TZip::open_file(const ZTCHAR *fn)
{ hfin=0; bufin=0; selfclosehf=false; crc=ZCRCVAL_INITIAL; isize=0; csize=0; ired=0;
  if (fn==0) return ZZR_ARGS;
#ifdef ZIP_STD
  ZHANDLE hf = fopen(fn,"rb");
  if (hf==0) return ZZR_NOFILE;
  ZZRESULT res = open_handle(hf,0);
  if (res!=ZZR_OK) {fclose(hf); return res;}
#else
  ZHANDLE hf = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (hf==Z_INVALID_HANDLE_VALUE) return ZZR_NOFILE;
  ZZRESULT res = open_handle(hf,0);
  if (res!=ZZR_OK) {CloseHandle(hf); return res;}
#endif
  selfclosehf=true;
  return ZZR_OK;
}
ZZRESULT TZip::open_handle(ZHANDLE hf,unsigned int len)
{ hfin=0; bufin=0; selfclosehf=false; crc=ZCRCVAL_INITIAL; isize=0; csize=0; ired=0;
  if (hf==0 || hf==Z_INVALID_HANDLE_VALUE) return ZZR_ARGS;
  bool canseek;
#ifdef ZIP_STD
    struct stat st; fstat(fileno(hf),&st); canseek = S_ISREG(st.st_mode);
#else
  ZDWORD res = SetFilePointer(hfout,0,0,FILE_CURRENT);
  canseek = (res!=0xFFFFFFFF);
#endif
  if (canseek)
  { ZZRESULT res = ZGetFileInfo(hf,&attr,&isize,&times,&timestamp);
    if (res!=ZZR_OK) return res;
#ifdef ZIP_STD
    fseek(hf,0,SEEK_SET);
#else
    SetFilePointer(hf,0,NULL,FILE_BEGIN); // because GetFileInfo will have screwed it up
#endif
    iseekable=true; hfin=hf;
    return ZZR_OK;
  }
  else
  { attr= 0x80000000;      // just a normal file
    isize = -1;            // can't know size until at the end
    if (len!=0) isize=len; // unless we were told explicitly!
    iseekable=false;
    ZWORD dosdate, dostime; ZGetNow(&times.atime, &dosdate, &dostime);
    times.mtime=times.atime;
    times.ctime=times.atime;
    timestamp = (ZWORD)dostime | (((ZDWORD)dosdate)<<16);
    hfin=hf;
    return ZZR_OK;
  }
}
ZZRESULT TZip::open_mem(void *src,unsigned int len)
{ hfin=0; bufin=(const char*)src; selfclosehf=false; crc=ZCRCVAL_INITIAL; ired=0; csize=0; ired=0;
  lenin=len; posin=0;
  if (src==0 || len==0) return ZZR_ARGS;
  attr= 0x80000000; // just a normal file
  isize = len;
  iseekable=true;
  ZWORD dosdate, dostime; ZGetNow(&times.atime, &dosdate, &dostime);
  times.mtime=times.atime;
  times.ctime=times.atime;
  timestamp = (ZWORD)dostime | (((ZDWORD)dosdate)<<16);
  return ZZR_OK;
}
ZZRESULT TZip::open_dir()
{ hfin=0; bufin=0; selfclosehf=false; crc=ZCRCVAL_INITIAL; isize=0; csize=0; ired=0;
  attr= 0x41C00010; // a readable writable directory, and again directory
  isize = 0;
  iseekable=false;
  ZWORD dosdate, dostime; ZGetNow(&times.atime, &dosdate, &dostime);
  times.mtime=times.atime;
  times.ctime=times.atime;
  timestamp = (ZWORD)dostime | (((ZDWORD)dosdate)<<16);
  return ZZR_OK;
}

unsigned TZip::sread(ZTState &s,char *buf,unsigned size)
{ // static
  TZip *zip = (TZip*)s.param;
  return zip->read(buf,size);
}

unsigned TZip::read(char *buf, unsigned size)
{ if (bufin!=0)
  { if (posin>=lenin) return 0; // end of input
    uLong red = lenin-posin;
    if (red>size) red=size;
    memcpy(buf, bufin+posin, red);
    posin += red;
    ired += red;
    crc = zcrc32(crc, (uChar*)buf, red);
    return red;
  }
  else if (hfin!=0)
  { ZDWORD red;
#ifdef ZIP_STD
    red = (ZDWORD)fread(buf,1,size,hfin);
    if (red==0) return 0;
#else
    BOOL ok = ReadFile(hfin,buf,size,&red,NULL);
    if (!ok) return 0;
#endif
    ired += red;
    crc = zcrc32(crc, (uChar*)buf, red);
    return red;
  }
  else {oerr=ZZR_NOTINITED; return 0;}
}

ZZRESULT TZip::iclose()
{
#ifdef ZIP_STD
  if (selfclosehf && hfin!=0) fclose(hfin); hfin=0;
#else
  if (selfclosehf && hfin!=0) CloseHandle(hfin); hfin=0;
#endif
  bool mismatch = (isize!=-1 && isize!=ired);
  isize=ired; // and crc has been being updated anyway
  if (mismatch) return ZZR_MISSIZE;
  else return ZZR_OK;
}



ZZRESULT TZip::ideflate(ZTZipFileInfo *zfi)
{ if (state==0) state=new ZTState();
  // It's a very big object! 500k! We allocate it on the heap, because PocketPC's
  // stack breaks if we try to put it all on the stack. It will be deleted lazily
  state->err=0;
  state->readfunc=sread; state->flush_outbuf=sflush;
  state->param=this; state->level=8; state->seekable=iseekable; state->err=NULL;
  // the following line will make ct_init realise it has to perform the init
  state->ts.static_dtree[0].dl.len = 0;
  // Thanks to Alvin77 for this crucial fix:
  state->ds.window_size=0;
  //  I think that covers everything that needs to be initted.
  //
  zbi_init(*state,buf, sizeof(buf), 1); // it used to be just 1024-size, not 16384 as here
  zct_init(*state,&zfi->att);
  zlm_init(*state,state->level, &zfi->flg);
  uLong sz = zdeflate(*state);
  csize=sz;
  ZZRESULT r=ZZR_OK; if (state->err!=NULL) r=ZZR_FLATE;
  return r;
}

ZZRESULT TZip::istore()
{ uLong size=0;
  for (;;)
  { unsigned int cin=read(buf,16384); if (cin<=0 || cin==(unsigned int)EOF) break;
    unsigned int cout = write(buf,cin); if (cout!=cin) return ZZR_MISSIZE;
    size += cin;
  }
  csize=size;
  return ZZR_OK;
}

bool has_seeded=false;
ZZRESULT TZip::Add(const ZTCHAR *odstzn, void *src,unsigned int len, ZDWORD flags)
{ if (oerr) return ZZR_FAILED;
  if (hasputcen) return ZZR_ENDED;

  // if we use password encryption, then every isize and csize is 12 bytes bigger
  int passex=0; if (password!=0 && flags!=ZZIP_FOLDER) passex=12;

  // zip has its own notion of what its names should look like: i.e. dir/file.stuff
  ZTCHAR dstzn[_Z_MAX_PATH]; z_tcsncpy(dstzn,odstzn,_Z_MAX_PATH); dstzn[_Z_MAX_PATH-1]=0;
  if (*dstzn==0) return ZZR_ARGS;
  ZTCHAR *d=dstzn; while (*d!=0) {if (*d==Z_T('\\')) *d=Z_T('/'); d++;}
  bool isdir = (flags==ZZIP_FOLDER);
  bool needs_trailing_slash = (isdir && dstzn[z_tcslen(dstzn)-1]!=Z_T('/'));
  int method=ZDEFLATE; if (isdir || ZHasZipSuffix(dstzn)) method=ZSTORE;

  // now open whatever was our input source:
  ZZRESULT openres;
  if (flags==ZZIP_FILENAME) openres=open_file((const ZTCHAR*)src);
  else if (flags==ZZIP_HANDLE) openres=open_handle((ZHANDLE)src,len);
  else if (flags==ZZIP_MEMORY) openres=open_mem(src,len);
  else if (flags==ZZIP_FOLDER) openres=open_dir();
  else return ZZR_ARGS;
  if (openres!=ZZR_OK) return openres;

  // A zip "entry" consists of a local header (which includes the file name),
  // then the compressed data, and possibly an extended local header.

  // Initialize the local header
  ZTZipFileInfo zfi; zfi.nxt=NULL;
  strcpy(zfi.name,"");
#ifdef UNICODE
  WideCharToMultiByte(CP_UTF8,0,dstzn,-1,zfi.iname,_Z_MAX_PATH,0,0);
#else
  strncpy(zfi.iname,dstzn,_Z_MAX_PATH); zfi.iname[_Z_MAX_PATH-1]=0;
#endif
  zfi.nam=strlen(zfi.iname);
  if (needs_trailing_slash) {strcat(zfi.iname,"/"); zfi.nam++;}
  strcpy(zfi.zname,"");
  zfi.extra=NULL; zfi.ext=0;   // extra header to go after this compressed data, and its length
  zfi.cextra=NULL; zfi.cext=0; // extra header to go in the central end-of-zip directory, and its length
  zfi.comment=NULL; zfi.com=0; // comment, and its length
  zfi.mark = 1;
  zfi.dosflag = 0;
  zfi.att = (uShort)ZBINARY;
  zfi.vem = (uShort)0xB17; // 0xB00 is win32 os-code. 0x17 is 23 in decimal: zip 2.3
  zfi.ver = (uShort)20;    // Needs PKUNZIP 2.0 to unzip it
  zfi.tim = timestamp;
  // Even though we write the header now, it will have to be rewritten, since we don't know compressed size or crc.
  zfi.crc = 0;            // to be updated later
  zfi.flg = 8;            // 8 means 'there is an extra header'. Assume for the moment that we need it.
  if (password!=0 && !isdir) zfi.flg=9;  // and 1 means 'password-encrypted'
  zfi.lflg = zfi.flg;     // to be updated later
  zfi.how = (uShort)method;  // to be updated later
  zfi.siz = (uLong)(method==ZSTORE && isize>=0 ? isize+passex : 0); // to be updated later
  zfi.len = (uLong)(isize);  // to be updated later
  zfi.dsk = 0;
  zfi.atx = attr;
  zfi.off = writ+ooffset;         // offset within file of the start of this local record
  // stuff the 'times' structure into zfi.extra

  // nb. apparently there's a problem with PocketPC CE(zip)->CE(unzip) fails. And removing the following block fixes it up.
  char xloc[ZEB_L_UT_SIZE]; zfi.extra=xloc;  zfi.ext=ZEB_L_UT_SIZE;
  char xcen[ZEB_C_UT_SIZE]; zfi.cextra=xcen; zfi.cext=ZEB_C_UT_SIZE;
  xloc[0]  = 'U';
  xloc[1]  = 'T';
  xloc[2]  = ZEB_UT_LEN(3);       // length of data part of e.f.
  xloc[3]  = 0;
  xloc[4]  = ZEB_UT_FL_MTIME | ZEB_UT_FL_ATIME | ZEB_UT_FL_CTIME;
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
  memcpy(zfi.cextra,zfi.extra,ZEB_C_UT_SIZE);
  zfi.cextra[ZEB_LEN] = ZEB_UT_LEN(1);


  // (1) Start by writing the local header:
  int r = zputlocal(&zfi,swrite,this);
  if (r!=ZZE_OK) {iclose(); return ZZR_WRITE;}
  writ += 4 + ZLOCHEAD + (unsigned int)zfi.nam + (unsigned int)zfi.ext;
  if (oerr!=ZZR_OK) {iclose(); return oerr;}

  // (1.5) if necessary, write the encryption header
  keys[0]=305419896L;
  keys[1]=591751049L;
  keys[2]=878082192L;
  for (const char *cp=password; cp!=0 && *cp!=0; cp++) zupdate_keys(keys,*cp);
  // generate some random bytes
#ifdef ZIP_STD
  if (!has_seeded) srand((unsigned)time(0));
#else
  if (!has_seeded) srand(GetTickCount()^(unsigned long)GetDesktopWindow());
#endif
  char encbuf[12]; for (int i=0; i<12; i++) encbuf[i]=(char)((rand()>>7)&0xff);
  encbuf[11] = (char)((zfi.tim>>8)&0xff);
  for (int ei=0; ei<12; ei++) encbuf[ei]=zencode(keys,encbuf[ei]);
  if (password!=0 && !isdir) {swrite(this,encbuf,12); writ+=12;}

  //(2) Write deflated/stored file to zip file
  ZZRESULT writeres=ZZR_OK;
  encwriting = (password!=0 && !isdir);  // an object member variable to say whether we write to disk encrypted
  if (!isdir && method==ZDEFLATE) writeres=ideflate(&zfi);
  else if (!isdir && method==ZSTORE) writeres=istore();
  else if (isdir) csize=0;
  encwriting = false;
  iclose();
  writ += csize;
  if (oerr!=ZZR_OK) return oerr;
  if (writeres!=ZZR_OK) return ZZR_WRITE;

  // (3) Either rewrite the local header with correct information...
  bool first_header_has_size_right = (zfi.siz==csize+passex);
  zfi.crc = crc;
  zfi.siz = csize+passex;
  zfi.len = isize;
  if (ocanseek && (password==0 || isdir))
  { zfi.how = (uShort)method;
    if ((zfi.flg & 1) == 0) zfi.flg &= ~8; // clear the extended local header flag
    zfi.lflg = zfi.flg;
    // rewrite the local header:
    if (!oseek(zfi.off-ooffset)) return ZZR_SEEK;
    if ((r = zputlocal(&zfi, swrite,this)) != ZZE_OK) return ZZR_WRITE;
    if (!oseek(writ)) return ZZR_SEEK;
  }
  else
  { // (4) ... or put an updated header at the end
    if (zfi.how != (uShort) method) return ZZR_NOCHANGE;
    if (method==ZSTORE && !first_header_has_size_right) return ZZR_NOCHANGE;
    if ((r = zputextended(&zfi, swrite,this)) != ZZE_OK) return ZZR_WRITE;
    writ += 16L;
    zfi.flg = zfi.lflg; // if flg modified by inflate, for the central index
  }
  if (oerr!=ZZR_OK) return oerr;

  // Keep a copy of the zipfileinfo, for our end-of-zip directory
  char *cextra = new char[zfi.cext]; memcpy(cextra,zfi.cextra,zfi.cext); zfi.cextra=cextra;
  ZTZipFileInfo *pzfi = new ZTZipFileInfo; memcpy(pzfi,&zfi,sizeof(zfi));
  if (zfis==NULL) zfis=pzfi;
  else {ZTZipFileInfo *z=zfis; while (z->nxt!=NULL) z=z->nxt; z->nxt=pzfi;}
  return ZZR_OK;
}

ZZRESULT TZip::AddCentral()
{ // write central directory
  int numentries = 0;
  uLong pos_at_start_of_central = writ;
  //uLong tot_unc_size=0, tot_compressed_size=0;
  bool okay=true;
  for (ZTZipFileInfo *zfi=zfis; zfi!=NULL; )
  { if (okay)
    { int res = zputcentral(zfi, swrite,this);
      if (res!=ZZE_OK) okay=false;
    }
    writ += 4 + ZCENHEAD + (unsigned int)zfi->nam + (unsigned int)zfi->cext + (unsigned int)zfi->com;
    //tot_unc_size += zfi->len;
    //tot_compressed_size += zfi->siz;
    numentries++;
    //
    ZTZipFileInfo *zfinext = zfi->nxt;
    if (zfi->cextra!=0) delete[] zfi->cextra;
    delete zfi;
    zfi = zfinext;
  }
  uLong center_size = writ - pos_at_start_of_central;
  if (okay)
  { int res = zputend(numentries, center_size, pos_at_start_of_central+ooffset, 0, NULL, swrite,this);
    if (res!=ZZE_OK) okay=false;
    writ += 4 + ZENDHEAD + 0;
  }
  if (!okay) return ZZR_WRITE;
  return ZZR_OK;
}

ZZRESULT zlasterrorZ=ZZR_OK;

unsigned int ZFormatZipMessageZ(ZZRESULT code, ZTCHAR *buf,unsigned int len)
{ if (code==ZZR_RECENT) code=zlasterrorZ;
  const ZTCHAR *msg=Z_T("unknown zip result code");
  switch (code)
  {
  case ZZR_OK: msg = Z_T("Success"); break;
  case ZZR_NODUPH: msg = Z_T("Culdn't duplicate handle"); break;
  case ZZR_NOFILE: msg = Z_T("Couldn't create/open file"); break;
  case ZZR_NOALLOC: msg = Z_T("Failed to allocate memory"); break;
  case ZZR_WRITE: msg = Z_T("Error writing to file"); break;
  case ZZR_NOTFOUND: msg = Z_T("File not found in the zipfile"); break;
  case ZZR_MORE: msg = Z_T("Still more data to unzip"); break;
  case ZZR_CORRUPT: msg = Z_T("Zipfile is corrupt or not a zipfile"); break;
  case ZZR_READ: msg = Z_T("Error reading file"); break;
  case ZZR_ARGS: msg = Z_T("Caller: faulty arguments"); break;
  case ZZR_PARTIALUNZ: msg = Z_T("Caller: the file had already been partially unzipped"); break;
  case ZZR_NOTMMAP: msg = Z_T("Caller: can only get memory of a memory zipfile"); break;
  case ZZR_MEMSIZE: msg = Z_T("Caller: not enough space allocated for memory zipfile"); break;
  case ZZR_FAILED: msg = Z_T("Caller: there was a previous error"); break;
  case ZZR_ENDED: msg = Z_T("Caller: additions to the zip have already been ended"); break;
  case ZZR_ZMODE: msg = Z_T("Caller: mixing creation and opening of zip"); break;
  case ZZR_NOTINITED: msg = Z_T("Zip-bug: internal initialisation not completed"); break;
  case ZZR_SEEK: msg = Z_T("Zip-bug: trying to seek the unseekable"); break;
  case ZZR_MISSIZE: msg = Z_T("Zip-bug: the anticipated size turned out wrong"); break;
  case ZZR_NOCHANGE: msg = Z_T("Zip-bug: tried to change mind, but not allowed"); break;
  case ZZR_FLATE: msg = Z_T("Zip-bug: an internal error during flation"); break;
  }
  unsigned int mlen=(unsigned int)z_tcslen(msg);
  if (buf==0 || len==0) return mlen;
  unsigned int n=mlen; if (n+1>len) n=len-1;
  z_tcsncpy(buf,msg,n); buf[n]=Z_T('\0');
  return mlen;
}



typedef struct
{ ZDWORD flag;
  TZip *zip;
} TZipHandleData;


HZIP CreateZipInternal(void *z,unsigned int len,ZDWORD flags, const char *password)
{ TZip *zip = new TZip(password);
  zlasterrorZ = zip->Create(z,len,flags);
  if (zlasterrorZ!=ZZR_OK) {delete zip; return 0;}
  TZipHandleData *han = new TZipHandleData;
  han->flag=2; han->zip=zip; return (HZIP)han;
}
HZIP CreateZipHandle(ZHANDLE h, const char *password) {return CreateZipInternal(h,0,ZZIP_HANDLE,password);}
HZIP CreateZip(const ZTCHAR *fn, const char *password) {return CreateZipInternal((void*)fn,0,ZZIP_FILENAME,password);}
HZIP CreateZip(void *z,unsigned int len, const char *password) {return CreateZipInternal(z,len,ZZIP_MEMORY,password);}


ZZRESULT ZipAddInternal(HZIP hz,const ZTCHAR *dstzn, void *src,unsigned int len, ZDWORD flags)
{ if (hz==0) {zlasterrorZ=ZZR_ARGS;return ZZR_ARGS;}
  TZipHandleData *han = (TZipHandleData*)hz;
  if (han->flag!=2) {zlasterrorZ=ZZR_ZMODE;return ZZR_ZMODE;}
  TZip *zip = han->zip;
  zlasterrorZ = zip->Add(dstzn,src,len,flags);
  return zlasterrorZ;
}
ZZRESULT ZipAdd(HZIP hz,const ZTCHAR *dstzn, const ZTCHAR *fn) {return ZipAddInternal(hz,dstzn,(void*)fn,0,ZZIP_FILENAME);}
ZZRESULT ZipAdd(HZIP hz,const ZTCHAR *dstzn, void *src,unsigned int len) {return ZipAddInternal(hz,dstzn,src,len,ZZIP_MEMORY);}
ZZRESULT ZipAddHandle(HZIP hz,const ZTCHAR *dstzn, ZHANDLE h) {return ZipAddInternal(hz,dstzn,h,0,ZZIP_HANDLE);}
ZZRESULT ZipAddHandle(HZIP hz,const ZTCHAR *dstzn, ZHANDLE h, unsigned int len) {return ZipAddInternal(hz,dstzn,h,len,ZZIP_HANDLE);}
ZZRESULT ZipAddFolder(HZIP hz,const ZTCHAR *dstzn) {return ZipAddInternal(hz,dstzn,0,0,ZZIP_FOLDER);}



ZZRESULT ZipGetMemory(HZIP hz, void **buf, unsigned long *len)
{ if (hz==0) {if (buf!=0) *buf=0; if (len!=0) *len=0; zlasterrorZ=ZZR_ARGS;return ZZR_ARGS;}
  TZipHandleData *han = (TZipHandleData*)hz;
  if (han->flag!=2) {zlasterrorZ=ZZR_ZMODE;return ZZR_ZMODE;}
  TZip *zip = han->zip;
  zlasterrorZ = zip->GetMemory(buf,len);
  return zlasterrorZ;
}

ZZRESULT CloseZipZ(HZIP hz)
{ if (hz==0) {zlasterrorZ=ZZR_ARGS;return ZZR_ARGS;}
  TZipHandleData *han = (TZipHandleData*)hz;
  if (han->flag!=2) {zlasterrorZ=ZZR_ZMODE;return ZZR_ZMODE;}
  TZip *zip = han->zip;
  zlasterrorZ = zip->Close();
  delete zip;
  delete han;
  return zlasterrorZ;
}

bool IsZipHandleZ(HZIP hz)
{ if (hz==0) return false;
  TZipHandleData *han = (TZipHandleData*)hz;
  return (han->flag==2);
}


#endif //__ZIP_H
