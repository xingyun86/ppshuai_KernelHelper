#ifndef __UNZIP_H
#define __UNZIP_H

#include "zdef.h"

typedef struct
{ int index;                 // index of this file within the zip
  ZTCHAR name[_Z_MAX_PATH];      // filename within the zip
  ZDWORD attr;                // attributes, as in GetFileAttributes.
  ZFILETIME atime,ctime,mtime;// access, create, modify filetimes
  long comp_size;            // sizes of item, compressed and uncompressed. These
  long unc_size;             // may be -1 if not yet known (e.g. being streamed in)
} ZIPENTRY;


HZIP OpenZip(const ZTCHAR *fn, const char *password);
HZIP OpenZip(void *z,unsigned int len, const char *password);
HZIP OpenZipHandle(ZHANDLE h, const char *password);
// OpenZip - opens a zip file and returns a handle with which you can
// subsequently examine its contents. You can open a zip file from:
// from a pipe:             OpenZipHandle(hpipe_read,0);
// from a file (by handle): OpenZipHandle(hfile,0);
// from a file (by name):   OpenZip("c:\\test.zip","password");
// from a memory block:     OpenZip(bufstart, buflen,0);
// If the file is opened through a pipe, then items may only be
// accessed in increasing order, and an item may only be unzipped once,
// although GetZipItem can be called immediately before and after unzipping
// it. If it's opened in any other way, then full random access is possible.
// Note: pipe input is not yet implemented.
// Note: zip passwords are ascii, not unicode.
// Note: for windows-ce, you cannot close the handle until after CloseZip.
// but for real windows, the zip makes its own copy of your handle, so you
// can close yours anytime.

ZZRESULT GetZipItem(HZIP hz, int index, ZIPENTRY *ze);
// GetZipItem - call this to get information about an item in the zip.
// If index is -1 and the file wasn't opened through a pipe,
// then it returns information about the whole zipfile
// (and in particular ze.index returns the number of index items).
// Note: the item might be a directory (ze.attr & FILE_ATTRIBUTE_DIRECTORY)
// See below for notes on what happens when you unzip such an item.
// Note: if you are opening the zip through a pipe, then random access
// is not possible and GetZipItem(-1) fails and you can't discover the number
// of items except by calling GetZipItem on each one of them in turn,
// starting at 0, until eventually the call fails. Also, in the event that
// you are opening through a pipe and the zip was itself created into a pipe,
// then then comp_size and sometimes unc_size as well may not be known until
// after the item has been unzipped.

ZZRESULT FindZipItem(HZIP hz, const ZTCHAR *name, bool ic, int *index, ZIPENTRY *ze);
// FindZipItem - finds an item by name. ic means 'insensitive to case'.
// It returns the index of the item, and returns information about it.
// If nothing was found, then index is set to -1 and the function returns
// an error code.

ZZRESULT UnzipItem(HZIP hz, int index, const ZTCHAR *fn);
ZZRESULT UnzipItem(HZIP hz, int index, void *z,unsigned int len);
ZZRESULT UnzipItemHandle(HZIP hz, int index, ZHANDLE h);
// UnzipItem - given an index to an item, unzips it. You can unzip to:
// to a pipe:             UnzipItemHandle(hz,i, hpipe_write);
// to a file (by handle): UnzipItemHandle(hz,i, hfile);
// to a file (by name):   UnzipItem(hz,i, ze.name);
// to a memory block:     UnzipItem(hz,i, buf,buflen);
// In the final case, if the buffer isn't large enough to hold it all,
// then the return code indicates that more is yet to come. If it was
// large enough, and you want to know precisely how big, GetZipItem.
// Note: zip files are normally stored with relative pathnames. If you
// unzip with ZIP_FILENAME a relative pathname then the item gets created
// relative to the current directory - it first ensures that all necessary
// subdirectories have been created. Also, the item may itself be a directory.
// If you unzip a directory with ZIP_FILENAME, then the directory gets created.
// If you unzip it to a handle or a memory block, then nothing gets created
// and it emits 0 bytes.
ZZRESULT SetUnzipBaseDir(HZIP hz, const ZTCHAR *dir);
// if unzipping to a filename, and it's a relative filename, then it will be relative to here.
// (defaults to current-directory).

#define ZZR_PASSWORD   0x00001000     // we didn't get the right password to unzip the file

// e.g.
//
// SetCurrentDirectory("c:\\docs\\stuff");
// HZIP hz = OpenZip("c:\\stuff.zip",0);
// ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
// for (int i=0; i<numitems; i++)
// { GetZipItem(hz,i,&ze);
//   UnzipItem(hz,i,ze.name);
// }
// CloseZip(hz);
//
//
// HRSRC hrsrc = FindResource(hInstance,MAKEINTRESOURCE(1),RT_RCDATA);
// ZHANDLE hglob = LoadResource(hInstance,hrsrc);
// void *zipbuf=LockResource(hglob);
// unsigned int ziplen=SizeofResource(hInstance,hrsrc);
// HZIP hz = OpenZip(zipbuf, ziplen, 0);
//   - unzip to a membuffer -
// ZIPENTRY ze; int i; FindZipItem(hz,"file.dat",true,&i,&ze);
// char *ibuf = new char[ze.unc_size];
// UnzipItem(hz,i, ibuf, ze.unc_size);
// delete[] ibuf;
//   - unzip to a fixed membuff -
// ZIPENTRY ze; int i; FindZipItem(hz,"file.dat",true,&i,&ze);
// char ibuf[1024]; ZZRESULT zr=ZR_MORE; unsigned long totsize=0;
// while (zr==ZR_MORE)
// { zr = UnzipItem(hz,i, ibuf,1024);
//   unsigned long bufsize=1024; if (zr==ZR_OK) bufsize=ze.unc_size-totsize;
//   totsize+=bufsize;
// }
//   - unzip to a pipe -
// ZHANDLE hwrite; HANDLE hthread=CreateWavReaderThread(&hwrite);
// int i; ZIPENTRY ze; FindZipItem(hz,"sound.wav",true,&i,&ze);
// UnzipItemHandle(hz,i, hwrite);
// CloseHandle(hwrite);
// WaitForSingleObject(hthread,INFINITE);
// CloseHandle(hwrite); CloseHandle(hthread);
//   - finished -
// CloseZip(hz);
// // note: no need to free resources obtained through Find/Load/LockResource
//
//
// SetCurrentDirectory("c:\\docs\\pipedzipstuff");
// ZHANDLE hread,hwrite; CreatePipe(&hread,&hwrite,0,0);
// CreateZipWriterThread(hwrite);
// HZIP hz = OpenZipHandle(hread,0);
// for (int i=0; ; i++)
// { ZIPENTRY ze;
//   ZZRESULT zr=GetZipItem(hz,i,&ze); if (zr!=ZR_OK) break; // no more
//   UnzipItem(hz,i, ze.name);
// }
// CloseZip(hz);
//
//




// Now we indulge in a little skullduggery so that the code works whether
// the user has included just zip or both zip and unzip.
// Idea: if header files for both zip and unzip are present, then presumably
// the cpp files for zip and unzip are both present, so we will call
// one or the other of them based on a dynamic choice. If the header file
// for only one is present, then we will bind to that particular one.
ZZRESULT CloseZipU(HZIP hz);
unsigned int ZFormatZipMessageU(ZZRESULT code, ZTCHAR *buf,unsigned int len);
bool IsZipHandleU(HZIP hz);
#ifdef __ZIP_H
#undef CloseZip
#define CloseZip(hz) (IsZipHandleU(hz)?CloseZipU(hz):CloseZipZ(hz))
#else
#define CloseZip CloseZipU
#define FormatZipMessage FormatZipMessageU
#endif


#ifdef ZIP_STD
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#ifdef _MSC_VER
#include <sys/utime.h> // microsoft puts it here
#else
#include <utime.h>
#endif
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
#include <direct.h>
#define lumkdir(t) (mkdir(t))
#else
#include <unistd.h>
#define lumkdir(t) (mkdir(t,0755))
#endif
#include <sys/types.h>
#include <sys/stat.h>

#else
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#endif

// THIS FILE is almost entirely based upon code by Jean-loup Gailly
// and Mark Adler. It has been modified by Lucian Wischik.
// The modifications were: incorporate the bugfixes of 1.1.4, allow
// unzipping to/from handles/pipes/files/memory, encryption, unicode,
// a windowsish api, and putting everything into a single .cpp file.
// The original code may be found at http://www.gzip.org/zlib/
// The original copyright text follows.
//
//
//
// zlib.h -- interface of the 'zlib' general purpose compression library
//  version 1.1.3, July 9th, 1998
//
//  Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.
//
//  Jean-loup Gailly        Mark Adler
//  jloup@gzip.org          madler@alumni.caltech.edu
//
//
//  The data format used by the zlib library is described by RFCs (Request for
//  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
//  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
//
//
//     The 'zlib' compression library provides in-memory compression and
//  decompression functions, including integrity checks of the uncompressed
//  data.  This version of the library supports only one compression method
//  (deflation) but other algorithms will be added later and will have the same
//  stream interface.
//
//     Compression can be done in a single step if the buffers are large
//  enough (for example if an input file is mmap'ed), or can be done by
//  repeated calls of the compression function.  In the latter case, the
//  application must provide more input and/or consume the output
//  (providing more output space) before each call.
//
//     The library also supports reading and writing files in gzip (.gz) format
//  with an interface similar to that of stdio.
//
//     The library does not install any signal handler. The decoder checks
//  the consistency of the compressed data, so the library should never
//  crash even in case of corrupted input.
//
// for more info about .ZIP format, see ftp://ftp.cdrom.com/pub/infozip/doc/appnote-970311-iz.zip
//   PkWare has also a specification at ftp://ftp.pkware.com/probdesc.zip

#define ZZIP_HANDLE   1
#define ZZIP_FILENAME 2
#define ZZIP_MEMORY   3


#define zzmalloc(len) malloc(len)

#define zzfree(p) free(p)

/*
void *zmalloc(unsigned int len)
{ char *buf = new char[len+32];
  for (int i=0; i<16; i++)
  { buf[i]=i;
    buf[len+31-i]=i;
  }
  *((unsigned int*)buf) = len;
  char c[1000]; wsprintf(c,"malloc 0x%lx  - %lu",buf+16,len);
  OutputDebugString(c);
  return buf+16;
}

void zfree(void *buf)
{ char c[1000]; wsprintf(c,"free   0x%lx",buf);
  OutputDebugString(c);
  char *p = ((char*)buf)-16;
  unsigned int len = *((unsigned int*)p);
  bool blown=false;
  for (int i=0; i<16; i++)
  { char lo = p[i];
    char hi = p[len+31-i];
    if (hi!=i || (lo!=i && i>4)) blown=true;
  }
  if (blown)
  { OutputDebugString("BLOWN!!!");
  }
  delete[] p;
}
*/


typedef struct ztm_unz_s
{ unsigned int tm_sec;            // seconds after the minute - [0,59]
  unsigned int tm_min;            // minutes after the hour - [0,59]
  unsigned int tm_hour;           // hours since midnight - [0,23]
  unsigned int tm_mday;           // day of the month - [1,31]
  unsigned int tm_mon;            // months since January - [0,11]
  unsigned int tm_year;           // years - [1980..2044]
} ztm_unz;



// ----------------------------------------------------------------------
// some windows<->linux portability things
#ifdef ZIP_STD
ZDWORD ZGetFilePosU(ZHANDLE hfout)
{ struct stat st; fstat(fileno(hfout),&st);
  if ((st.st_mode&S_IFREG)==0) return 0xFFFFFFFF;
  return ftell(hfout);
}

bool ZFileExists(const ZTCHAR *fn)
{ struct stat st;
  int res=stat(fn,&st);
  return (res==0);
}

ZFILETIME zdosdatetime2filetime(ZWORD dosdate,ZWORD dostime)
{ struct tm t;
  t.tm_year = (ZWORD)(((dosdate>>9)&0x7f) + 1980 - 1900);
  t.tm_isdst = -1;
  t.tm_mon = (ZWORD)((dosdate>>5)&0xf - 1);
  t.tm_mday = (ZWORD)(dosdate&0x1f);
  t.tm_hour = (ZWORD)((dostime>>11)&0x1f);
  t.tm_min = (ZWORD)((dostime>>5)&0x3f);
  t.tm_sec = (ZWORD)((dostime&0x1f)*2);
  time_t t2 = mktime(&t);
  return t2;
}

void ZLocalFileTimeToFileTime(ZFILETIME *lft, ZFILETIME *ft)
{ *ft = *lft;
}

ZFILETIME ztimet2filetime(const z_l_utime_t t)
{ return t;
}

#else
// ----------------------------------------------------------------------
ZDWORD ZGetFilePosU(ZHANDLE hfout)
{ return SetFilePointer(hfout,0,0,FILE_CURRENT);
}

ZFILETIME ztimet2filetime(const z_l_utime_t t)
{ LONGLONG i = Int32x32To64(t,10000000) + 116444736000000000LL;
  FILETIME ft;
  ft.dwLowDateTime = (ZDWORD) i;
  ft.dwHighDateTime = (ZDWORD)(i >>32);
  return ft;
}

ZFILETIME zdosdatetime2filetime(ZWORD dosdate,ZWORD dostime)
{ // date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
  // time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
  SYSTEMTIME st;
  st.wYear = (ZWORD)(((dosdate>>9)&0x7f) + 1980);
  st.wMonth = (ZWORD)((dosdate>>5)&0xf);
  st.wDay = (ZWORD)(dosdate&0x1f);
  st.wHour = (ZWORD)((dostime>>11)&0x1f);
  st.wMinute = (ZWORD)((dostime>>5)&0x3f);
  st.wSecond = (ZWORD)((dostime&0x1f)*2);
  st.wMilliseconds = 0;
  ZFILETIME ft; SystemTimeToFileTime(&st,&ft);
  return ft;
}

bool ZFileExists(const ZTCHAR *fn)
{ return (GetFileAttributes(fn)!=0xFFFFFFFF);
}
#endif
// ----------------------------------------------------------------------



// unz_global_info structure contain global data about the ZIPfile
typedef struct zunz_global_info_s
{ unsigned long number_entry;         // total number of entries in the central dir on this disk
  unsigned long size_comment;         // size of the global comment of the zipfile
} zunz_global_info;

// unz_file_info contain information about a file in the zipfile
typedef struct zunz_file_info_s
{ unsigned long version;              // version made by                 2 bytes
  unsigned long version_needed;       // version needed to extract       2 bytes
  unsigned long flag;                 // general purpose bit flag        2 bytes
  unsigned long compression_method;   // compression method              2 bytes
  unsigned long dosDate;              // last mod file date in Dos fmt   4 bytes
  unsigned long crc;                  // crc-32                          4 bytes
  unsigned long compressed_size;      // compressed size                 4 bytes
  unsigned long uncompressed_size;    // uncompressed size               4 bytes
  unsigned long size_filename;        // filename length                 2 bytes
  unsigned long size_file_extra;      // extra field length              2 bytes
  unsigned long size_file_comment;    // file comment length             2 bytes
  unsigned long disk_num_start;       // disk number start               2 bytes
  unsigned long internal_fa;          // internal file attributes        2 bytes
  unsigned long external_fa;          // external file attributes        4 bytes
  ztm_unz tmu_date;
} zunz_file_info;


#define ZUNZ_OK                  (0)
#define ZUNZ_END_OF_LIST_OF_FILE (-100)
#define ZUNZ_ERRNO               (ZZ_ERRNO)
#define ZUNZ_EOF                 (0)
#define ZUNZ_PARAMERROR          (-102)
#define ZUNZ_BADZIPFILE          (-103)
#define ZUNZ_INTERNALERROR       (-104)
#define ZUNZ_CRCERROR            (-105)
#define ZUNZ_PASSWORD            (-106)

#define ZZLIB_VERSION "1.X.X"


// Allowed flush values; see deflate() for details
#define ZZ_NO_FLUSH      0
#define ZZ_SYNC_FLUSH    2
#define ZZ_FULL_FLUSH    3
#define ZZ_FINISH        4


// compression levels
#define ZZ_NO_COMPRESSION         0
#define ZZ_BEST_SPEED             1
#define ZZ_BEST_COMPRESSION       9
#define ZZ_DEFAULT_COMPRESSION  (-1)

// compression strategy; see deflateInit2() for details
#define ZZ_FILTERED            1
#define ZZ_HUFFMAN_ONLY        2
#define ZZ_DEFAULT_STRATEGY    0

// Possible values of the data_type field
#define ZZ_BINARY   0
#define ZZ_ASCII    1
#define ZZ_UNKNOWN  2

// The deflate compression method (the only one supported in this version)
#define ZZ_DEFLATED   8

// for initializing zalloc, zfree, opaque
#define ZZ_NULL  0

// case sensitivity when searching for filenames
#define ZCASE_SENSITIVE 1
#define ZCASE_INSENSITIVE 2


// Return codes for the compression/decompression functions. Negative
// values are errors, positive values are used for special but normal events.
#define ZZ_OK            0
#define ZZ_STREAM_END    1
#define ZZ_NEED_DICT     2
#define ZZ_ERRNO        (-1)
#define ZZ_STREAM_ERROR (-2)
#define ZZ_DATA_ERROR   (-3)
#define ZZ_MEM_ERROR    (-4)
#define ZZ_BUF_ERROR    (-5)
#define ZZ_VERSION_ERROR (-6)

typedef voidpf (*zalloc_func) (voidpf opaque, uInt items, uInt size);
typedef void   (*zfree_func)  (voidpf opaque, voidpf address);

struct zinternal_state;

typedef struct zz_stream_s {
    uChar    *next_in;  // next input byte
    uInt     avail_in;  // number of bytes available at next_in
    uLong    total_in;  // total nb of input bytes read so far

    uChar    *next_out; // next output byte should be put there
    uInt     avail_out; // remaining free space at next_out
    uLong    total_out; // total nb of bytes output so far

    char     *msg;      // last error message, NULL if no error
    struct zinternal_state *state; // not visible by applications

    zalloc_func zalloc;  // used to allocate the internal state
    zfree_func  zfree;   // used to free the internal state
    voidpf     opaque;  // private data object passed to zalloc and zfree

    int     data_type;  // best guess about the data type: ascii or binary
    uLong   adler;      // adler32 value of the uncompressed data
    uLong   reserved;   // reserved for future use
} zz_stream;

typedef zz_stream *zz_stream_ptr;


//   The application must update next_in and avail_in when avail_in has
//   dropped to zero. It must update next_out and avail_out when avail_out
//   has dropped to zero. The application must initialize zalloc, zfree and
//   opaque before calling the init function. All other fields are set by the
//   compression library and must not be updated by the application.
//
//   The opaque value provided by the application will be passed as the first
//   parameter for calls of zalloc and zfree. This can be useful for custom
//   memory management. The compression library attaches no meaning to the
//   opaque value.
//
//   zalloc must return Z_NULL if there is not enough memory for the object.
//   If zlib is used in a multi-threaded application, zalloc and zfree must be
//   thread safe.
//
//   The fields total_in and total_out can be used for statistics or
//   progress reports. After compression, total_in holds the total size of
//   the uncompressed data and may be saved for use in the decompressor
//   (particularly if the decompressor wants to decompress everything in
//   a single step).
//


// basic functions

const char *zzlibVersion ();
// The application can compare zlibVersion and ZLIB_VERSION for consistency.
// If the first character differs, the library code actually used is
// not compatible with the zlib.h header file used by the application.
// This check is automatically made by inflateInit.






int zinflate (zz_stream_ptr strm, int flush);
//
//    inflate decompresses as much data as possible, and stops when the input
//  buffer becomes empty or the output buffer becomes full. It may some
//  introduce some output latency (reading input without producing any output)
//  except when forced to flush.
//
//  The detailed semantics are as follows. inflate performs one or both of the
//  following actions:
//
//  - Decompress more input starting at next_in and update next_in and avail_in
//    accordingly. If not all input can be processed (because there is not
//    enough room in the output buffer), next_in is updated and processing
//    will resume at this point for the next call of inflate().
//
//  - Provide more output starting at next_out and update next_out and avail_out
//    accordingly.  inflate() provides as much output as possible, until there
//    is no more input data or no more space in the output buffer (see below
//    about the flush parameter).
//
//  Before the call of inflate(), the application should ensure that at least
//  one of the actions is possible, by providing more input and/or consuming
//  more output, and updating the next_* and avail_* values accordingly.
//  The application can consume the uncompressed output when it wants, for
//  example when the output buffer is full (avail_out == 0), or after each
//  call of inflate(). If inflate returns Z_OK and with zero avail_out, it
//  must be called again after making room in the output buffer because there
//  might be more output pending.
//
//    If the parameter flush is set to Z_SYNC_FLUSH, inflate flushes as much
//  output as possible to the output buffer. The flushing behavior of inflate is
//  not specified for values of the flush parameter other than Z_SYNC_FLUSH
//  and Z_FINISH, but the current implementation actually flushes as much output
//  as possible anyway.
//
//    inflate() should normally be called until it returns Z_STREAM_END or an
//  error. However if all decompression is to be performed in a single step
//  (a single call of inflate), the parameter flush should be set to
//  Z_FINISH. In this case all pending input is processed and all pending
//  output is flushed; avail_out must be large enough to hold all the
//  uncompressed data. (The size of the uncompressed data may have been saved
//  by the compressor for this purpose.) The next operation on this stream must
//  be inflateEnd to deallocate the decompression state. The use of Z_FINISH
//  is never required, but can be used to inform inflate that a faster routine
//  may be used for the single inflate() call.
//
//     If a preset dictionary is needed at this point (see inflateSetDictionary
//  below), inflate sets strm-adler to the adler32 checksum of the
//  dictionary chosen by the compressor and returns Z_NEED_DICT; otherwise
//  it sets strm->adler to the adler32 checksum of all output produced
//  so far (that is, total_out bytes) and returns Z_OK, Z_STREAM_END or
//  an error code as described below. At the end of the stream, inflate()
//  checks that its computed adler32 checksum is equal to that saved by the
//  compressor and returns Z_STREAM_END only if the checksum is correct.
//
//    inflate() returns Z_OK if some progress has been made (more input processed
//  or more output produced), Z_STREAM_END if the end of the compressed data has
//  been reached and all uncompressed output has been produced, Z_NEED_DICT if a
//  preset dictionary is needed at this point, Z_DATA_ERROR if the input data was
//  corrupted (input stream not conforming to the zlib format or incorrect
//  adler32 checksum), Z_STREAM_ERROR if the stream structure was inconsistent
//  (for example if next_in or next_out was NULL), Z_MEM_ERROR if there was not
//  enough memory, Z_BUF_ERROR if no progress is possible or if there was not
//  enough room in the output buffer when Z_FINISH is used. In the Z_DATA_ERROR
//  case, the application may then call inflateSync to look for a good
//  compression block.
//


int zinflateEnd (zz_stream_ptr strm);
//
//     All dynamically allocated data structures for this stream are freed.
//   This function discards any unprocessed input and does not flush any
//   pending output.
//
//     inflateEnd returns Z_OK if success, Z_STREAM_ERROR if the stream state
//   was inconsistent. In the error case, msg may be set but then points to a
//   static string (which must not be deallocated).

                        // Advanced functions

//  The following functions are needed only in some special applications.





int zinflateSetDictionary (zz_stream_ptr strm,
                                             const uChar *dictionary,
                                             uInt  dictLength);
//
//     Initializes the decompression dictionary from the given uncompressed byte
//   sequence. This function must be called immediately after a call of inflate
//   if this call returned Z_NEED_DICT. The dictionary chosen by the compressor
//   can be determined from the Adler32 value returned by this call of
//   inflate. The compressor and decompressor must use exactly the same
//   dictionary.
//
//     inflateSetDictionary returns Z_OK if success, Z_STREAM_ERROR if a
//   parameter is invalid (such as NULL dictionary) or the stream state is
//   inconsistent, Z_DATA_ERROR if the given dictionary doesn't match the
//   expected one (incorrect Adler32 value). inflateSetDictionary does not
//   perform any decompression: this will be done by subsequent calls of
//   inflate().


int zinflateSync (zz_stream_ptr strm);
//
//    Skips invalid compressed data until a full flush point can be found, or until all
//  available input is skipped. No output is provided.
//
//    inflateSync returns Z_OK if a full flush point has been found, Z_BUF_ERROR
//  if no more input was provided, Z_DATA_ERROR if no flush point has been found,
//  or Z_STREAM_ERROR if the stream structure was inconsistent. In the success
//  case, the application may save the current current value of total_in which
//  indicates where valid compressed data was found. In the error case, the
//  application may repeatedly call inflateSync, providing more input each time,
//  until success or end of the input data.


int zinflateReset (zz_stream_ptr strm);
//     This function is equivalent to inflateEnd followed by inflateInit,
//   but does not free and reallocate all the internal decompression state.
//   The stream will keep attributes that may have been set by inflateInit2.
//
//      inflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
//   stream state was inconsistent (such as zalloc or state being NULL).
//



// checksum functions
// These functions are not related to compression but are exported
// anyway because they might be useful in applications using the
// compression library.

uLong zadler32 (uLong adler, const uChar *buf, uInt len);
//     Update a running Adler-32 checksum with the bytes buf[0..len-1] and
//   return the updated checksum. If buf is NULL, this function returns
//   the required initial value for the checksum.
//   An Adler-32 checksum is almost as reliable as a CRC32 but can be computed
//   much faster. Usage example:
//
//     uLong adler = adler32(0L, Z_NULL, 0);
//
//     while (read_buffer(buffer, length) != EOF) {
//       adler = adler32(adler, buffer, length);
//     }
//     if (adler != original_adler) error();

uLong zucrc32   (uLong crc, const uChar *buf, uInt len);
//     Update a running crc with the bytes buf[0..len-1] and return the updated
//   crc. If buf is NULL, this function returns the required initial value
//   for the crc. Pre- and post-conditioning (one's complement) is performed
//   within this function so it shouldn't be done by the application.
//   Usage example:
//
//     uLong crc = crc32(0L, Z_NULL, 0);
//
//     while (read_buffer(buffer, length) != EOF) {
//       crc = crc32(crc, buffer, length);
//     }
//     if (crc != original_crc) error();




const char   *zzError           (int err);
int           zinflateSyncPoint (zz_stream_ptr z);

const char * const zz_errmsg[10] = { // indexed by 2-zlib_error
"need dictionary",     // Z_NEED_DICT       2
"stream end",          // Z_STREAM_END      1
"",                    // Z_OK              0
"file error",          // Z_ERRNO         (-1)
"stream error",        // Z_STREAM_ERROR  (-2)
"data error",          // Z_DATA_ERROR    (-3)
"insufficient memory", // Z_MEM_ERROR     (-4)
"buffer error",        // Z_BUF_ERROR     (-5)
"incompatible version",// Z_VERSION_ERROR (-6)
""};


#define ZERR_MSG(err) zz_errmsg[ZZ_NEED_DICT-(err)]

#define ZERR_RETURN(strm,err) \
  return (strm->msg = (char*)ZERR_MSG(err), (err))
// To be used only when the state is known to be valid

        // common constants


#define ZSTORED_BLOCK 0
#define ZSTATIC_TREES 1
#define ZDYN_TREES    2
// The three kinds of block type

#define ZMIN_MATCH  3
#define ZMAX_MATCH  258
// The minimum and maximum match lengths

#define ZPRESET_DICT 0x20 // preset dictionary flag in zlib header

        // target dependencies

#define ZOS_CODE  0x0b  // Window 95 & Windows NT



         // functions

#define zzmemzero(dest, len) memset(dest, 0, len)

// Diagnostic functions
#define LuAssert(cond,msg)
#define LuTrace(x)
#define LuTracev(x)
#define LuTracevv(x)
#define LuTracec(c,x)
#define LuTracecv(c,x)


typedef uLong (*zcheck_func) (uLong check, const uChar *buf, uInt len);
voidpf zzcalloc (voidpf opaque, unsigned items, unsigned size);
void   zzcfree  (voidpf opaque, voidpf ptr);

#define ZZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))

//void ZZFREE(z_streamp strm,voidpf addr)
//{ *((strm)->zfree))((strm)->opaque, addr);
//}

#define ZTRY_FREE(s, p) {if (p) ZFREE(s, p);}




// Huffman code lookup table entry--this entry is four bytes for machines
// that have 16-bit pointers (e.g. PC's in the small or medium model).


typedef struct zinflate_huft_s zinflate_huft;

struct zinflate_huft_s {
  union {
    struct {
      uChar Exop;        // number of extra bits or operation
      uChar Bits;        // number of bits in this code or subcode
    } what;
    uInt pad;           // pad structure to a power of 2 (4 bytes for
  } word;               //  16-bit, 8 bytes for 32-bit int's)
  uInt base;            // literal, length base, distance base, or table offset
};

// Maximum size of dynamic tree.  The maximum found in a long but non-
//   exhaustive search was 1004 huft structures (850 for length/literals
//   and 154 for distances, the latter actually the result of an
//   exhaustive search).  The actual maximum is not known, but the
//   value below is more than safe.
#define ZMANY 1440

int zinflate_trees_bits (
    uInt *,                    // 19 code lengths
    uInt *,                    // bits tree desired/actual depth
    zinflate_huft * *,       // bits tree result
    zinflate_huft *,             // space for trees
    zz_stream_ptr);                // for messages

int zinflate_trees_dynamic (
    uInt,                       // number of literal/length codes
    uInt,                       // number of distance codes
    uInt *,                    // that many (total) code lengths
    uInt *,                    // literal desired/actual bit depth
    uInt *,                    // distance desired/actual bit depth
    zinflate_huft * *,       // literal/length tree result
    zinflate_huft * *,       // distance tree result
    zinflate_huft *,             // space for trees
    zz_stream_ptr);                // for messages

int zinflate_trees_fixed (
    uInt *,                    // literal desired/actual bit depth
    uInt *,                    // distance desired/actual bit depth
    const zinflate_huft * *,       // literal/length tree result
    const zinflate_huft * *,       // distance tree result
    zz_stream_ptr);                // for memory allocation





struct zinflate_blocks_state;
typedef struct zinflate_blocks_state zinflate_blocks_statef;

zinflate_blocks_statef * zinflate_blocks_new (
    zz_stream_ptr z,
    zcheck_func c,               // check function
    uInt w);                   // window size

int zinflate_blocks (
    zinflate_blocks_statef *,
    zz_stream_ptr ,
    int);                      // initial return code

void zinflate_blocks_reset (
    zinflate_blocks_statef *,
    zz_stream_ptr ,
    uLong *);                  // check value on output

int zinflate_blocks_free (
    zinflate_blocks_statef *,
    zz_stream_ptr);

void zinflate_set_dictionary (
    zinflate_blocks_statef *s,
    const uChar *d,  // dictionary
    uInt  n);       // dictionary length

int zinflate_blocks_sync_point (
    zinflate_blocks_statef *s);

struct zinflate_codes_state;
typedef struct zinflate_codes_state zinflate_codes_statef;

zinflate_codes_statef *zinflate_codes_new (
    uInt, uInt,
    const zinflate_huft *, const zinflate_huft *,
    zz_stream_ptr );

int zinflate_codes (
    zinflate_blocks_statef *,
    zz_stream_ptr ,
    int);

void zinflate_codes_free (
    zinflate_codes_statef *,
    zz_stream_ptr );

typedef enum {
      ZIBM_TYPE,     // get type bits (3, including end bit)
      ZIBM_LENS,     // get lengths for stored
      ZIBM_STORED,   // processing stored block
      ZIBM_TABLE,    // get table lengths
      ZIBM_BTREE,    // get bit lengths tree for a dynamic block
      ZIBM_DTREE,    // get length, distance trees for a dynamic block
      ZIBM_CODES,    // processing fixed or dynamic block
      ZIBM_DRY,      // output remaining window bytes
      ZIBM_DONE,     // finished last block, done
      ZIBM_BAD}      // got a data error--stuck here
zinflate_block_mode;

// inflate blocks semi-private state
struct zinflate_blocks_state {

  // mode
  zinflate_block_mode  mode;     // current inflate_block mode

  // mode dependent information
  union {
    uInt left;          // if STORED, bytes left to copy
    struct {
      uInt table;               // table lengths (14 bits)
      uInt index;               // index into blens (or border)
      uInt *blens;             // bit lengths of codes
      uInt bb;                  // bit length tree depth
      zinflate_huft *tb;         // bit length decoding tree
    } trees;            // if DTREE, decoding info for trees
    struct {
      zinflate_codes_statef
         *codes;
    } decode;           // if CODES, current state
  } sub;                // submode
  uInt last;            // true if this block is the last block

  // mode independent information
  uInt bitk;            // bits in bit buffer
  uLong bitb;           // bit buffer
  zinflate_huft *hufts;  // single malloc for tree space
  uChar *window;        // sliding window
  uChar *end;           // one byte after sliding window
  uChar *read;          // window read pointer
  uChar *write;         // window write pointer
  zcheck_func checkfn;   // check function
  uLong check;          // check on output

};


// defines for inflate input/output
//   update pointers and return
#define ZUPDBITS {s->bitb=b;s->bitk=k;}
#define ZUPDIN {z->avail_in=n;z->total_in+=(uLong)(p-z->next_in);z->next_in=p;}
#define ZUPDOUT {s->write=q;}
#define ZUPDATE {ZUPDBITS ZUPDIN ZUPDOUT}
#define ZLEAVE {ZUPDATE return zinflate_flush(s,z,r);}
//   get bytes and bits
#define ZLOADIN {p=z->next_in;n=z->avail_in;b=s->bitb;k=s->bitk;}
#define ZNEEDBYTE {if(n)r=ZZ_OK;else ZLEAVE}
#define ZNEXTBYTE (n--,*p++)
#define ZNEEDBITS(j) {while(k<(j)){ZNEEDBYTE;b|=((uLong)ZNEXTBYTE)<<k;k+=8;}}
#define ZDUMPBITS(j) {b>>=(j);k-=(j);}
//   output bytes
#define ZWAVAIL (uInt)(q<s->read?s->read-q-1:s->end-q)
#define ZLOADOUT {q=s->write;m=(uInt)ZWAVAIL;m;}
#define ZWRAP {if(q==s->end&&s->read!=s->window){q=s->window;m=(uInt)ZWAVAIL;}}
#define ZFLUSH {ZUPDOUT r=zinflate_flush(s,z,r); ZLOADOUT}
#define ZNEEDOUT {if(m==0){ZWRAP if(m==0){ZFLUSH ZWRAP if(m==0) ZLEAVE}}r=ZZ_OK;}
#define ZOUTBYTE(a) {*q++=(uChar)(a);m--;}
//   load local pointers
#define ZLOAD {ZLOADIN ZLOADOUT}

// masks for lower bits (size given to avoid silly warnings with Visual C++)
// And'ing with mask[n] masks the lower n bits
const uInt zinflate_mask[17] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

// copy as much as possible from the sliding window to the output area
int zinflate_flush (zinflate_blocks_statef *, zz_stream_ptr, int);

int zinflate_fast (uInt, uInt, const zinflate_huft *, const zinflate_huft *, zinflate_blocks_statef *, zz_stream_ptr );

const uInt zfixed_bl = 9;
const uInt zfixed_bd = 5;
const zinflate_huft zfixed_tl[] = {
    {{{96,7}},256}, {{{0,8}},80}, {{{0,8}},16}, {{{84,8}},115},
    {{{82,7}},31}, {{{0,8}},112}, {{{0,8}},48}, {{{0,9}},192},
    {{{80,7}},10}, {{{0,8}},96}, {{{0,8}},32}, {{{0,9}},160},
    {{{0,8}},0}, {{{0,8}},128}, {{{0,8}},64}, {{{0,9}},224},
    {{{80,7}},6}, {{{0,8}},88}, {{{0,8}},24}, {{{0,9}},144},
    {{{83,7}},59}, {{{0,8}},120}, {{{0,8}},56}, {{{0,9}},208},
    {{{81,7}},17}, {{{0,8}},104}, {{{0,8}},40}, {{{0,9}},176},
    {{{0,8}},8}, {{{0,8}},136}, {{{0,8}},72}, {{{0,9}},240},
    {{{80,7}},4}, {{{0,8}},84}, {{{0,8}},20}, {{{85,8}},227},
    {{{83,7}},43}, {{{0,8}},116}, {{{0,8}},52}, {{{0,9}},200},
    {{{81,7}},13}, {{{0,8}},100}, {{{0,8}},36}, {{{0,9}},168},
    {{{0,8}},4}, {{{0,8}},132}, {{{0,8}},68}, {{{0,9}},232},
    {{{80,7}},8}, {{{0,8}},92}, {{{0,8}},28}, {{{0,9}},152},
    {{{84,7}},83}, {{{0,8}},124}, {{{0,8}},60}, {{{0,9}},216},
    {{{82,7}},23}, {{{0,8}},108}, {{{0,8}},44}, {{{0,9}},184},
    {{{0,8}},12}, {{{0,8}},140}, {{{0,8}},76}, {{{0,9}},248},
    {{{80,7}},3}, {{{0,8}},82}, {{{0,8}},18}, {{{85,8}},163},
    {{{83,7}},35}, {{{0,8}},114}, {{{0,8}},50}, {{{0,9}},196},
    {{{81,7}},11}, {{{0,8}},98}, {{{0,8}},34}, {{{0,9}},164},
    {{{0,8}},2}, {{{0,8}},130}, {{{0,8}},66}, {{{0,9}},228},
    {{{80,7}},7}, {{{0,8}},90}, {{{0,8}},26}, {{{0,9}},148},
    {{{84,7}},67}, {{{0,8}},122}, {{{0,8}},58}, {{{0,9}},212},
    {{{82,7}},19}, {{{0,8}},106}, {{{0,8}},42}, {{{0,9}},180},
    {{{0,8}},10}, {{{0,8}},138}, {{{0,8}},74}, {{{0,9}},244},
    {{{80,7}},5}, {{{0,8}},86}, {{{0,8}},22}, {{{192,8}},0},
    {{{83,7}},51}, {{{0,8}},118}, {{{0,8}},54}, {{{0,9}},204},
    {{{81,7}},15}, {{{0,8}},102}, {{{0,8}},38}, {{{0,9}},172},
    {{{0,8}},6}, {{{0,8}},134}, {{{0,8}},70}, {{{0,9}},236},
    {{{80,7}},9}, {{{0,8}},94}, {{{0,8}},30}, {{{0,9}},156},
    {{{84,7}},99}, {{{0,8}},126}, {{{0,8}},62}, {{{0,9}},220},
    {{{82,7}},27}, {{{0,8}},110}, {{{0,8}},46}, {{{0,9}},188},
    {{{0,8}},14}, {{{0,8}},142}, {{{0,8}},78}, {{{0,9}},252},
    {{{96,7}},256}, {{{0,8}},81}, {{{0,8}},17}, {{{85,8}},131},
    {{{82,7}},31}, {{{0,8}},113}, {{{0,8}},49}, {{{0,9}},194},
    {{{80,7}},10}, {{{0,8}},97}, {{{0,8}},33}, {{{0,9}},162},
    {{{0,8}},1}, {{{0,8}},129}, {{{0,8}},65}, {{{0,9}},226},
    {{{80,7}},6}, {{{0,8}},89}, {{{0,8}},25}, {{{0,9}},146},
    {{{83,7}},59}, {{{0,8}},121}, {{{0,8}},57}, {{{0,9}},210},
    {{{81,7}},17}, {{{0,8}},105}, {{{0,8}},41}, {{{0,9}},178},
    {{{0,8}},9}, {{{0,8}},137}, {{{0,8}},73}, {{{0,9}},242},
    {{{80,7}},4}, {{{0,8}},85}, {{{0,8}},21}, {{{80,8}},258},
    {{{83,7}},43}, {{{0,8}},117}, {{{0,8}},53}, {{{0,9}},202},
    {{{81,7}},13}, {{{0,8}},101}, {{{0,8}},37}, {{{0,9}},170},
    {{{0,8}},5}, {{{0,8}},133}, {{{0,8}},69}, {{{0,9}},234},
    {{{80,7}},8}, {{{0,8}},93}, {{{0,8}},29}, {{{0,9}},154},
    {{{84,7}},83}, {{{0,8}},125}, {{{0,8}},61}, {{{0,9}},218},
    {{{82,7}},23}, {{{0,8}},109}, {{{0,8}},45}, {{{0,9}},186},
    {{{0,8}},13}, {{{0,8}},141}, {{{0,8}},77}, {{{0,9}},250},
    {{{80,7}},3}, {{{0,8}},83}, {{{0,8}},19}, {{{85,8}},195},
    {{{83,7}},35}, {{{0,8}},115}, {{{0,8}},51}, {{{0,9}},198},
    {{{81,7}},11}, {{{0,8}},99}, {{{0,8}},35}, {{{0,9}},166},
    {{{0,8}},3}, {{{0,8}},131}, {{{0,8}},67}, {{{0,9}},230},
    {{{80,7}},7}, {{{0,8}},91}, {{{0,8}},27}, {{{0,9}},150},
    {{{84,7}},67}, {{{0,8}},123}, {{{0,8}},59}, {{{0,9}},214},
    {{{82,7}},19}, {{{0,8}},107}, {{{0,8}},43}, {{{0,9}},182},
    {{{0,8}},11}, {{{0,8}},139}, {{{0,8}},75}, {{{0,9}},246},
    {{{80,7}},5}, {{{0,8}},87}, {{{0,8}},23}, {{{192,8}},0},
    {{{83,7}},51}, {{{0,8}},119}, {{{0,8}},55}, {{{0,9}},206},
    {{{81,7}},15}, {{{0,8}},103}, {{{0,8}},39}, {{{0,9}},174},
    {{{0,8}},7}, {{{0,8}},135}, {{{0,8}},71}, {{{0,9}},238},
    {{{80,7}},9}, {{{0,8}},95}, {{{0,8}},31}, {{{0,9}},158},
    {{{84,7}},99}, {{{0,8}},127}, {{{0,8}},63}, {{{0,9}},222},
    {{{82,7}},27}, {{{0,8}},111}, {{{0,8}},47}, {{{0,9}},190},
    {{{0,8}},15}, {{{0,8}},143}, {{{0,8}},79}, {{{0,9}},254},
    {{{96,7}},256}, {{{0,8}},80}, {{{0,8}},16}, {{{84,8}},115},
    {{{82,7}},31}, {{{0,8}},112}, {{{0,8}},48}, {{{0,9}},193},
    {{{80,7}},10}, {{{0,8}},96}, {{{0,8}},32}, {{{0,9}},161},
    {{{0,8}},0}, {{{0,8}},128}, {{{0,8}},64}, {{{0,9}},225},
    {{{80,7}},6}, {{{0,8}},88}, {{{0,8}},24}, {{{0,9}},145},
    {{{83,7}},59}, {{{0,8}},120}, {{{0,8}},56}, {{{0,9}},209},
    {{{81,7}},17}, {{{0,8}},104}, {{{0,8}},40}, {{{0,9}},177},
    {{{0,8}},8}, {{{0,8}},136}, {{{0,8}},72}, {{{0,9}},241},
    {{{80,7}},4}, {{{0,8}},84}, {{{0,8}},20}, {{{85,8}},227},
    {{{83,7}},43}, {{{0,8}},116}, {{{0,8}},52}, {{{0,9}},201},
    {{{81,7}},13}, {{{0,8}},100}, {{{0,8}},36}, {{{0,9}},169},
    {{{0,8}},4}, {{{0,8}},132}, {{{0,8}},68}, {{{0,9}},233},
    {{{80,7}},8}, {{{0,8}},92}, {{{0,8}},28}, {{{0,9}},153},
    {{{84,7}},83}, {{{0,8}},124}, {{{0,8}},60}, {{{0,9}},217},
    {{{82,7}},23}, {{{0,8}},108}, {{{0,8}},44}, {{{0,9}},185},
    {{{0,8}},12}, {{{0,8}},140}, {{{0,8}},76}, {{{0,9}},249},
    {{{80,7}},3}, {{{0,8}},82}, {{{0,8}},18}, {{{85,8}},163},
    {{{83,7}},35}, {{{0,8}},114}, {{{0,8}},50}, {{{0,9}},197},
    {{{81,7}},11}, {{{0,8}},98}, {{{0,8}},34}, {{{0,9}},165},
    {{{0,8}},2}, {{{0,8}},130}, {{{0,8}},66}, {{{0,9}},229},
    {{{80,7}},7}, {{{0,8}},90}, {{{0,8}},26}, {{{0,9}},149},
    {{{84,7}},67}, {{{0,8}},122}, {{{0,8}},58}, {{{0,9}},213},
    {{{82,7}},19}, {{{0,8}},106}, {{{0,8}},42}, {{{0,9}},181},
    {{{0,8}},10}, {{{0,8}},138}, {{{0,8}},74}, {{{0,9}},245},
    {{{80,7}},5}, {{{0,8}},86}, {{{0,8}},22}, {{{192,8}},0},
    {{{83,7}},51}, {{{0,8}},118}, {{{0,8}},54}, {{{0,9}},205},
    {{{81,7}},15}, {{{0,8}},102}, {{{0,8}},38}, {{{0,9}},173},
    {{{0,8}},6}, {{{0,8}},134}, {{{0,8}},70}, {{{0,9}},237},
    {{{80,7}},9}, {{{0,8}},94}, {{{0,8}},30}, {{{0,9}},157},
    {{{84,7}},99}, {{{0,8}},126}, {{{0,8}},62}, {{{0,9}},221},
    {{{82,7}},27}, {{{0,8}},110}, {{{0,8}},46}, {{{0,9}},189},
    {{{0,8}},14}, {{{0,8}},142}, {{{0,8}},78}, {{{0,9}},253},
    {{{96,7}},256}, {{{0,8}},81}, {{{0,8}},17}, {{{85,8}},131},
    {{{82,7}},31}, {{{0,8}},113}, {{{0,8}},49}, {{{0,9}},195},
    {{{80,7}},10}, {{{0,8}},97}, {{{0,8}},33}, {{{0,9}},163},
    {{{0,8}},1}, {{{0,8}},129}, {{{0,8}},65}, {{{0,9}},227},
    {{{80,7}},6}, {{{0,8}},89}, {{{0,8}},25}, {{{0,9}},147},
    {{{83,7}},59}, {{{0,8}},121}, {{{0,8}},57}, {{{0,9}},211},
    {{{81,7}},17}, {{{0,8}},105}, {{{0,8}},41}, {{{0,9}},179},
    {{{0,8}},9}, {{{0,8}},137}, {{{0,8}},73}, {{{0,9}},243},
    {{{80,7}},4}, {{{0,8}},85}, {{{0,8}},21}, {{{80,8}},258},
    {{{83,7}},43}, {{{0,8}},117}, {{{0,8}},53}, {{{0,9}},203},
    {{{81,7}},13}, {{{0,8}},101}, {{{0,8}},37}, {{{0,9}},171},
    {{{0,8}},5}, {{{0,8}},133}, {{{0,8}},69}, {{{0,9}},235},
    {{{80,7}},8}, {{{0,8}},93}, {{{0,8}},29}, {{{0,9}},155},
    {{{84,7}},83}, {{{0,8}},125}, {{{0,8}},61}, {{{0,9}},219},
    {{{82,7}},23}, {{{0,8}},109}, {{{0,8}},45}, {{{0,9}},187},
    {{{0,8}},13}, {{{0,8}},141}, {{{0,8}},77}, {{{0,9}},251},
    {{{80,7}},3}, {{{0,8}},83}, {{{0,8}},19}, {{{85,8}},195},
    {{{83,7}},35}, {{{0,8}},115}, {{{0,8}},51}, {{{0,9}},199},
    {{{81,7}},11}, {{{0,8}},99}, {{{0,8}},35}, {{{0,9}},167},
    {{{0,8}},3}, {{{0,8}},131}, {{{0,8}},67}, {{{0,9}},231},
    {{{80,7}},7}, {{{0,8}},91}, {{{0,8}},27}, {{{0,9}},151},
    {{{84,7}},67}, {{{0,8}},123}, {{{0,8}},59}, {{{0,9}},215},
    {{{82,7}},19}, {{{0,8}},107}, {{{0,8}},43}, {{{0,9}},183},
    {{{0,8}},11}, {{{0,8}},139}, {{{0,8}},75}, {{{0,9}},247},
    {{{80,7}},5}, {{{0,8}},87}, {{{0,8}},23}, {{{192,8}},0},
    {{{83,7}},51}, {{{0,8}},119}, {{{0,8}},55}, {{{0,9}},207},
    {{{81,7}},15}, {{{0,8}},103}, {{{0,8}},39}, {{{0,9}},175},
    {{{0,8}},7}, {{{0,8}},135}, {{{0,8}},71}, {{{0,9}},239},
    {{{80,7}},9}, {{{0,8}},95}, {{{0,8}},31}, {{{0,9}},159},
    {{{84,7}},99}, {{{0,8}},127}, {{{0,8}},63}, {{{0,9}},223},
    {{{82,7}},27}, {{{0,8}},111}, {{{0,8}},47}, {{{0,9}},191},
    {{{0,8}},15}, {{{0,8}},143}, {{{0,8}},79}, {{{0,9}},255}
  };
const zinflate_huft zfixed_td[] = {
    {{{80,5}},1}, {{{87,5}},257}, {{{83,5}},17}, {{{91,5}},4097},
    {{{81,5}},5}, {{{89,5}},1025}, {{{85,5}},65}, {{{93,5}},16385},
    {{{80,5}},3}, {{{88,5}},513}, {{{84,5}},33}, {{{92,5}},8193},
    {{{82,5}},9}, {{{90,5}},2049}, {{{86,5}},129}, {{{192,5}},24577},
    {{{80,5}},2}, {{{87,5}},385}, {{{83,5}},25}, {{{91,5}},6145},
    {{{81,5}},7}, {{{89,5}},1537}, {{{85,5}},97}, {{{93,5}},24577},
    {{{80,5}},4}, {{{88,5}},769}, {{{84,5}},49}, {{{92,5}},12289},
    {{{82,5}},13}, {{{90,5}},3073}, {{{86,5}},193}, {{{192,5}},24577}
  };

// copy as much as possible from the sliding window to the output area
int zinflate_flush(zinflate_blocks_statef *s,zz_stream_ptr z,int r)
{
  uInt n;
  uChar *p;
  uChar *q;

  // local copies of source and destination pointers
  p = z->next_out;
  q = s->read;

  // compute number of bytes to copy as far as end of window
  n = (uInt)((q <= s->write ? s->write : s->end) - q);
  if (n > z->avail_out) n = z->avail_out;
  if (n && r == ZZ_BUF_ERROR) r = ZZ_OK;

  // update counters
  z->avail_out -= n;
  z->total_out += n;

  // update check information
  if (s->checkfn != ZZ_NULL)
    z->adler = s->check = (*s->checkfn)(s->check, q, n);

  // copy as far as end of window
  if (n!=0)          // check for n!=0 to avoid waking up CodeGuard
  { memcpy(p, q, n);
    p += n;
    q += n;
  }

  // see if more to copy at beginning of window
  if (q == s->end)
  {
    // wrap pointers
    q = s->window;
    if (s->write == s->end)
      s->write = s->window;

    // compute bytes to copy
    n = (uInt)(s->write - q);
    if (n > z->avail_out) n = z->avail_out;
    if (n && r == ZZ_BUF_ERROR) r = ZZ_OK;

    // update counters
    z->avail_out -= n;
    z->total_out += n;

    // update check information
    if (s->checkfn != ZZ_NULL)
      z->adler = s->check = (*s->checkfn)(s->check, q, n);

    // copy
    if (n!=0) {memcpy(p,q,n); p+=n; q+=n;}
  }

  // update pointers
  z->next_out = p;
  s->read = q;

  // done
  return r;
}

// simplify the use of the inflate_huft type with some defines
#define zexop word.what.Exop
#define zbits word.what.Bits

typedef enum {        // waiting for "i:"=input, "o:"=output, "x:"=nothing
      ZSTART,    // x: set up for LEN
      ZLEN,      // i: get length/literal/eob next
      ZLENEXT,   // i: getting length extra (have base)
      ZDIST,     // i: get distance next
      ZDISTEXT,  // i: getting distance extra
      ZCOPY,     // o: copying bytes in window, waiting for space
      ZLIT,      // o: got literal, waiting for output space
      ZWASH,     // o: got eob, possibly still output waiting
      ZEND,      // x: got eob and all data flushed
      ZBADCODE}  // x: got error
zinflate_codes_mode;

// inflate codes private state
struct zinflate_codes_state {

  // mode
  zinflate_codes_mode mode;      // current inflate_codes mode

  // mode dependent information
  uInt len;
  union {
    struct {
      const zinflate_huft *tree;       // pointer into tree
      uInt need;                // bits needed
    } code;             // if LEN or DIST, where in tree
    uInt lit;           // if LIT, literal
    struct {
      uInt get;                 // bits to get for extra
      uInt dist;                // distance back to copy from
    } copy;             // if EXT or COPY, where and how much
  } sub;                // submode

  // mode independent information
  uChar lbits;           // ltree bits decoded per branch
  uChar dbits;           // dtree bits decoder per branch
  const zinflate_huft *ltree;          // literal/length/eob tree
  const zinflate_huft *dtree;          // distance tree

};


zinflate_codes_statef *zinflate_codes_new(
uInt bl, uInt bd,
const zinflate_huft *tl,
const zinflate_huft *td, // need separate declaration for Borland C++
zz_stream_ptr z)
{
  zinflate_codes_statef *c;

  if ((c = (zinflate_codes_statef *)
       ZZALLOC(z,1,sizeof(struct zinflate_codes_state))) != ZZ_NULL)
  {
    c->mode = ZSTART;
    c->lbits = (uChar)bl;
    c->dbits = (uChar)bd;
    c->ltree = tl;
    c->dtree = td;
    LuTracev((stderr, "inflate:       codes new\n"));
  }
  return c;
}


int zinflate_codes(zinflate_blocks_statef *s, zz_stream_ptr z, int r)
{
  uInt j;               // temporary storage
  const zinflate_huft *t;      // temporary pointer
  uInt e;               // extra bits or operation
  uLong b;              // bit buffer
  uInt k;               // bits in bit buffer
  uChar *p;             // input data pointer
  uInt n;               // bytes available there
  uChar *q;             // output window write pointer
  uInt m;               // bytes to end of window or read pointer
  uChar *f;             // pointer to copy strings from
  zinflate_codes_statef *c = s->sub.decode.codes;  // codes state

  // copy input/output information to locals (UPDATE macro restores)
  ZLOAD

  // process input and output based on current state
  for(;;) switch (c->mode)
  {             // waiting for "i:"=input, "o:"=output, "x:"=nothing
    case ZSTART:         // x: set up for LEN
#ifndef SLOW
      if (m >= 258 && n >= 10)
      {
        ZUPDATE
        r = zinflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
        ZLOAD
        if (r != ZZ_OK)
        {
          c->mode = r == ZZ_STREAM_END ? ZWASH : ZBADCODE;
          break;
        }
      }
#endif // !SLOW
      c->sub.code.need = c->lbits;
      c->sub.code.tree = c->ltree;
      c->mode = ZLEN;
    case ZLEN:           // i: get length/literal/eob next
      j = c->sub.code.need;
      ZNEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & zinflate_mask[j]);
      ZDUMPBITS(t->zbits)
      e = (uInt)(t->zexop);
      if (e == 0)               // literal
      {
        c->sub.lit = t->base;
        LuTracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                 "inflate:         literal '%c'\n" :
                 "inflate:         literal 0x%02x\n", t->base));
        c->mode = ZLIT;
        break;
      }
      if (e & 16)               // length
      {
        c->sub.copy.get = e & 15;
        c->len = t->base;
        c->mode = ZLENEXT;
        break;
      }
      if ((e & 64) == 0)        // next table
      {
        c->sub.code.need = e;
        c->sub.code.tree = t + t->base;
        break;
      }
      if (e & 32)               // end of block
      {
        LuTracevv((stderr, "inflate:         end of block\n"));
        c->mode = ZWASH;
        break;
      }
      c->mode = ZBADCODE;        // invalid code
      z->msg = (char*)"invalid literal/length code";
      r = ZZ_DATA_ERROR;
      ZLEAVE
    case ZLENEXT:        // i: getting length extra (have base)
      j = c->sub.copy.get;
      ZNEEDBITS(j)
      c->len += (uInt)b & zinflate_mask[j];
      ZDUMPBITS(j)
      c->sub.code.need = c->dbits;
      c->sub.code.tree = c->dtree;
      LuTracevv((stderr, "inflate:         length %u\n", c->len));
      c->mode = ZDIST;
    case ZDIST:          // i: get distance next
      j = c->sub.code.need;
      ZNEEDBITS(j)
      t = c->sub.code.tree + ((uInt)b & zinflate_mask[j]);
      ZDUMPBITS(t->zbits)
      e = (uInt)(t->zexop);
      if (e & 16)               // distance
      {
        c->sub.copy.get = e & 15;
        c->sub.copy.dist = t->base;
        c->mode = ZDISTEXT;
        break;
      }
      if ((e & 64) == 0)        // next table
      {
        c->sub.code.need = e;
        c->sub.code.tree = t + t->base;
        break;
      }
      c->mode = ZBADCODE;        // invalid code
      z->msg = (char*)"invalid distance code";
      r = ZZ_DATA_ERROR;
      ZLEAVE
    case ZDISTEXT:       // i: getting distance extra
      j = c->sub.copy.get;
      ZNEEDBITS(j)
      c->sub.copy.dist += (uInt)b & zinflate_mask[j];
      ZDUMPBITS(j)
      LuTracevv((stderr, "inflate:         distance %u\n", c->sub.copy.dist));
      c->mode = ZCOPY;
    case ZCOPY:          // o: copying bytes in window, waiting for space
      f = q - c->sub.copy.dist;
      while (f < s->window)             // modulo window size-"while" instead
        f += s->end - s->window;        // of "if" handles invalid distances
      while (c->len)
      {
        ZNEEDOUT
        ZOUTBYTE(*f++)
        if (f == s->end)
          f = s->window;
        c->len--;
      }
      c->mode = ZSTART;
      break;
    case ZLIT:           // o: got literal, waiting for output space
      ZNEEDOUT
      ZOUTBYTE(c->sub.lit)
      c->mode = ZSTART;
      break;
    case ZWASH:          // o: got eob, possibly more output
      if (k > 7)        // return unused byte, if any
      {
        //Assert(k < 16, "inflate_codes grabbed too many bytes")
        k -= 8;
        n++;
        p--;            // can always return one
      }
      ZFLUSH
      if (s->read != s->write)
        ZLEAVE
      c->mode = ZEND;
    case ZEND:
      r = ZZ_STREAM_END;
      ZLEAVE
    case ZBADCODE:       // x: got error
      r = ZZ_DATA_ERROR;
      ZLEAVE
    default:
      r = ZZ_STREAM_ERROR;
      ZLEAVE
  }
}


void zinflate_codes_free(zinflate_codes_statef *c,zz_stream_ptr z)
{ ZZFREE(z, c);
  LuTracev((stderr, "inflate:       codes free\n"));
}



// infblock.c -- interpret and process block types to last block
// Copyright (C) 1995-1998 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h

//struct inflate_codes_state {int dummy;}; // for buggy compilers



// Table for deflate from PKZIP's appnote.txt.
const uInt zborder[] = { // Order of the bit length code lengths
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

//
// Notes beyond the 1.93a appnote.txt:
//
// 1. Distance pointers never point before the beginning of the output stream.
// 2. Distance pointers can point back across blocks, up to 32k away.
// 3. There is an implied maximum of 7 bits for the bit length table and
//    15 bits for the actual data.
// 4. If only one code exists, then it is encoded using one bit.  (Zero
//    would be more efficient, but perhaps a little confusing.)  If two
//    codes exist, they are coded using one bit each (0 and 1).
// 5. There is no way of sending zero distance codes--a dummy must be
//    sent if there are none.  (History: a pre 2.0 version of PKZIP would
//    store blocks with no distance codes, but this was discovered to be
//    too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
//    zero distance codes, which is sent as one code of zero bits in
//    length.
// 6. There are up to 286 literal/length codes.  Code 256 represents the
//    end-of-block.  Note however that the static length tree defines
//    288 codes just to fill out the Huffman codes.  Codes 286 and 287
//    cannot be used though, since there is no length base or extra bits
//    defined for them.  Similarily, there are up to 30 distance codes.
//    However, static trees define 32 codes (all 5 bits) to fill out the
//    Huffman codes, but the last two had better not show up in the data.
// 7. Unzip can check dynamic Huffman blocks for complete code sets.
//    The exception is that a single code would not be complete (see #4).
// 8. The five bits following the block type is really the number of
//    literal codes sent minus 257.
// 9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
//    (1+6+6).  Therefore, to output three times the length, you output
//    three codes (1+1+1), whereas to output four times the same length,
//    you only need two codes (1+3).  Hmm.
//10. In the tree reconstruction algorithm, Code = Code + Increment
//    only if BitLength(i) is not zero.  (Pretty obvious.)
//11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
//12. Note: length code 284 can represent 227-258, but length code 285
//    really is 258.  The last length deserves its own, short code
//    since it gets used a lot in very redundant files.  The length
//    258 is special since 258 - 3 (the min match length) is 255.
//13. The literal/length and distance code bit lengths are read as a
//    single stream of lengths.  It is possible (and advantageous) for
//    a repeat code (16, 17, or 18) to go across the boundary between
//    the two sets of lengths.


void zinflate_blocks_reset(zinflate_blocks_statef *s, zz_stream_ptr z, uLong *c)
{
  if (c != ZZ_NULL)
    *c = s->check;
  if (s->mode == ZIBM_BTREE || s->mode == ZIBM_DTREE)
    ZZFREE(z, s->sub.trees.blens);
  if (s->mode == ZIBM_CODES)
    zinflate_codes_free(s->sub.decode.codes, z);
  s->mode = ZIBM_TYPE;
  s->bitk = 0;
  s->bitb = 0;
  s->read = s->write = s->window;
  if (s->checkfn != ZZ_NULL)
    z->adler = s->check = (*s->checkfn)(0L, (const uChar *)ZZ_NULL, 0);
  LuTracev((stderr, "inflate:   blocks reset\n"));
}


zinflate_blocks_statef *zinflate_blocks_new(zz_stream_ptr z, zcheck_func c, uInt w)
{
  zinflate_blocks_statef *s;

  if ((s = (zinflate_blocks_statef *)ZZALLOC
       (z,1,sizeof(struct zinflate_blocks_state))) == ZZ_NULL)
    return s;
  if ((s->hufts =
       (zinflate_huft *)ZZALLOC(z, sizeof(zinflate_huft), ZMANY)) == ZZ_NULL)
  {
    ZZFREE(z, s);
    return ZZ_NULL;
  }
  if ((s->window = (uChar *)ZZALLOC(z, 1, w)) == ZZ_NULL)
  {
    ZZFREE(z, s->hufts);
    ZZFREE(z, s);
    return ZZ_NULL;
  }
  s->end = s->window + w;
  s->checkfn = c;
  s->mode = ZIBM_TYPE;
  LuTracev((stderr, "inflate:   blocks allocated\n"));
  zinflate_blocks_reset(s, z, ZZ_NULL);
  return s;
}


int zinflate_blocks(zinflate_blocks_statef *s, zz_stream_ptr z, int r)
{
  uInt t;               // temporary storage
  uLong b;              // bit buffer
  uInt k;               // bits in bit buffer
  uChar *p;             // input data pointer
  uInt n;               // bytes available there
  uChar *q;             // output window write pointer
  uInt m;               // bytes to end of window or read pointer

  // copy input/output information to locals (UPDATE macro restores)
  ZLOAD

  // process input based on current state
  for(;;) switch (s->mode)
  {
    case ZIBM_TYPE:
      ZNEEDBITS(3)
      t = (uInt)b & 7;
      s->last = t & 1;
      switch (t >> 1)
      {
        case 0:                         // stored
          LuTracev((stderr, "inflate:     stored block%s\n",
                 s->last ? " (last)" : ""));
          ZDUMPBITS(3)
          t = k & 7;                    // go to byte boundary
          ZDUMPBITS(t)
          s->mode = ZIBM_LENS;               // get length of stored block
          break;
        case 1:                         // fixed
          LuTracev((stderr, "inflate:     fixed codes block%s\n",
                 s->last ? " (last)" : ""));
          {
            uInt bl, bd;
            const zinflate_huft *tl, *td;

            zinflate_trees_fixed(&bl, &bd, &tl, &td, z);
            s->sub.decode.codes = zinflate_codes_new(bl, bd, tl, td, z);
            if (s->sub.decode.codes == ZZ_NULL)
            {
              r = ZZ_MEM_ERROR;
              ZLEAVE
            }
          }
          ZDUMPBITS(3)
          s->mode = ZIBM_CODES;
          break;
        case 2:                         // dynamic
          LuTracev((stderr, "inflate:     dynamic codes block%s\n",
                 s->last ? " (last)" : ""));
          ZDUMPBITS(3)
          s->mode = ZIBM_TABLE;
          break;
        case 3:                         // illegal
          ZDUMPBITS(3)
          s->mode = ZIBM_BAD;
          z->msg = (char*)"invalid block type";
          r = ZZ_DATA_ERROR;
          ZLEAVE
      }
      break;
    case ZIBM_LENS:
      ZNEEDBITS(32)
      if ((((~b) >> 16) & 0xffff) != (b & 0xffff))
      {
        s->mode = ZIBM_BAD;
        z->msg = (char*)"invalid stored block lengths";
        r = ZZ_DATA_ERROR;
        ZLEAVE
      }
      s->sub.left = (uInt)b & 0xffff;
      b = k = 0;                      // dump bits
      LuTracev((stderr, "inflate:       stored length %u\n", s->sub.left));
      s->mode = s->sub.left ? ZIBM_STORED : (s->last ? ZIBM_DRY : ZIBM_TYPE);
      break;
    case ZIBM_STORED:
      if (n == 0)
        ZLEAVE
      ZNEEDOUT
      t = s->sub.left;
      if (t > n) t = n;
      if (t > m) t = m;
      memcpy(q, p, t);
      p += t;  n -= t;
      q += t;  m -= t;
      if ((s->sub.left -= t) != 0)
        break;
      LuTracev((stderr, "inflate:       stored end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      s->mode = s->last ? ZIBM_DRY : ZIBM_TYPE;
      break;
    case ZIBM_TABLE:
      ZNEEDBITS(14)
      s->sub.trees.table = t = (uInt)b & 0x3fff;
      // remove this section to workaround bug in pkzip
      if ((t & 0x1f) > 29 || ((t >> 5) & 0x1f) > 29)
      {
        s->mode = ZIBM_BAD;
        z->msg = (char*)"too many length or distance symbols";
        r = ZZ_DATA_ERROR;
        ZLEAVE
      }
      // end remove
      t = 258 + (t & 0x1f) + ((t >> 5) & 0x1f);
      if ((s->sub.trees.blens = (uInt*)ZZALLOC(z, t, sizeof(uInt))) == ZZ_NULL)
      {
        r = ZZ_MEM_ERROR;
        ZLEAVE
      }
      ZDUMPBITS(14)
      s->sub.trees.index = 0;
      LuTracev((stderr, "inflate:       table sizes ok\n"));
      s->mode = ZIBM_BTREE;
    case ZIBM_BTREE:
      while (s->sub.trees.index < 4 + (s->sub.trees.table >> 10))
      {
        ZNEEDBITS(3)
        s->sub.trees.blens[zborder[s->sub.trees.index++]] = (uInt)b & 7;
        ZDUMPBITS(3)
      }
      while (s->sub.trees.index < 19)
        s->sub.trees.blens[zborder[s->sub.trees.index++]] = 0;
      s->sub.trees.bb = 7;
      t = zinflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb,
                             &s->sub.trees.tb, s->hufts, z);
      if (t != ZZ_OK)
      {
        r = t;
        if (r == ZZ_DATA_ERROR)
        {
          ZZFREE(z, s->sub.trees.blens);
          s->mode = ZIBM_BAD;
        }
        ZLEAVE
      }
      s->sub.trees.index = 0;
      LuTracev((stderr, "inflate:       bits tree ok\n"));
      s->mode = ZIBM_DTREE;
    case ZIBM_DTREE:
      while (t = s->sub.trees.table,
             s->sub.trees.index < 258 + (t & 0x1f) + ((t >> 5) & 0x1f))
      {
        zinflate_huft *h;
        uInt i, j, c;

        t = s->sub.trees.bb;
        ZNEEDBITS(t)
        h = s->sub.trees.tb + ((uInt)b & zinflate_mask[t]);
        t = h->zbits;
        c = h->base;
        if (c < 16)
        {
          ZDUMPBITS(t)
          s->sub.trees.blens[s->sub.trees.index++] = c;
        }
        else // c == 16..18
        {
          i = c == 18 ? 7 : c - 14;
          j = c == 18 ? 11 : 3;
          ZNEEDBITS(t + i)
          ZDUMPBITS(t)
          j += (uInt)b & zinflate_mask[i];
          ZDUMPBITS(i)
          i = s->sub.trees.index;
          t = s->sub.trees.table;
          if (i + j > 258 + (t & 0x1f) + ((t >> 5) & 0x1f) ||
              (c == 16 && i < 1))
          {
            ZZFREE(z, s->sub.trees.blens);
            s->mode = ZIBM_BAD;
            z->msg = (char*)"invalid bit length repeat";
            r = ZZ_DATA_ERROR;
            ZLEAVE
          }
          c = c == 16 ? s->sub.trees.blens[i - 1] : 0;
          do {
            s->sub.trees.blens[i++] = c;
          } while (--j);
          s->sub.trees.index = i;
        }
      }
      s->sub.trees.tb = ZZ_NULL;
      {
        uInt bl, bd;
        zinflate_huft *tl, *td;
        zinflate_codes_statef *c;

        bl = 9;         // must be <= 9 for lookahead assumptions
        bd = 6;         // must be <= 9 for lookahead assumptions
        t = s->sub.trees.table;
        t = zinflate_trees_dynamic(257 + (t & 0x1f), 1 + ((t >> 5) & 0x1f),
                                  s->sub.trees.blens, &bl, &bd, &tl, &td,
                                  s->hufts, z);
        if (t != ZZ_OK)
        {
          if (t == (uInt)ZZ_DATA_ERROR)
          {
            ZZFREE(z, s->sub.trees.blens);
            s->mode = ZIBM_BAD;
          }
          r = t;
          ZLEAVE
        }
        LuTracev((stderr, "inflate:       trees ok\n"));
        if ((c = zinflate_codes_new(bl, bd, tl, td, z)) == ZZ_NULL)
        {
          r = ZZ_MEM_ERROR;
          ZLEAVE
        }
        s->sub.decode.codes = c;
      }
      ZZFREE(z, s->sub.trees.blens);
      s->mode = ZIBM_CODES;
    case ZIBM_CODES:
      ZUPDATE
      if ((r = zinflate_codes(s, z, r)) != ZZ_STREAM_END)
        return zinflate_flush(s, z, r);
      r = ZZ_OK;
      zinflate_codes_free(s->sub.decode.codes, z);
      ZLOAD
      LuTracev((stderr, "inflate:       codes end, %lu total out\n",
              z->total_out + (q >= s->read ? q - s->read :
              (s->end - s->read) + (q - s->window))));
      if (!s->last)
      {
        s->mode = ZIBM_TYPE;
        break;
      }
      s->mode = ZIBM_DRY;
    case ZIBM_DRY:
      ZFLUSH
      if (s->read != s->write)
        ZLEAVE
      s->mode = ZIBM_DONE;
    case ZIBM_DONE:
      r = ZZ_STREAM_END;
      ZLEAVE
    case ZIBM_BAD:
      r = ZZ_DATA_ERROR;
      ZLEAVE
    default:
      r = ZZ_STREAM_ERROR;
      ZLEAVE
  }
}


int zinflate_blocks_free(zinflate_blocks_statef *s, zz_stream_ptr z)
{
  zinflate_blocks_reset(s, z, ZZ_NULL);
  ZZFREE(z, s->window);
  ZZFREE(z, s->hufts);
  ZZFREE(z, s);
  LuTracev((stderr, "inflate:   blocks freed\n"));
  return ZZ_OK;
}



// inftrees.c -- generate Huffman trees for efficient decoding
// Copyright (C) 1995-1998 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h
//



extern const char zinflate_copyright[] =
   " inflate 1.X.X Copyright 1995-2018 Mark Adler ";
// If you use the zlib library in a product, an acknowledgment is welcome
// in the documentation of your product. If for some reason you cannot
// include such an acknowledgment, I would appreciate that you keep this
// copyright string in the executable of your product.



int zhuft_build (
    uInt *,            // code lengths in bits
    uInt,               // number of codes
    uInt,               // number of "simple" codes
    const uInt *,      // list of base values for non-simple codes
    const uInt *,      // list of extra bits for non-simple codes
    zinflate_huft **,// result: starting table
    uInt *,            // maximum lookup bits (returns actual)
    zinflate_huft *,     // space for trees
    uInt *,             // hufts used in space
    uInt * );         // space for values

// Tables for deflate from PKZIP's appnote.txt.
const uInt zcplens[31] = { // Copy lengths for literal codes 257..285
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
        // see note #13 above about 258
const uInt zcplext[31] = { // Extra bits for literal codes 257..285
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 112, 112}; // 112==invalid
const uInt zcpdist[30] = { // Copy offsets for distance codes 0..29
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
const uInt zcpdext[30] = { // Extra bits for distance codes
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

//
//   Huffman code decoding is performed using a multi-level table lookup.
//   The fastest way to decode is to simply build a lookup table whose
//   size is determined by the longest code.  However, the time it takes
//   to build this table can also be a factor if the data being decoded
//   is not very long.  The most common codes are necessarily the
//   shortest codes, so those codes dominate the decoding time, and hence
//   the speed.  The idea is you can have a shorter table that decodes the
//   shorter, more probable codes, and then point to subsidiary tables for
//   the longer codes.  The time it costs to decode the longer codes is
//   then traded against the time it takes to make longer tables.
//
//   This results of this trade are in the variables lbits and dbits
//   below.  lbits is the number of bits the first level table for literal/
//   length codes can decode in one step, and dbits is the same thing for
//   the distance codes.  Subsequent tables are also less than or equal to
//   those sizes.  These values may be adjusted either when all of the
//   codes are shorter than that, in which case the longest code length in
//   bits is used, or when the shortest code is *longer* than the requested
//   table size, in which case the length of the shortest code in bits is
//   used.
//
//   There are two different values for the two tables, since they code a
//   different number of possibilities each.  The literal/length table
//   codes 286 possible values, or in a flat code, a little over eight
//   bits.  The distance table codes 30 possible values, or a little less
//   than five bits, flat.  The optimum values for speed end up being
//   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
//   The optimum values may differ though from machine to machine, and
//   possibly even between compilers.  Your mileage may vary.
//


// If BMAX needs to be larger than 16, then h and x[] should be uLong.
#define ZBMAX 15         // maximum bit length of any code

int zhuft_build(
uInt *b,               // code lengths in bits (all assumed <= BMAX)
uInt n,                 // number of codes (assumed <= 288)
uInt s,                 // number of simple-valued codes (0..s-1)
const uInt *d,         // list of base values for non-simple codes
const uInt *e,         // list of extra bits for non-simple codes
zinflate_huft * *t,  // result: starting table
uInt *m,               // maximum lookup bits, returns actual
zinflate_huft *hp,       // space for trees
uInt *hn,               // hufts used in space
uInt *v)               // working area: values in order of bit length
// Given a list of code lengths and a maximum table size, make a set of
// tables to decode that set of codes.  Return Z_OK on success, Z_BUF_ERROR
// if the given code set is incomplete (the tables are still built in this
// case), or Z_DATA_ERROR if the input is invalid.
{

  uInt a;                       // counter for codes of length k
  uInt c[ZBMAX+1];               // bit length count table
  uInt f;                       // i repeats in table every f entries
  int g;                        // maximum code length
  int h;                        // table level
  register uInt i;              // counter, current code
  register uInt j;              // counter
  register int k;               // number of bits in current code
  int l;                        // bits per table (returned in m)
  uInt mask;                    // (1 << w) - 1, to avoid cc -O bug on HP
  register uInt *p;            // pointer into c[], b[], or v[]
  zinflate_huft *q;              // points to current table
  struct zinflate_huft_s r;      // table entry for structure assignment
  zinflate_huft *u[ZBMAX];        // table stack
  register int w;               // bits before this table == (l * h)
  uInt x[ZBMAX+1];               // bit offsets, then code stack
  uInt *xp;                    // pointer into x
  int y;                        // number of dummy codes added
  uInt z;                       // number of entries in current table


  // Generate counts for each bit length
  p = c;
#define C0 *p++ = 0;
#define C2 C0 C0 C0 C0
#define C4 C2 C2 C2 C2
  C4; p;                          // clear c[]--assume BMAX+1 is 16
  p = b;  i = n;
  do {
    c[*p++]++;                  // assume all entries <= BMAX
  } while (--i);
  if (c[0] == n)                // null input--all zero length codes
  {
    *t = (zinflate_huft *)ZZ_NULL;
    *m = 0;
    return ZZ_OK;
  }


  // Find minimum and maximum length, bound *m by those
  l = *m;
  for (j = 1; j <= ZBMAX; j++)
    if (c[j])
      break;
  k = j;                        // minimum code length
  if ((uInt)l < j)
    l = j;
  for (i = ZBMAX; i; i--)
    if (c[i])
      break;
  g = i;                        // maximum code length
  if ((uInt)l > i)
    l = i;
  *m = l;


  // Adjust last length count to fill out codes, if needed
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return ZZ_DATA_ERROR;
  if ((y -= c[i]) < 0)
    return ZZ_DATA_ERROR;
  c[i] += y;


  // Generate starting offsets into the value table for each length
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 // note that i == g from above
    *xp++ = (j += *p++);
  }


  // Make a table of values in order of bit lengths
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);
  n = x[g];                     // set n to length of v


  // Generate the Huffman codes and for each, make the table entries
  x[0] = i = 0;                 // first Huffman code is zero
  p = v;                        // grab values in bit order
  h = -1;                       // no tables yet--level -1
  w = -l;                       // bits decoded == (l * h)
  u[0] = (zinflate_huft *)ZZ_NULL;        // just to keep compilers happy
  q = (zinflate_huft *)ZZ_NULL;   // ditto
  z = 0;                        // ditto

  // go through the bit lengths (k already is bits in shortest code)
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      // here i is the Huffman code of length k bits for value *p
      // make tables up to required level
      while (k > w + l)
      {
        h++;
        w += l;                 // previous table always l bits

        // compute minimum size table less than or equal to l bits
        z = g - w;
        z = z > (uInt)l ? l : z;        // table size upper limit
        if ((f = 1 << (j = k - w)) > a + 1)     // try a k-w bit table
        {                       // too few codes for k-w bit table
          f -= a + 1;           // deduct codes from patterns left
          xp = c + k;
          if (j < z)
            while (++j < z)     // try smaller tables up to z bits
            {
              if ((f <<= 1) <= *++xp)
                break;          // enough codes to use up j bits
              f -= *xp;         // else deduct codes from patterns
            }
        }
        z = 1 << j;             // table entries for j-bit table

        // allocate new table
        if (*hn + z > ZMANY)     // (note: doesn't matter for fixed)
          return ZZ_DATA_ERROR;  // overflow of MANY
        u[h] = q = hp + *hn;
        *hn += z;

        // connect to last table, if there is one
        if (h)
        {
          x[h] = i;             // save pattern for backing up
          r.zbits = (uChar)l;     // bits to dump before this table
          r.zexop = (uChar)j;     // bits in this table
          j = i >> (w - l);
          r.base = (uInt)(q - u[h-1] - j);   // offset to this table
          u[h-1][j] = r;        // connect to last table
        }
        else
          *t = q;               // first table is returned result
      }

      // set up table entry in r
      r.zbits = (uChar)(k - w);
      if (p >= v + n)
        r.zexop = 128 + 64;      // out of values--invalid code
      else if (*p < s)
      {
        r.zexop = (uChar)(*p < 256 ? 0 : 32 + 64);     // 256 is end-of-block
        r.base = *p++;          // simple code is just the value
      }
      else
      {
        r.zexop = (uChar)(e[*p - s] + 16 + 64);// non-simple--look up in lists
        r.base = d[*p++ - s];
      }

      // fill code-like entries with r
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      // backwards increment the k-bit code i
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      // backup over finished tables
      mask = (1 << w) - 1;      // needed on HP, cc -O bug
      while ((i & mask) != x[h])
      {
        h--;                    // don't need to update q
        w -= l;
        mask = (1 << w) - 1;
      }
    }
  }


  // Return Z_BUF_ERROR if we were given an incomplete table
  return y != 0 && g != 1 ? ZZ_BUF_ERROR : ZZ_OK;
}


int zinflate_trees_bits(
uInt *c,               // 19 code lengths
uInt *bb,              // bits tree desired/actual depth
zinflate_huft * *tb, // bits tree result
zinflate_huft *hp,       // space for trees
zz_stream_ptr z)            // for messages
{
  int r;
  uInt hn = 0;          // hufts used in space
  uInt *v;             // work area for huft_build

  if ((v = (uInt*)ZZALLOC(z, 19, sizeof(uInt))) == ZZ_NULL)
    return ZZ_MEM_ERROR;
  r = zhuft_build(c, 19, 19, (uInt*)ZZ_NULL, (uInt*)ZZ_NULL,
                 tb, bb, hp, &hn, v);
  if (r == ZZ_DATA_ERROR)
    z->msg = (char*)"oversubscribed dynamic bit lengths tree";
  else if (r == ZZ_BUF_ERROR || *bb == 0)
  {
    z->msg = (char*)"incomplete dynamic bit lengths tree";
    r = ZZ_DATA_ERROR;
  }
  ZZFREE(z, v);
  return r;
}


int zinflate_trees_dynamic(
uInt nl,                // number of literal/length codes
uInt nd,                // number of distance codes
uInt *c,               // that many (total) code lengths
uInt *bl,              // literal desired/actual bit depth
uInt *bd,              // distance desired/actual bit depth
zinflate_huft * *tl, // literal/length tree result
zinflate_huft * *td, // distance tree result
zinflate_huft *hp,       // space for trees
zz_stream_ptr z)            // for messages
{
  int r;
  uInt hn = 0;          // hufts used in space
  uInt *v;             // work area for huft_build

  // allocate work area
  if ((v = (uInt*)ZZALLOC(z, 288, sizeof(uInt))) == ZZ_NULL)
    return ZZ_MEM_ERROR;

  // build literal/length tree
  r = zhuft_build(c, nl, 257, zcplens, zcplext, tl, bl, hp, &hn, v);
  if (r != ZZ_OK || *bl == 0)
  {
    if (r == ZZ_DATA_ERROR)
      z->msg = (char*)"oversubscribed literal/length tree";
    else if (r != ZZ_MEM_ERROR)
    {
      z->msg = (char*)"incomplete literal/length tree";
      r = ZZ_DATA_ERROR;
    }
    ZZFREE(z, v);
    return r;
  }

  // build distance tree
  r = zhuft_build(c + nl, nd, 0, zcpdist, zcpdext, td, bd, hp, &hn, v);
  if (r != ZZ_OK || (*bd == 0 && nl > 257))
  {
    if (r == ZZ_DATA_ERROR)
      z->msg = (char*)"oversubscribed distance tree";
    else if (r == ZZ_BUF_ERROR) {
      z->msg = (char*)"incomplete distance tree";
      r = ZZ_DATA_ERROR;
    }
    else if (r != ZZ_MEM_ERROR)
    {
      z->msg = (char*)"empty distance tree with lengths";
      r = ZZ_DATA_ERROR;
    }
    ZZFREE(z, v);
    return r;
  }

  // done
  ZZFREE(z, v);
  return ZZ_OK;
}





int zinflate_trees_fixed(
uInt *bl,               // literal desired/actual bit depth
uInt *bd,               // distance desired/actual bit depth
const zinflate_huft * * tl,     // literal/length tree result
const zinflate_huft * *td,     // distance tree result
zz_stream_ptr )             // for memory allocation
{
  *bl = zfixed_bl;
  *bd = zfixed_bd;
  *tl = zfixed_tl;
  *td = zfixed_td;
  return ZZ_OK;
}


// inffast.c -- process literals and length/distance pairs fast
// Copyright (C) 1995-1998 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h
//


//struct inflate_codes_state {int dummy;}; // for buggy compilers


// macros for bit input with no checking and for returning unused bytes
#define ZGRABBITS(j) {while(k<(j)){b|=((uLong)ZNEXTBYTE)<<k;k+=8;}}
#define ZUNGRAB {c=z->avail_in-n;c=(k>>3)<c?k>>3:c;n+=c;p-=c;k-=c<<3;}

// Called with number of bytes left to write in window at least 258
// (the maximum string length) and number of input bytes available
// at least ten.  The ten bytes are six bytes for the longest length/
// distance pair plus four bytes for overloading the bit buffer.

int zinflate_fast(
uInt bl, uInt bd,
const zinflate_huft *tl,
const zinflate_huft *td, // need separate declaration for Borland C++
zinflate_blocks_statef *s,
zz_stream_ptr z)
{
  const zinflate_huft *t;      // temporary pointer
  uInt e;               // extra bits or operation
  uLong b;              // bit buffer
  uInt k;               // bits in bit buffer
  uChar *p;             // input data pointer
  uInt n;               // bytes available there
  uChar *q;             // output window write pointer
  uInt m;               // bytes to end of window or read pointer
  uInt ml;              // mask for literal/length tree
  uInt md;              // mask for distance tree
  uInt c;               // bytes to copy
  uInt d;               // distance back to copy from
  uChar *r;             // copy source pointer

  // load input, output, bit values
  ZLOAD

  // initialize masks
  ml = zinflate_mask[bl];
  md = zinflate_mask[bd];

  // do until not enough input or output space for fast loop
  do {                          // assume called with m >= 258 && n >= 10
    // get literal/length code
    ZGRABBITS(20)                // max bits for literal/length code
    if ((e = (t = tl + ((uInt)b & ml))->zexop) == 0)
    {
      ZDUMPBITS(t->zbits)
      LuTracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                "inflate:         * literal '%c'\n" :
                "inflate:         * literal 0x%02x\n", t->base));
      *q++ = (uChar)t->base;
      m--;
      continue;
    }
    for (;;) {
      ZDUMPBITS(t->zbits)
      if (e & 16)
      {
        // get extra bits for length
        e &= 15;
        c = t->base + ((uInt)b & zinflate_mask[e]);
        ZDUMPBITS(e)
        LuTracevv((stderr, "inflate:         * length %u\n", c));

        // decode distance base of block to copy
        ZGRABBITS(15);           // max bits for distance code
        e = (t = td + ((uInt)b & md))->zexop;
        for (;;) {
          ZDUMPBITS(t->zbits)
          if (e & 16)
          {
            // get extra bits to add to distance base
            e &= 15;
            ZGRABBITS(e)         // get extra bits (up to 13)
            d = t->base + ((uInt)b & zinflate_mask[e]);
            ZDUMPBITS(e)
            LuTracevv((stderr, "inflate:         * distance %u\n", d));

            // do the copy
            m -= c;
            r = q - d;
            if (r < s->window)                  // wrap if needed
            {
              do {
                r += s->end - s->window;        // force pointer in window
              } while (r < s->window);          // covers invalid distances
              e = (uInt) (s->end - r);
              if (c > e)
              {
                c -= e;                         // wrapped copy
                do {
                    *q++ = *r++;
                } while (--e);
                r = s->window;
                do {
                    *q++ = *r++;
                } while (--c);
              }
              else                              // normal copy
              {
                *q++ = *r++;  c--;
                *q++ = *r++;  c--;
                do {
                    *q++ = *r++;
                } while (--c);
              }
            }
            else                                /* normal copy */
            {
              *q++ = *r++;  c--;
              *q++ = *r++;  c--;
              do {
                *q++ = *r++;
              } while (--c);
            }
            break;
          }
          else if ((e & 64) == 0)
          {
            t += t->base;
            e = (t += ((uInt)b & zinflate_mask[e]))->zexop;
          }
          else
          {
            z->msg = (char*)"invalid distance code";
            ZUNGRAB
            ZUPDATE
            return ZZ_DATA_ERROR;
          }
        };
        break;
      }
      if ((e & 64) == 0)
      {
        t += t->base;
        if ((e = (t += ((uInt)b & zinflate_mask[e]))->zexop) == 0)
        {
          ZDUMPBITS(t->zbits)
          LuTracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                    "inflate:         * literal '%c'\n" :
                    "inflate:         * literal 0x%02x\n", t->base));
          *q++ = (uChar)t->base;
          m--;
          break;
        }
      }
      else if (e & 32)
      {
        LuTracevv((stderr, "inflate:         * end of block\n"));
        ZUNGRAB
        ZUPDATE
        return ZZ_STREAM_END;
      }
      else
      {
        z->msg = (char*)"invalid literal/length code";
        ZUNGRAB
        ZUPDATE
        return ZZ_DATA_ERROR;
      }
    };
  } while (m >= 258 && n >= 10);

  // not enough input or output--restore pointers and return
  ZUNGRAB
  ZUPDATE
  return ZZ_OK;
}

char zdecode(unsigned long *keys, char c)
{ c^=zdecrypt_byte(keys);
  zupdate_keys(keys,c);
  return c;
}



// adler32.c -- compute the Adler-32 checksum of a data stream
// Copyright (C) 1995-1998 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h

// @(#) $Id$


#define ZBASE 65521L // largest prime smaller than 65536
#define ZNMAX 5552
// NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define ZAD_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define ZAD_DO2(buf,i)  ZAD_DO1(buf,i); ZAD_DO1(buf,i+1);
#define ZAD_DO4(buf,i)  ZAD_DO2(buf,i); ZAD_DO2(buf,i+2);
#define ZAD_DO8(buf,i)  ZAD_DO4(buf,i); ZAD_DO4(buf,i+4);
#define ZAD_DO16(buf)   ZAD_DO8(buf,0); ZAD_DO8(buf,8);

// =========================================================================
uLong zadler32(uLong adler, const uChar *buf, uInt len)
{
    unsigned long s1 = adler & 0xffff;
    unsigned long s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == ZZ_NULL) return 1L;

    while (len > 0) {
        k = len < ZNMAX ? len : ZNMAX;
        len -= k;
        while (k >= 16) {
            ZAD_DO16(buf);
	    buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
	    s2 += s1;
        } while (--k);
        s1 %= ZBASE;
        s2 %= ZBASE;
    }
    return (s2 << 16) | s1;
}



// zutil.c -- target dependent utility functions for the compression library
// Copyright (C) 1995-1998 Jean-loup Gailly.
// For conditions of distribution and use, see copyright notice in zlib.h
// @(#) $Id$






const char * zzlibVersion()
{
    return ZZLIB_VERSION;
}

// exported to allow conversion of error code to string for compress() and
// uncompress()
const char * zzError(int err)
{ return ZERR_MSG(err);
}




voidpf zzcalloc (voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) items += size - size; // make compiler happy
    return (voidpf)calloc(items, size);
}

void  zzcfree (voidpf opaque, voidpf ptr)
{
    zzfree(ptr);
    if (opaque) return; // make compiler happy
}



// inflate.c -- zlib interface to inflate modules
// Copyright (C) 1995-1998 Mark Adler
// For conditions of distribution and use, see copyright notice in zlib.h

//struct inflate_blocks_state {int dummy;}; // for buggy compilers

typedef enum {
      ZIM_METHOD,   // waiting for method byte
      ZIM_FLAG,     // waiting for flag byte
      ZIM_DICT4,    // four dictionary check bytes to go
      ZIM_DICT3,    // three dictionary check bytes to go
      ZIM_DICT2,    // two dictionary check bytes to go
      ZIM_DICT1,    // one dictionary check byte to go
      ZIM_DICT0,    // waiting for inflateSetDictionary
      ZIM_BLOCKS,   // decompressing blocks
      ZIM_CHECK4,   // four check bytes to go
      ZIM_CHECK3,   // three check bytes to go
      ZIM_CHECK2,   // two check bytes to go
      ZIM_CHECK1,   // one check byte to go
      ZIM_DONE,     // finished check, done
      ZIM_BAD}      // got an error--stay here
zinflate_mode;

// inflate private state
struct zinternal_state {

  // mode
  zinflate_mode  mode;   // current inflate mode

  // mode dependent information
  union {
    uInt method;        // if IM_FLAGS, method byte
    struct {
      uLong was;                // computed check value
      uLong need;               // stream check value
    } check;            // if CHECK, check values to compare
    uInt marker;        // if IM_BAD, inflateSync's marker bytes count
  } sub;        // submode

  // mode independent information
  int  nowrap;          // flag for no wrapper
  uInt wbits;           // log2(window size)  (8..15, defaults to 15)
  zinflate_blocks_statef
    *blocks;            // current inflate_blocks state

};

int zinflateReset(zz_stream_ptr z)
{
  if (z == ZZ_NULL || z->state == ZZ_NULL)
    return ZZ_STREAM_ERROR;
  z->total_in = z->total_out = 0;
  z->msg = ZZ_NULL;
  z->state->mode = z->state->nowrap ? ZIM_BLOCKS : ZIM_METHOD;
  zinflate_blocks_reset(z->state->blocks, z, ZZ_NULL);
  LuTracev((stderr, "inflate: reset\n"));
  return ZZ_OK;
}

int zinflateEnd(zz_stream_ptr z)
{
  if (z == ZZ_NULL || z->state == ZZ_NULL || z->zfree == ZZ_NULL)
    return ZZ_STREAM_ERROR;
  if (z->state->blocks != ZZ_NULL)
    zinflate_blocks_free(z->state->blocks, z);
  ZZFREE(z, z->state);
  z->state = ZZ_NULL;
  LuTracev((stderr, "inflate: end\n"));
  return ZZ_OK;
}


int zinflateInit2(zz_stream_ptr z)
{ const char *version = ZZLIB_VERSION; int stream_size = sizeof(zz_stream);
  if (version == ZZ_NULL || version[0] != ZZLIB_VERSION[0] || stream_size != sizeof(zz_stream)) return ZZ_VERSION_ERROR;

  int w = -15; // MAX_WBITS: 32K LZ77 window.
  // Warning: reducing MAX_WBITS makes minigzip unable to extract .gz files created by gzip.
  // The memory requirements for deflate are (in bytes):
  //            (1 << (windowBits+2)) +  (1 << (memLevel+9))
  // that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
  // plus a few kilobytes for small objects. For example, if you want to reduce
  // the default memory requirements from 256K to 128K, compile with
  //     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
  // Of course this will generally degrade compression (there's no free lunch).
  //
  //   The memory requirements for inflate are (in bytes) 1 << windowBits
  // that is, 32K for windowBits=15 (default value) plus a few kilobytes
  // for small objects.

  // initialize state
  if (z == ZZ_NULL) return ZZ_STREAM_ERROR;
  z->msg = ZZ_NULL;
  if (z->zalloc == ZZ_NULL)
  {
    z->zalloc = zzcalloc;
    z->opaque = (voidpf)0;
  }
  if (z->zfree == ZZ_NULL) z->zfree = zzcfree;
  if ((z->state = (struct zinternal_state *)
       ZZALLOC(z,1,sizeof(struct zinternal_state))) == ZZ_NULL)
    return ZZ_MEM_ERROR;
  z->state->blocks = ZZ_NULL;

  // handle undocumented nowrap option (no zlib header or check)
  z->state->nowrap = 0;
  if (w < 0)
  {
    w = - w;
    z->state->nowrap = 1;
  }

  // set window size
  if (w < 8 || w > 15)
  {
    zinflateEnd(z);
    return ZZ_STREAM_ERROR;
  }
  z->state->wbits = (uInt)w;

  // create inflate_blocks state
  if ((z->state->blocks =
      zinflate_blocks_new(z, z->state->nowrap ? ZZ_NULL : zadler32, (uInt)1 << w))
      == ZZ_NULL)
  {
    zinflateEnd(z);
    return ZZ_MEM_ERROR;
  }
  LuTracev((stderr, "inflate: allocated\n"));

  // reset state
  zinflateReset(z);
  return ZZ_OK;
}



#define ZIM_NEEDBYTE {if(z->avail_in==0)return r;r=f;}
#define ZIM_NEXTBYTE (z->avail_in--,z->total_in++,*z->next_in++)

int zinflate(zz_stream_ptr z, int f)
{
  int r;
  uInt b;

  if (z == ZZ_NULL || z->state == ZZ_NULL || z->next_in == ZZ_NULL)
    return ZZ_STREAM_ERROR;
  f = f == ZZ_FINISH ? ZZ_BUF_ERROR : ZZ_OK;
  r = ZZ_BUF_ERROR;
  for (;;) switch (z->state->mode)
  {
    case ZIM_METHOD:
      ZIM_NEEDBYTE
      if (((z->state->sub.method = ZIM_NEXTBYTE) & 0xf) != ZZ_DEFLATED)
      {
        z->state->mode = ZIM_BAD;
        z->msg = (char*)"unknown compression method";
        z->state->sub.marker = 5;       // can't try inflateSync
        break;
      }
      if ((z->state->sub.method >> 4) + 8 > z->state->wbits)
      {
        z->state->mode = ZIM_BAD;
        z->msg = (char*)"invalid window size";
        z->state->sub.marker = 5;       // can't try inflateSync
        break;
      }
      z->state->mode = ZIM_FLAG;
    case ZIM_FLAG:
      ZIM_NEEDBYTE
      b = ZIM_NEXTBYTE;
      if (((z->state->sub.method << 8) + b) % 31)
      {
        z->state->mode = ZIM_BAD;
        z->msg = (char*)"incorrect header check";
        z->state->sub.marker = 5;       // can't try inflateSync
        break;
      }
      LuTracev((stderr, "inflate: zlib header ok\n"));
      if (!(b & ZPRESET_DICT))
      {
        z->state->mode = ZIM_BLOCKS;
        break;
      }
      z->state->mode = ZIM_DICT4;
    case ZIM_DICT4:
      ZIM_NEEDBYTE
      z->state->sub.check.need = (uLong)ZIM_NEXTBYTE << 24;
      z->state->mode = ZIM_DICT3;
    case ZIM_DICT3:
      ZIM_NEEDBYTE
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE << 16;
      z->state->mode = ZIM_DICT2;
    case ZIM_DICT2:
      ZIM_NEEDBYTE
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE << 8;
      z->state->mode = ZIM_DICT1;
    case ZIM_DICT1:
      ZIM_NEEDBYTE; r;
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE;
      z->adler = z->state->sub.check.need;
      z->state->mode = ZIM_DICT0;
      return ZZ_NEED_DICT;
    case ZIM_DICT0:
      z->state->mode = ZIM_BAD;
      z->msg = (char*)"need dictionary";
      z->state->sub.marker = 0;       // can try inflateSync
      return ZZ_STREAM_ERROR;
    case ZIM_BLOCKS:
      r = zinflate_blocks(z->state->blocks, z, r);
      if (r == ZZ_DATA_ERROR)
      {
        z->state->mode = ZIM_BAD;
        z->state->sub.marker = 0;       // can try inflateSync
        break;
      }
      if (r == ZZ_OK)
        r = f;
      if (r != ZZ_STREAM_END)
        return r;
      r = f;
      zinflate_blocks_reset(z->state->blocks, z, &z->state->sub.check.was);
      if (z->state->nowrap)
      {
        z->state->mode = ZIM_DONE;
        break;
      }
      z->state->mode = ZIM_CHECK4;
    case ZIM_CHECK4:
      ZIM_NEEDBYTE
      z->state->sub.check.need = (uLong)ZIM_NEXTBYTE << 24;
      z->state->mode = ZIM_CHECK3;
    case ZIM_CHECK3:
      ZIM_NEEDBYTE
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE << 16;
      z->state->mode = ZIM_CHECK2;
    case ZIM_CHECK2:
      ZIM_NEEDBYTE
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE << 8;
      z->state->mode = ZIM_CHECK1;
    case ZIM_CHECK1:
      ZIM_NEEDBYTE
      z->state->sub.check.need += (uLong)ZIM_NEXTBYTE;

      if (z->state->sub.check.was != z->state->sub.check.need)
      {
        z->state->mode = ZIM_BAD;
        z->msg = (char*)"incorrect data check";
        z->state->sub.marker = 5;       // can't try inflateSync
        break;
      }
      LuTracev((stderr, "inflate: zlib check ok\n"));
      z->state->mode = ZIM_DONE;
    case ZIM_DONE:
      return ZZ_STREAM_END;
    case ZIM_BAD:
      return ZZ_DATA_ERROR;
    default:
      return ZZ_STREAM_ERROR;
  }
}





// unzip.c -- IO on .zip files using zlib
// Version 0.15 beta, Mar 19th, 1998,
// Read unzip.h for more info




#define ZUNZ_BUFSIZE (16384)
#define ZUNZ_MAXFILENAMEINZIP (256)
#define ZSIZECENTRALDIRITEM (0x2e)
#define ZSIZEZIPLOCALHEADER (0x1e)


const char zunz_copyright[] = " unzip 0.15 Copyright 1998 Gilles Vollant ";

// unz_file_info_interntal contain internal info about a file in zipfile
typedef struct zunz_file_info_internal_s
{
    uLong offset_curfile;// relative offset of local header 4 bytes
} zunz_file_info_internal;


typedef struct
{ bool is_handle; // either a handle or memory
  bool canseek;
  // for handles:
  ZHANDLE h; bool herr; unsigned long initial_offset; bool mustclosehandle;
  // for memory:
  void *buf; unsigned int len,pos; // if it's a memory block
} ZLUFILE;


ZLUFILE *zlufopen(void *z,unsigned int len,ZDWORD flags,ZZRESULT *err)
{ if (flags!=ZZIP_HANDLE && flags!=ZZIP_FILENAME && flags!=ZZIP_MEMORY) {*err=ZZR_ARGS; return NULL;}
  //
  ZHANDLE h=0; bool canseek=false; *err=ZZR_OK;
  bool mustclosehandle=false;
  if (flags==ZZIP_HANDLE||flags==ZZIP_FILENAME)
  { if (flags==ZZIP_HANDLE)
    { ZHANDLE hf = (ZHANDLE)z;
      h=hf; mustclosehandle=false;
#ifdef DuplicateHandle
      BOOL res = DuplicateHandle(GetCurrentProcess(),hf,GetCurrentProcess(),&h,0,FALSE,DUPLICATE_SAME_ACCESS);
      if (!res) mustclosehandle=true;
#endif
    }
    else
    {
#ifdef ZIP_STD
      h=fopen((const char*)z,"rb");
      if (h==0) {*err=ZZR_NOFILE; return NULL;}
#else
      h=CreateFile((const ZTCHAR*)z,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
      if (h==Z_INVALID_HANDLE_VALUE) {*err=ZZR_NOFILE; return NULL;}
#endif
      mustclosehandle=true;
    }
    // test if we can seek on it. We can't use GetFileType(h)==FILE_TYPE_DISK since it's not on CE.
    ZDWORD res = ZGetFilePosU(h);
    canseek = (res!=0xFFFFFFFF);
  }
  ZLUFILE *lf = new ZLUFILE;
  if (flags==ZZIP_HANDLE||flags==ZZIP_FILENAME)
  { lf->is_handle=true; lf->mustclosehandle=mustclosehandle;
    lf->canseek=canseek;
    lf->h=h; lf->herr=false;
    lf->initial_offset=0;
    if (canseek) lf->initial_offset = ZGetFilePosU(h);
  }
  else
  { lf->is_handle=false;
    lf->canseek=true;
    lf->mustclosehandle=false;
    lf->buf=z; lf->len=len; lf->pos=0; lf->initial_offset=0;
  }
  *err=ZZR_OK;
  return lf;
}


int zlufclose(ZLUFILE *stream)
{ if (stream==NULL) return EOF;
#ifdef ZIP_STD
  if (stream->mustclosehandle) fclose(stream->h);
#else
  if (stream->mustclosehandle) CloseHandle(stream->h);
#endif
  delete stream;
  return 0;
}

int zluferror(ZLUFILE *stream)
{ if (stream->is_handle && stream->herr) return 1;
  else return 0;
}

long int zluftell(ZLUFILE *stream)
{ if (stream->is_handle && stream->canseek) return ZGetFilePosU(stream->h)-stream->initial_offset;
  else if (stream->is_handle) return 0;
  else return stream->pos;
}

int zlufseek(ZLUFILE *stream, long offset, int whence)
{ if (stream->is_handle && stream->canseek)
  {
#ifdef ZIP_STD
    return fseek(stream->h,stream->initial_offset+offset,whence);
#else
    if (whence==SEEK_SET) SetFilePointer(stream->h,stream->initial_offset+offset,0,FILE_BEGIN);
    else if (whence==SEEK_CUR) SetFilePointer(stream->h,offset,NULL,FILE_CURRENT);
    else if (whence==SEEK_END) SetFilePointer(stream->h,offset,NULL,FILE_END);
    else return 19; // EINVAL
    return 0;
#endif
  }
  else if (stream->is_handle) return 29; // ESPIPE
  else
  { if (whence==SEEK_SET) stream->pos=offset;
    else if (whence==SEEK_CUR) stream->pos+=offset;
    else if (whence==SEEK_END) stream->pos=stream->len+offset;
    return 0;
  }
}


size_t zlufread(void *ptr,size_t size,size_t n,ZLUFILE *stream)
{ unsigned int toread = (unsigned int)(size*n);
  if (stream->is_handle)
  {
#ifdef ZIP_STD
    return fread(ptr,size,n,stream->h);
#else
    ZDWORD red; BOOL res = ReadFile(stream->h,ptr,toread,&red,NULL);
    if (!res) stream->herr=true;
    return red/size;
#endif
  }
  if (stream->pos+toread > stream->len) toread = stream->len-stream->pos;
  memcpy(ptr, (char*)stream->buf + stream->pos, toread); ZDWORD red = toread;
  stream->pos += red;
  return red/size;
}

// file_in_zip_read_info_s contain internal information about a file in zipfile,
//  when reading and decompress it
typedef struct
{
	char  *read_buffer;         // internal buffer for compressed data
	zz_stream stream;            // zLib stream structure for inflate

	uLong pos_in_zipfile;       // position in byte on the zipfile, for fseek
	uLong stream_initialised;   // flag set if stream structure is initialised

	uLong offset_local_extrafield;// offset of the local extra field
	uInt  size_local_extrafield;// size of the local extra field
	uLong pos_local_extrafield;   // position in the local extra field in read

	uLong crc32;                // crc32 of all data uncompressed
	uLong crc32_wait;           // crc32 we must obtain after decompress all
	uLong rest_read_compressed; // number of byte to be decompressed
	uLong rest_read_uncompressed;//number of byte to be obtained after decomp
	ZLUFILE* file;                 // io structore of the zipfile
	uLong compression_method;   // compression method (0==store)
	uLong byte_before_the_zipfile;// byte before the zipfile, (>0 for sfx)
  bool encrypted;               // is it encrypted?
  unsigned long keys[3];        // decryption keys, initialized by unzOpenCurrentFile
  int encheadleft;              // the first call(s) to unzReadCurrentFile will read this many encryption-header bytes first
  char crcenctest;              // if encrypted, we'll check the encryption buffer against this
} zfile_in_zip_read_info_s;


// unz_s contain internal information about the zipfile
typedef struct
{
	ZLUFILE* file;               // io structore of the zipfile
	zunz_global_info gi;         // public global information
	uLong byte_before_the_zipfile;// byte before the zipfile, (>0 for sfx)
	uLong num_file;             // number of the current file in the zipfile
	uLong pos_in_central_dir;   // pos of the current file in the central dir
	uLong current_file_ok;      // flag about the usability of the current file
	uLong central_pos;          // position of the beginning of the central dir

	uLong size_central_dir;     // size of the central directory
	uLong offset_central_dir;   // offset of start of central directory with respect to the starting disk number

	zunz_file_info cur_file_info; // public info about the current file in zip
	zunz_file_info_internal cur_file_info_internal; // private info about it
    zfile_in_zip_read_info_s* pfile_in_zip_read; // structure about the current file if we are decompressing it
} zunz_s, *zunzFile;


int unzStringFileNameCompare (const char* fileName1,const char* fileName2,int iCaseSensitivity);
//   Compare two filename (fileName1,fileName2).

z_off_t zunztell (zunzFile file);
//  Give the current position in uncompressed data

int zunzeof (zunzFile file);
//  return 1 if the end of file was reached, 0 elsewhere

int zunzGetLocalExtrafield (zunzFile file, voidp buf, unsigned len);
//  Read extra field from the current file (opened by unzOpenCurrentFile)
//  This is the local-header version of the extra field (sometimes, there is
//    more info in the local-header version than in the central-header)
//
//  if buf==NULL, it return the size of the local extra field
//
//  if buf!=NULL, len is the size of the buffer, the extra header is copied in
//	buf.
//  the return value is the number of bytes copied in buf, or (if <0)
//	the error code



// ===========================================================================
//   Read a byte from a gz_stream; update next_in and avail_in. Return EOF
// for end of file.
// IN assertion: the stream s has been sucessfully opened for reading.

int zunzlocal_getByte(ZLUFILE *fin,int *pi)
{ unsigned char c;
  int err = (int)zlufread(&c, 1, 1, fin);
  if (err==1)
  { *pi = (int)c;
    return ZUNZ_OK;
  }
  else
  { if (zluferror(fin)) return ZUNZ_ERRNO;
    else return ZUNZ_EOF;
  }
}


// ===========================================================================
// Reads a long in LSB order from the given gz_stream. Sets
int zunzlocal_getShort (ZLUFILE *fin,uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = zunzlocal_getByte(fin,&i);
    x = (uLong)i;

    if (err==ZUNZ_OK)
        err = zunzlocal_getByte(fin,&i);
    x += ((uLong)i)<<8;

    if (err==ZUNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}

int zunzlocal_getLong (ZLUFILE *fin,uLong *pX)
{
    uLong x ;
    int i;
    int err;

    err = zunzlocal_getByte(fin,&i);
    x = (uLong)i;

    if (err==ZUNZ_OK)
        err = zunzlocal_getByte(fin,&i);
    x += ((uLong)i)<<8;

    if (err==ZUNZ_OK)
        err = zunzlocal_getByte(fin,&i);
    x += ((uLong)i)<<16;

    if (err==ZUNZ_OK)
        err = zunzlocal_getByte(fin,&i);
    x += ((uLong)i)<<24;

    if (err==ZUNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return err;
}


// My own strcmpi / strcasecmp
int zstrcmpcasenosensitive_internal (const char* fileName1,const char *fileName2)
{
	for (;;)
	{
		char c1=*(fileName1++);
		char c2=*(fileName2++);
		if ((c1>='a') && (c1<='z'))
			c1 -= (char)0x20;
		if ((c2>='a') && (c2<='z'))
			c2 -= (char)0x20;
		if (c1=='\0')
			return ((c2=='\0') ? 0 : -1);
		if (c2=='\0')
			return 1;
		if (c1<c2)
			return -1;
		if (c1>c2)
			return 1;
	}
}




//
// Compare two filename (fileName1,fileName2).
// If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
// If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi or strcasecmp)
//
int zunzStringFileNameCompare (const char*fileName1,const char*fileName2,int iCaseSensitivity)
{ if (iCaseSensitivity==1) return strcmp(fileName1,fileName2);
  else return zstrcmpcasenosensitive_internal(fileName1,fileName2);
}

#define ZBUFREADCOMMENT (0x400)


//  Locate the Central directory of a zipfile (at the end, just before
// the global comment). Lu bugfix 2005.07.26 - returns 0xFFFFFFFF if not found,
// rather than 0, since 0 is a valid central-dir-location for an empty zipfile.
uLong zunzlocal_SearchCentralDir(ZLUFILE *fin)
{ if (zlufseek(fin,0,SEEK_END) != 0) return 0xFFFFFFFF;
  uLong uSizeFile = zluftell(fin);

  uLong uMaxBack=0xffff; // maximum size of global comment
  if (uMaxBack>uSizeFile) uMaxBack = uSizeFile;

  unsigned char *buf = (unsigned char*)zzmalloc(ZBUFREADCOMMENT+4);
  if (buf==NULL) return 0xFFFFFFFF;
  uLong uPosFound=0xFFFFFFFF;

  uLong uBackRead = 4;
  while (uBackRead<uMaxBack)
  { uLong uReadSize,uReadPos ;
    int i;
    if (uBackRead+ZBUFREADCOMMENT>uMaxBack) uBackRead = uMaxBack;
    else uBackRead+=ZBUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;
    uReadSize = ((ZBUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ? (ZBUFREADCOMMENT+4) : (uSizeFile-uReadPos);
    if (zlufseek(fin,uReadPos,SEEK_SET)!=0) break;
    if (zlufread(buf,(uInt)uReadSize,1,fin)!=1) break;
    for (i=(int)uReadSize-3; (i--)>=0;)
    { if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&	((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      { uPosFound = uReadPos+i;	break;
      }
    }
    if (uPosFound!=0) break;
  }
  if (buf) zzfree(buf);
  return uPosFound;
}


int zunzGoToFirstFile (zunzFile file);
int zunzCloseCurrentFile (zunzFile file);

// Open a Zip file.
// If the zipfile cannot be opened (file don't exist or in not valid), return NULL.
// Otherwise, the return value is a unzFile Handle, usable with other unzip functions
zunzFile zunzOpenInternal(ZLUFILE *fin)
{ if (fin==NULL) return NULL;
  if (zunz_copyright[0]!=' ') {zlufclose(fin); return NULL;}

  int err=ZUNZ_OK;
  zunz_s us={0};
  uLong central_pos=0,uL=0;
  central_pos = zunzlocal_SearchCentralDir(fin);
  if (central_pos==0xFFFFFFFF) err=ZUNZ_ERRNO;
  if (err==ZUNZ_OK && zlufseek(fin,central_pos,SEEK_SET)!=0) err=ZUNZ_ERRNO;
  // the signature, already checked
  if (err==ZUNZ_OK && zunzlocal_getLong(fin,&uL)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // number of this disk
  uLong number_disk=0;          // number of the current dist, used for spanning ZIP, unsupported, always 0
  if (err==ZUNZ_OK && zunzlocal_getShort(fin,&number_disk)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // number of the disk with the start of the central directory
  uLong number_disk_with_CD=0;  // number the the disk with central dir, used for spaning ZIP, unsupported, always 0
  if (err==ZUNZ_OK && zunzlocal_getShort(fin,&number_disk_with_CD)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // total number of entries in the central dir on this disk
  if (err==ZUNZ_OK && zunzlocal_getShort(fin,&us.gi.number_entry)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // total number of entries in the central dir
  uLong number_entry_CD=0;      // total number of entries in the central dir (same than number_entry on nospan)
  if (err==ZUNZ_OK && zunzlocal_getShort(fin,&number_entry_CD)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  if (err==ZUNZ_OK && ((number_entry_CD!=us.gi.number_entry) || (number_disk_with_CD!=0) || (number_disk!=0))) err=ZUNZ_BADZIPFILE;
  // size of the central directory
  if (err==ZUNZ_OK && zunzlocal_getLong(fin,&us.size_central_dir)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // offset of start of central directory with respect to the starting disk number
  if (err==ZUNZ_OK && zunzlocal_getLong(fin,&us.offset_central_dir)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  // zipfile comment length
  if (err==ZUNZ_OK && zunzlocal_getShort(fin,&us.gi.size_comment)!=ZUNZ_OK) err=ZUNZ_ERRNO;
  if (err==ZUNZ_OK && ((central_pos+fin->initial_offset<us.offset_central_dir+us.size_central_dir) && (err==ZUNZ_OK))) err=ZUNZ_BADZIPFILE;
  if (err!=ZUNZ_OK) {zlufclose(fin);return NULL;}

  us.file=fin;
  us.byte_before_the_zipfile = central_pos+fin->initial_offset - (us.offset_central_dir+us.size_central_dir);
  us.central_pos = central_pos;
  us.pfile_in_zip_read = NULL;
  fin->initial_offset = 0; // since the zipfile itself is expected to handle this

  zunz_s *s = (zunz_s*)zzmalloc(sizeof(zunz_s));
  *s=us;
  zunzGoToFirstFile((zunzFile)s);
  return (zunzFile)s;
}



//  Close a ZipFile opened with unzipOpen.
//  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
//    these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
//  return UNZ_OK if there is no problem.
int zunzClose (zunzFile file)
{
	zunz_s* s;
	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;

    if (s->pfile_in_zip_read!=NULL)
        zunzCloseCurrentFile(file);

	zlufclose(s->file);
	if (s) zzfree(s); // unused s=0;
	return ZUNZ_OK;
}


//  Write info about the ZipFile in the *pglobal_info structure.
//  No preparation of the structure is needed
//  return UNZ_OK if there is no problem.
int zunzGetGlobalInfo (zunzFile file,zunz_global_info *pglobal_info)
{
	zunz_s* s;
	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
	*pglobal_info=s->gi;
	return ZUNZ_OK;
}


//   Translate date/time from Dos format to tm_unz (readable more easilty)
void zunzlocal_DosDateToTmuDate (uLong ulDosDate, ztm_unz* ptm)
{
    uLong uDate;
    uDate = (uLong)(ulDosDate>>16);
    ptm->tm_mday = (uInt)(uDate&0x1f) ;
    ptm->tm_mon =  (uInt)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (uInt)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (uInt) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (uInt) (2*(ulDosDate&0x1f)) ;
}

//  Get Info about the current file in the zipfile, with internal only info
int zunzlocal_GetCurrentFileInfoInternal (zunzFile file,
                                                  zunz_file_info *pfile_info,
                                                  zunz_file_info_internal
                                                  *pfile_info_internal,
                                                  char *szFileName,
												  uLong fileNameBufferSize,
                                                  void *extraField,
												  uLong extraFieldBufferSize,
                                                  char *szComment,
												  uLong commentBufferSize);

int zunzlocal_GetCurrentFileInfoInternal (zunzFile file, zunz_file_info *pfile_info,
   zunz_file_info_internal *pfile_info_internal, char *szFileName,
   uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize,
   char *szComment, uLong commentBufferSize)
{
	zunz_s* s;
	zunz_file_info file_info;
	zunz_file_info_internal file_info_internal;
	int err=ZUNZ_OK;
	uLong uMagic;
	long lSeek=0;

	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
	if (zlufseek(s->file,s->pos_in_central_dir+s->byte_before_the_zipfile,SEEK_SET)!=0)
		err=ZUNZ_ERRNO;


	// we check the magic
	if (err==ZUNZ_OK)
		if (zunzlocal_getLong(s->file,&uMagic) != ZUNZ_OK)
			err=ZUNZ_ERRNO;
		else if (uMagic!=0x02014b50)
			err=ZUNZ_BADZIPFILE;

	if (zunzlocal_getShort(s->file,&file_info.version) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.version_needed) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.flag) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.compression_method) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&file_info.dosDate) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

    zunzlocal_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

	if (zunzlocal_getLong(s->file,&file_info.crc) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&file_info.compressed_size) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&file_info.uncompressed_size) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.size_filename) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.size_file_extra) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.size_file_comment) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.disk_num_start) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&file_info.internal_fa) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&file_info.external_fa) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&file_info_internal.offset_curfile) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	lSeek+=file_info.size_filename;
	if ((err==ZUNZ_OK) && (szFileName!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_filename<fileNameBufferSize)
		{
			*(szFileName+file_info.size_filename)='\0';
			uSizeRead = file_info.size_filename;
		}
		else
			uSizeRead = fileNameBufferSize;

		if ((file_info.size_filename>0) && (fileNameBufferSize>0))
			if (zlufread(szFileName,(uInt)uSizeRead,1,s->file)!=1)
				err=ZUNZ_ERRNO;
		lSeek -= uSizeRead;
	}


	if ((err==ZUNZ_OK) && (extraField!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_file_extra<extraFieldBufferSize)
			uSizeRead = file_info.size_file_extra;
		else
			uSizeRead = extraFieldBufferSize;

		if (lSeek!=0)
			if (zlufseek(s->file,lSeek,SEEK_CUR)==0)
				lSeek=0;
			else
				err=ZUNZ_ERRNO;
		if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
			if (zlufread(extraField,(uInt)uSizeRead,1,s->file)!=1)
				err=ZUNZ_ERRNO;
		lSeek += file_info.size_file_extra - uSizeRead;
	}
	else
		lSeek+=file_info.size_file_extra;


	if ((err==ZUNZ_OK) && (szComment!=NULL))
	{
		uLong uSizeRead ;
		if (file_info.size_file_comment<commentBufferSize)
		{
			*(szComment+file_info.size_file_comment)='\0';
			uSizeRead = file_info.size_file_comment;
		}
		else
			uSizeRead = commentBufferSize;

		if (lSeek!=0)
			if (zlufseek(s->file,lSeek,SEEK_CUR)==0)
				{} // unused lSeek=0;
			else
				err=ZUNZ_ERRNO;
		if ((file_info.size_file_comment>0) && (commentBufferSize>0))
			if (zlufread(szComment,(uInt)uSizeRead,1,s->file)!=1)
				err=ZUNZ_ERRNO;
		//unused lSeek+=file_info.size_file_comment - uSizeRead;
	}
	else {} //unused lSeek+=file_info.size_file_comment;

	if ((err==ZUNZ_OK) && (pfile_info!=NULL))
		*pfile_info=file_info;

	if ((err==ZUNZ_OK) && (pfile_info_internal!=NULL))
		*pfile_info_internal=file_info_internal;

	return err;
}



//  Write info about the ZipFile in the *pglobal_info structure.
//  No preparation of the structure is needed
//  return UNZ_OK if there is no problem.
int zunzGetCurrentFileInfo (zunzFile file, zunz_file_info *pfile_info,
  char *szFileName, uLong fileNameBufferSize, void *extraField, uLong extraFieldBufferSize,
  char *szComment, uLong commentBufferSize)
{ return zunzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,szFileName,fileNameBufferSize,
      extraField,extraFieldBufferSize, szComment,commentBufferSize);
}


//  Set the current file of the zipfile to the first file.
//  return UNZ_OK if there is no problem
int zunzGoToFirstFile (zunzFile file)
{
	int err;
	zunz_s* s;
	if (file==NULL) return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
	s->pos_in_central_dir=s->offset_central_dir;
	s->num_file=0;
	err=zunzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
											 &s->cur_file_info_internal,
											 NULL,0,NULL,0,NULL,0);
	s->current_file_ok = (err == ZUNZ_OK);
	return err;
}


//  Set the current file of the zipfile to the next file.
//  return UNZ_OK if there is no problem
//  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
int zunzGoToNextFile (zunzFile file)
{
	zunz_s* s;
	int err;

	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
	if (!s->current_file_ok)
		return ZUNZ_END_OF_LIST_OF_FILE;
	if (s->num_file+1==s->gi.number_entry)
		return ZUNZ_END_OF_LIST_OF_FILE;

	s->pos_in_central_dir += ZSIZECENTRALDIRITEM + s->cur_file_info.size_filename +
			s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
	s->num_file++;
	err = zunzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
											   &s->cur_file_info_internal,
											   NULL,0,NULL,0,NULL,0);
	s->current_file_ok = (err == ZUNZ_OK);
	return err;
}


//  Try locate the file szFileName in the zipfile.
//  For the iCaseSensitivity signification, see unzStringFileNameCompare
//  return value :
//  UNZ_OK if the file is found. It becomes the current file.
//  UNZ_END_OF_LIST_OF_FILE if the file is not found
int zunzLocateFile (zunzFile file, const char *szFileName, int iCaseSensitivity)
{
	zunz_s* s;
	int err;


	uLong num_fileSaved;
	uLong pos_in_central_dirSaved;


	if (file==NULL)
		return ZUNZ_PARAMERROR;

    if (strlen(szFileName)>=ZUNZ_MAXFILENAMEINZIP)
        return ZUNZ_PARAMERROR;

	s=(zunz_s*)file;
	if (!s->current_file_ok)
		return ZUNZ_END_OF_LIST_OF_FILE;

	num_fileSaved = s->num_file;
	pos_in_central_dirSaved = s->pos_in_central_dir;

	err = zunzGoToFirstFile(file);

	while (err == ZUNZ_OK)
	{
		char szCurrentFileName[ZUNZ_MAXFILENAMEINZIP+1];
		zunzGetCurrentFileInfo(file,NULL,
								szCurrentFileName,sizeof(szCurrentFileName)-1,
								NULL,0,NULL,0);
		if (zunzStringFileNameCompare(szCurrentFileName,szFileName,iCaseSensitivity)==0)
			return ZUNZ_OK;
		err = zunzGoToNextFile(file);
	}

	s->num_file = num_fileSaved ;
	s->pos_in_central_dir = pos_in_central_dirSaved ;
	return err;
}


//  Read the local header of the current zipfile
//  Check the coherency of the local header and info in the end of central
//        directory about this file
//  store in *piSizeVar the size of extra info in local header
//        (filename and size of extra field data)
int zunzlocal_CheckCurrentFileCoherencyHeader (zunz_s *s,uInt *piSizeVar,
  uLong *poffset_local_extrafield, uInt  *psize_local_extrafield)
{
	uLong uMagic,uData,uFlags;
	uLong size_filename;
	uLong size_extra_field;
	int err=ZUNZ_OK;

	*piSizeVar = 0;
	*poffset_local_extrafield = 0;
	*psize_local_extrafield = 0;

	if (zlufseek(s->file,s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile,SEEK_SET)!=0)
		return ZUNZ_ERRNO;


	if (err==ZUNZ_OK)
		if (zunzlocal_getLong(s->file,&uMagic) != ZUNZ_OK)
			err=ZUNZ_ERRNO;
		else if (uMagic!=0x04034b50)
			err=ZUNZ_BADZIPFILE;

	if (zunzlocal_getShort(s->file,&uData) != ZUNZ_OK)
		err=ZUNZ_ERRNO;
//	else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
//		err=UNZ_BADZIPFILE;
	if (zunzlocal_getShort(s->file,&uFlags) != ZUNZ_OK)
		err=ZUNZ_ERRNO;

	if (zunzlocal_getShort(s->file,&uData) != ZUNZ_OK)
		err=ZUNZ_ERRNO;
	else if ((err==ZUNZ_OK) && (uData!=s->cur_file_info.compression_method))
		err=ZUNZ_BADZIPFILE;

    if ((err==ZUNZ_OK) && (s->cur_file_info.compression_method!=0) &&
                         (s->cur_file_info.compression_method!=ZZ_DEFLATED))
        err=ZUNZ_BADZIPFILE;

	if (zunzlocal_getLong(s->file,&uData) != ZUNZ_OK) // date/time
		err=ZUNZ_ERRNO;

	if (zunzlocal_getLong(s->file,&uData) != ZUNZ_OK) // crc
		err=ZUNZ_ERRNO;
	else if ((err==ZUNZ_OK) && (uData!=s->cur_file_info.crc) &&
		                      ((uFlags & 8)==0))
		err=ZUNZ_BADZIPFILE;

	if (zunzlocal_getLong(s->file,&uData) != ZUNZ_OK) // size compr
		err=ZUNZ_ERRNO;
	else if ((err==ZUNZ_OK) && (uData!=s->cur_file_info.compressed_size) &&
							  ((uFlags & 8)==0))
		err=ZUNZ_BADZIPFILE;

	if (zunzlocal_getLong(s->file,&uData) != ZUNZ_OK) // size uncompr
		err=ZUNZ_ERRNO;
	else if ((err==ZUNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) &&
							  ((uFlags & 8)==0))
		err=ZUNZ_BADZIPFILE;


	if (zunzlocal_getShort(s->file,&size_filename) != ZUNZ_OK)
		err=ZUNZ_ERRNO;
	else if ((err==ZUNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
		err=ZUNZ_BADZIPFILE;

	*piSizeVar += (uInt)size_filename;

	if (zunzlocal_getShort(s->file,&size_extra_field) != ZUNZ_OK)
		err=ZUNZ_ERRNO;
	*poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
									ZSIZEZIPLOCALHEADER + size_filename;
	*psize_local_extrafield = (uInt)size_extra_field;

	*piSizeVar += (uInt)size_extra_field;

	return err;
}





//  Open for reading data the current file in the zipfile.
//  If there is no error and the file is opened, the return value is UNZ_OK.
int zunzOpenCurrentFile (zunzFile file, const char *password)
{
	int err;
	int Store;
	uInt iSizeVar;
	zunz_s* s;
	zfile_in_zip_read_info_s* pfile_in_zip_read_info;
	uLong offset_local_extrafield;  // offset of the local extra field
	uInt  size_local_extrafield;    // size of the local extra field

	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
	if (!s->current_file_ok)
		return ZUNZ_PARAMERROR;

    if (s->pfile_in_zip_read != NULL)
        zunzCloseCurrentFile(file);

	if (zunzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar,
				&offset_local_extrafield,&size_local_extrafield)!=ZUNZ_OK)
		return ZUNZ_BADZIPFILE;

	pfile_in_zip_read_info = (zfile_in_zip_read_info_s*)zzmalloc(sizeof(zfile_in_zip_read_info_s));
	if (pfile_in_zip_read_info==NULL)
		return ZUNZ_INTERNALERROR;

	pfile_in_zip_read_info->read_buffer=(char*)zzmalloc(ZUNZ_BUFSIZE);
	pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
	pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
	pfile_in_zip_read_info->pos_local_extrafield=0;

	if (pfile_in_zip_read_info->read_buffer==NULL)
	{
		if (pfile_in_zip_read_info!=0) zzfree(pfile_in_zip_read_info); //unused pfile_in_zip_read_info=0;
		return ZUNZ_INTERNALERROR;
	}

	pfile_in_zip_read_info->stream_initialised=0;

	if ((s->cur_file_info.compression_method!=0) && (s->cur_file_info.compression_method!=ZZ_DEFLATED))
        { // unused err=ZUNZ_BADZIPFILE;
        }
	Store = s->cur_file_info.compression_method==0;

	pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
	pfile_in_zip_read_info->crc32=0;
	pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
	pfile_in_zip_read_info->file=s->file;
	pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

	if (!Store)
	{
	  pfile_in_zip_read_info->stream.zalloc = (zalloc_func)0;
	  pfile_in_zip_read_info->stream.zfree = (zfree_func)0;
	  pfile_in_zip_read_info->stream.opaque = (voidpf)0;

          err=zinflateInit2(&pfile_in_zip_read_info->stream);
	  if (err == ZZ_OK)
	    pfile_in_zip_read_info->stream_initialised=1;
        // windowBits is passed < 0 to tell that there is no zlib header.
        // Note that in this case inflate *requires* an extra "dummy" byte
        // after the compressed stream in order to complete decompression and
        // return Z_STREAM_END.
        // In unzip, i don't wait absolutely Z_STREAM_END because I known the
        // size of both compressed and uncompressed data
	}
	pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size ;
	pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size ;
  pfile_in_zip_read_info->encrypted = (s->cur_file_info.flag&1)!=0;
  bool extlochead = (s->cur_file_info.flag&8)!=0;
  if (extlochead) pfile_in_zip_read_info->crcenctest = (char)((s->cur_file_info.dosDate>>8)&0xff);
  else pfile_in_zip_read_info->crcenctest = (char)(s->cur_file_info.crc >> 24);
  pfile_in_zip_read_info->encheadleft = (pfile_in_zip_read_info->encrypted?12:0);
  pfile_in_zip_read_info->keys[0] = 305419896L;
  pfile_in_zip_read_info->keys[1] = 591751049L;
  pfile_in_zip_read_info->keys[2] = 878082192L;
  for (const char *cp=password; cp!=0 && *cp!=0; cp++) zupdate_keys(pfile_in_zip_read_info->keys,*cp);//ppshuai

	pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + ZSIZEZIPLOCALHEADER +
			  iSizeVar;

	pfile_in_zip_read_info->stream.avail_in = (uInt)0;

	s->pfile_in_zip_read = pfile_in_zip_read_info;

  return ZUNZ_OK;
}


//  Read bytes from the current file.
//  buf contain buffer where data must be copied
//  len the size of buf.
//  return the number of byte copied if somes bytes are copied (and also sets *reached_eof)
//  return 0 if the end of file was reached. (and also sets *reached_eof).
//  return <0 with error code if there is an error. (in which case *reached_eof is meaningless)
//    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
int zunzReadCurrentFile  (zunzFile file, voidp buf, unsigned len, bool *reached_eof)
{ int err=ZUNZ_OK;
  uInt iRead = 0;
  if (reached_eof!=0) *reached_eof=false;

  zunz_s *s = (zunz_s*)file;
  if (s==NULL) return ZUNZ_PARAMERROR;

  zfile_in_zip_read_info_s* pfile_in_zip_read_info = s->pfile_in_zip_read;
  if (pfile_in_zip_read_info==NULL) return ZUNZ_PARAMERROR;
  if ((pfile_in_zip_read_info->read_buffer == NULL)) return ZUNZ_END_OF_LIST_OF_FILE;
  if (len==0) return 0;

  pfile_in_zip_read_info->stream.next_out = (uChar*)buf;
  pfile_in_zip_read_info->stream.avail_out = (uInt)len;

  if (len>pfile_in_zip_read_info->rest_read_uncompressed)
  { pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;
  }

  while (pfile_in_zip_read_info->stream.avail_out>0)
  { if ((pfile_in_zip_read_info->stream.avail_in==0) && (pfile_in_zip_read_info->rest_read_compressed>0))
    { uInt uReadThis = ZUNZ_BUFSIZE;
      if (pfile_in_zip_read_info->rest_read_compressed<uReadThis) uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
      if (uReadThis == 0) {if (reached_eof!=0) *reached_eof=true; return ZUNZ_EOF;}
      if (zlufseek(pfile_in_zip_read_info->file, pfile_in_zip_read_info->pos_in_zipfile + pfile_in_zip_read_info->byte_before_the_zipfile,SEEK_SET)!=0) return ZUNZ_ERRNO;
      if (zlufread(pfile_in_zip_read_info->read_buffer,uReadThis,1,pfile_in_zip_read_info->file)!=1) return ZUNZ_ERRNO;
      pfile_in_zip_read_info->pos_in_zipfile += uReadThis;
      pfile_in_zip_read_info->rest_read_compressed-=uReadThis;
      pfile_in_zip_read_info->stream.next_in = (uChar*)pfile_in_zip_read_info->read_buffer;
      pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
      //
      if (pfile_in_zip_read_info->encrypted)
      { char *buf = (char*)pfile_in_zip_read_info->stream.next_in;
        for (unsigned int i=0; i<uReadThis; i++) buf[i]=zdecode(pfile_in_zip_read_info->keys,buf[i]);
      }
    }

    unsigned int uDoEncHead = pfile_in_zip_read_info->encheadleft;
    if (uDoEncHead>pfile_in_zip_read_info->stream.avail_in) uDoEncHead=pfile_in_zip_read_info->stream.avail_in;
    if (uDoEncHead>0)
    { char bufcrc=pfile_in_zip_read_info->stream.next_in[uDoEncHead-1];
      //////////////////////////////////////////////////////////
      //Modify by ppshuai
      //注释此行错误的处理，保证数据的完整性.
      //pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoEncHead;
      pfile_in_zip_read_info->stream.next_in += uDoEncHead;
      pfile_in_zip_read_info->encheadleft -= uDoEncHead;
      if (pfile_in_zip_read_info->encheadleft==0)
      { if (bufcrc!=pfile_in_zip_read_info->crcenctest) return ZUNZ_PASSWORD;
      }
    }

    if (pfile_in_zip_read_info->compression_method==0)
    { uInt uDoCopy,i ;
      if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in)
      { uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
      }
      else
      { uDoCopy = pfile_in_zip_read_info->stream.avail_in ;
      }
      for (i=0;i<uDoCopy;i++) *(pfile_in_zip_read_info->stream.next_out+i) = *(pfile_in_zip_read_info->stream.next_in+i);
      pfile_in_zip_read_info->crc32 = zcrc32(pfile_in_zip_read_info->crc32,pfile_in_zip_read_info->stream.next_out,uDoCopy);//ppshuai
      pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
      pfile_in_zip_read_info->stream.next_out += uDoCopy;
      pfile_in_zip_read_info->stream.next_in += uDoCopy;
      pfile_in_zip_read_info->stream.total_out += uDoCopy;
      iRead += uDoCopy;
      if (pfile_in_zip_read_info->rest_read_uncompressed==0) {if (reached_eof!=0) *reached_eof=true;}
    }
    else
    { uLong uTotalOutBefore,uTotalOutAfter;
      const uChar *bufBefore;
      uLong uOutThis;
      int flush=ZZ_SYNC_FLUSH;
      uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
      bufBefore = pfile_in_zip_read_info->stream.next_out;
      //
      err=zinflate(&pfile_in_zip_read_info->stream,flush);
      //
      uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
      uOutThis = uTotalOutAfter-uTotalOutBefore;
      pfile_in_zip_read_info->crc32 = zcrc32(pfile_in_zip_read_info->crc32,bufBefore,(uInt)(uOutThis));//ppshuai
      pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;
      iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);
      if (err==ZZ_STREAM_END && pfile_in_zip_read_info->rest_read_uncompressed==0)//ppshuai
      { if (reached_eof!=0) *reached_eof=true;
        return iRead;
      }
      if (err!=ZZ_OK) break;
    }
  }

  if (err==ZZ_OK) return iRead;
  return err;
}


//  Give the current position in uncompressed data
z_off_t unztell (zunzFile file)
{
	zunz_s* s;
	zfile_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return ZUNZ_PARAMERROR;

	return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}


//  return 1 if the end of file was reached, 0 elsewhere
int zunzeof (zunzFile file)
{
	zunz_s* s;
	zfile_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return ZUNZ_PARAMERROR;

	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
		return 1;
	else
		return 0;
}



//  Read extra field from the current file (opened by unzOpenCurrentFile)
//  This is the local-header version of the extra field (sometimes, there is
//    more info in the local-header version than in the central-header)
//  if buf==NULL, it return the size of the local extra field that can be read
//  if buf!=NULL, len is the size of the buffer, the extra header is copied in buf.
//  the return value is the number of bytes copied in buf, or (if <0) the error code
int zunzGetLocalExtrafield (zunzFile file,voidp buf,unsigned len)
{
	zunz_s* s;
	zfile_in_zip_read_info_s* pfile_in_zip_read_info;
	uInt read_now;
	uLong size_to_read;

	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return ZUNZ_PARAMERROR;

	size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
				pfile_in_zip_read_info->pos_local_extrafield);

	if (buf==NULL)
		return (int)size_to_read;

	if (len>size_to_read)
		read_now = (uInt)size_to_read;
	else
		read_now = (uInt)len ;

	if (read_now==0)
		return 0;

	if (zlufseek(pfile_in_zip_read_info->file, pfile_in_zip_read_info->offset_local_extrafield +  pfile_in_zip_read_info->pos_local_extrafield,SEEK_SET)!=0)
		return ZUNZ_ERRNO;

	if (zlufread(buf,(uInt)size_to_read,1,pfile_in_zip_read_info->file)!=1)
		return ZUNZ_ERRNO;

	return (int)read_now;
}

//  Close the file in zip opened with unzipOpenCurrentFile
//  Return UNZ_CRCERROR if all the file was read but the CRC is not good
int zunzCloseCurrentFile (zunzFile file)
{
	int err=ZUNZ_OK;

	zunz_s* s;
	zfile_in_zip_read_info_s* pfile_in_zip_read_info;
	if (file==NULL)
		return ZUNZ_PARAMERROR;
	s=(zunz_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

	if (pfile_in_zip_read_info==NULL)
		return ZUNZ_PARAMERROR;


	if (pfile_in_zip_read_info->rest_read_uncompressed == 0)
	{
		if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
			err=ZUNZ_CRCERROR;
	}


	if (pfile_in_zip_read_info->read_buffer!=0)
        { void *buf = pfile_in_zip_read_info->read_buffer;
          zzfree(buf);
          pfile_in_zip_read_info->read_buffer=0;
        }
	pfile_in_zip_read_info->read_buffer = NULL;
	if (pfile_in_zip_read_info->stream_initialised)
		zinflateEnd(&pfile_in_zip_read_info->stream);

	pfile_in_zip_read_info->stream_initialised = 0;
        if (pfile_in_zip_read_info!=0) zzfree(pfile_in_zip_read_info); // unused pfile_in_zip_read_info=0;

    s->pfile_in_zip_read=NULL;

	return err;
}


//  Get the global comment string of the ZipFile, in the szComment buffer.
//  uSizeBuf is the size of the szComment buffer.
//  return the number of byte copied or an error code <0
int zunzGetGlobalComment (zunzFile file, char *szComment, uLong uSizeBuf)
{ //int err=UNZ_OK;
  zunz_s* s;
  uLong uReadThis ;
  if (file==NULL) return ZUNZ_PARAMERROR;
  s=(zunz_s*)file;
  uReadThis = uSizeBuf;
  if (uReadThis>s->gi.size_comment) uReadThis = s->gi.size_comment;
  if (zlufseek(s->file,s->central_pos+22,SEEK_SET)!=0) return ZUNZ_ERRNO;
  if (uReadThis>0)
  { *szComment='\0';
    if (zlufread(szComment,(uInt)uReadThis,1,s->file)!=1) return ZUNZ_ERRNO;
  }
  if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment)) *(szComment+s->gi.size_comment)='\0';
  return (int)uReadThis;
}





int zunzOpenCurrentFile (zunzFile file, const char *password);
int zunzReadCurrentFile (zunzFile file, void *buf, unsigned len);
int zunzCloseCurrentFile (zunzFile file);




class TUnzip
{ public:
  TUnzip(const char *pwd) : uf(0), unzbuf(0), currentfile(-1), czei(-1), password(0) {if (pwd!=0) {password=new char[strlen(pwd)+1]; strcpy(password,pwd);}}
  ~TUnzip() {if (password!=0) delete[] password; password=0; if (unzbuf!=0) delete[] unzbuf; unzbuf=0;}

  zunzFile uf; int currentfile; ZIPENTRY cze; int czei;
  char *password;
  char *unzbuf;            // lazily created and destroyed, used by Unzip
  ZTCHAR rootdir[_Z_MAX_PATH]; // includes a trailing slash

  ZZRESULT Open(void *z,unsigned int len,ZDWORD flags);
  ZZRESULT Get(int index,ZIPENTRY *ze);
  ZZRESULT Find(const ZTCHAR *name,bool ic,int *index,ZIPENTRY *ze);
  ZZRESULT Unzip(int index,void *dst,unsigned int len,ZDWORD flags);
  ZZRESULT SetUnzipBaseDir(const ZTCHAR *dir);
  ZZRESULT Close();
};


ZZRESULT TUnzip::Open(void *z,unsigned int len,ZDWORD flags)
{ if (uf!=0 || currentfile!=-1) return ZZR_NOTINITED;
  //
#ifdef ZIP_STD
  getcwd(rootdir,_Z_MAX_PATH-1);
#else
#ifdef GetCurrentDirectory
  GetCurrentDirectory(_Z_MAX_PATH-1,rootdir);
#else
  rootdir[0]='/'; rootdir[1]=0;
#endif
#endif
  ZTCHAR *lastchar = &rootdir[z_tcslen(rootdir)-1];
  if (*lastchar!='\\' && *lastchar!='/') {lastchar[1]='/'; lastchar[2]=0;}
  //
  if (flags==ZZIP_HANDLE)
  { // test if we can seek on it. We can't use GetFileType(h)==FILE_TYPE_DISK since it's not on CE.
    ZDWORD res = ZGetFilePosU((ZHANDLE)z);
    bool canseek = (res!=0xFFFFFFFF);
    if (!canseek) return ZZR_SEEK;
  }
  ZZRESULT e; ZLUFILE *f = zlufopen(z,len,flags,&e);
  if (f==NULL) return e;
  uf = zunzOpenInternal(f);
  if (uf==0) return ZZR_NOFILE;
  return ZZR_OK;
}

ZZRESULT TUnzip::SetUnzipBaseDir(const ZTCHAR *dir)
{ z_tcsncpy(rootdir,dir,_Z_MAX_PATH-1);
  ZTCHAR *lastchar = &rootdir[z_tcslen(rootdir)-1];
  if (*lastchar!='\\' && *lastchar!='/') {lastchar[1]='/'; lastchar[2]=0;}
  return ZZR_OK;
}

ZZRESULT TUnzip::Get(int index,ZIPENTRY *ze)
{ if (index<-1 || index>=(int)uf->gi.number_entry) return ZZR_ARGS;
  if (currentfile!=-1) zunzCloseCurrentFile(uf); currentfile=-1;
  if (index==czei && index!=-1) {memcpy(ze,&cze,sizeof(ZIPENTRY)); return ZZR_OK;}
  if (index==-1)
  { ze->index = uf->gi.number_entry;
    ze->name[0]=0;
    ze->attr=0;
#ifdef ZIP_STD
    ze->atime=0;
    ze->ctime=0;
    ze->mtime=0;
#else
    ze->atime.dwLowDateTime=0; ze->atime.dwHighDateTime=0;
    ze->ctime.dwLowDateTime=0; ze->ctime.dwHighDateTime=0;
    ze->mtime.dwLowDateTime=0; ze->mtime.dwHighDateTime=0;
#endif
    ze->comp_size=0;
    ze->unc_size=0;
    return ZZR_OK;
  }
  if (index<(int)uf->num_file) zunzGoToFirstFile(uf);
  while ((int)uf->num_file<index) zunzGoToNextFile(uf);
  zunz_file_info ufi; char fn[_Z_MAX_PATH];
  zunzGetCurrentFileInfo(uf,&ufi,fn,_Z_MAX_PATH,NULL,0,NULL,0);
  // now get the extra header. We do this ourselves, instead of
  // calling unzOpenCurrentFile &c., to avoid allocating more than necessary.
  unsigned int extralen,iSizeVar; unsigned long offset;
  int res = zunzlocal_CheckCurrentFileCoherencyHeader(uf,&iSizeVar,&offset,&extralen);
  if (res!=ZUNZ_OK) return ZZR_CORRUPT;
  if (zlufseek(uf->file,offset,SEEK_SET)!=0) return ZZR_READ;
  unsigned char *extra = new unsigned char[extralen];
  if (zlufread(extra,1,(uInt)extralen,uf->file)!=extralen) {delete[] extra; return ZZR_READ;}
  //
  ze->index=uf->num_file;
  ZTCHAR tfn[_Z_MAX_PATH];
#ifdef UNICODE
  MultiByteToWideChar(CP_UTF8,0,fn,-1,tfn,_Z_MAX_PATH);
#else
  strcpy(tfn,fn);
#endif
  // As a safety feature: if the zip filename had sneaky stuff
  // like "c:\windows\file.txt" or "\windows\file.txt" or "fred\..\..\..\windows\file.txt"
  // then we get rid of them all. That way, when the programmer does UnzipItem(hz,i,ze.name),
  // it won't be a problem. (If the programmer really did want to get the full evil information,
  // then they can edit out this security feature from here).
  // In particular, we chop off any prefixes that are "c:\" or "\" or "/" or "[stuff]\.." or "[stuff]/.."
  const ZTCHAR *sfn=tfn;
  for (;;)
  { if (sfn[0]!=0 && sfn[1]==':') {sfn+=2; continue;}
    if (sfn[0]=='\\') {sfn++; continue;}
    if (sfn[0]=='/') {sfn++; continue;}
    const ZTCHAR *c;
    c=z_tcsstr(sfn,Z_T("\\..\\")); if (c!=0) {sfn=c+4; continue;}
    c=z_tcsstr(sfn,Z_T("\\../")); if (c!=0) {sfn=c+4; continue;}
    c=z_tcsstr(sfn,Z_T("/../")); if (c!=0) {sfn=c+4; continue;}
    c=z_tcsstr(sfn,Z_T("/..\\")); if (c!=0) {sfn=c+4; continue;}
    break;
  }
  z_tcsncpy(ze->name, sfn,_Z_MAX_PATH);


  unsigned long a = ufi.external_fa;
  // zip has an 'attribute' 32bit value. Its lower half is windows stuff
  // its upper half is standard unix stat.st_mode. We'll start trying
  // to read it in unix mode
  bool isdir  =   (a&0x40000000)!=0;
  bool readonly=  (a&0x00800000)==0;
  //bool readable=  (a&0x01000000)!=0; // unused
  //bool executable=(a&0x00400000)!=0; // unused
  bool hidden=false, system=false, archive=true;
  // but in normal hostmodes these are overridden by the lower half...
  int host = ufi.version>>8;
  if (host==0 || host==7 || host==11 || host==14)
  { readonly=  (a&0x00000001)!=0;
    hidden=    (a&0x00000002)!=0;
    system=    (a&0x00000004)!=0;
    isdir=     (a&0x00000010)!=0;
    archive=   (a&0x00000020)!=0;
  }
  readonly; hidden; system; isdir; archive;
  ze->attr=0;
#ifdef ZIP_STD
  ze->attr = (a&0xFFFF0000)>>16;
  if (isdir) ze->attr |= S_IFDIR;
  if (readonly) ze->attr &= ~S_IWUSR;
#else
  if (isdir) ze->attr |= FILE_ATTRIBUTE_DIRECTORY;
  if (archive) ze->attr|=FILE_ATTRIBUTE_ARCHIVE;
  if (hidden) ze->attr|=FILE_ATTRIBUTE_HIDDEN;
  if (readonly) ze->attr|=FILE_ATTRIBUTE_READONLY;
  if (system) ze->attr|=FILE_ATTRIBUTE_SYSTEM;
#endif
  ze->comp_size = ufi.compressed_size;
  ze->unc_size = ufi.uncompressed_size;
  //
  ZWORD dostime = (ZWORD)(ufi.dosDate&0xFFFF);
  ZWORD dosdate = (ZWORD)((ufi.dosDate>>16)&0xFFFF);
  ZFILETIME ftd = zdosdatetime2filetime(dosdate,dostime);
  ZFILETIME ft; LocalFileTimeToFileTime((const FILETIME *)&ftd,(LPFILETIME)&ft);
  ze->atime=ft; ze->ctime=ft; ze->mtime=ft;
  // the zip will always have at least that dostime. But if it also has
  // an extra header, then we'll instead get the info from that.
  unsigned int epos=0;
  while (epos+4<extralen)
  { char etype[3]; etype[0]=extra[epos+0]; etype[1]=extra[epos+1]; etype[2]=0;
    int size = extra[epos+2];
    if (strcmp(etype,"UT")!=0) {epos += 4+size; continue;}
    int flags = extra[epos+4];
    bool hasmtime = (flags&1)!=0;
    bool hasatime = (flags&2)!=0;
    bool hasctime = (flags&4)!=0;
    epos+=5;
    if (hasmtime)
    { z_l_utime_t mtime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
	  epos+=4;
      ze->mtime = ztimet2filetime(mtime);
    }
    if (hasatime)
    { z_l_utime_t atime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
      epos+=4;
      ze->atime = ztimet2filetime(atime);
    }
    if (hasctime)
    { z_l_utime_t ctime = ((extra[epos+0])<<0) | ((extra[epos+1])<<8) |((extra[epos+2])<<16) | ((extra[epos+3])<<24);
      epos+=4;
      ze->ctime = ztimet2filetime(ctime);
    }
    break;
  }
  //
  if (extra!=0) delete[] extra;
  memcpy(&cze,ze,sizeof(ZIPENTRY)); czei=index;
  return ZZR_OK;
}

ZZRESULT TUnzip::Find(const ZTCHAR *tname,bool ic,int *index,ZIPENTRY *ze)
{ char name[_Z_MAX_PATH];
#ifdef UNICODE
  WideCharToMultiByte(CP_UTF8,0,tname,-1,name,_Z_MAX_PATH,0,0);
#else
  strcpy(name,tname);
#endif
  int res = zunzLocateFile(uf,name,ic?ZCASE_INSENSITIVE:ZCASE_SENSITIVE);
  if (res!=ZUNZ_OK)
  { if (index!=0) *index=-1;
    if (ze!=NULL) {memset(ze,0,sizeof(ZIPENTRY)); ze->index=-1;}
    return ZZR_NOTFOUND;
  }
  if (currentfile!=-1) zunzCloseCurrentFile(uf); currentfile=-1;
  int i = (int)uf->num_file;
  if (index!=NULL) *index=i;
  if (ze!=NULL)
  { ZZRESULT zres = Get(i,ze);
    if (zres!=ZZR_OK) return zres;
  }
  return ZZR_OK;
}

void EnsureDirectory(const ZTCHAR *rootdir, const ZTCHAR *dir)
{ // first check that rootdir exists. nb. rootdir has a trailing slash
  if (rootdir!=0)
  { ZTCHAR rd[_Z_MAX_PATH]; z_tcsncpy(rd,rootdir,_Z_MAX_PATH); size_t len=z_tcslen(rd);
    if (len>0 && (rd[len-1]==Z_T('/') || rd[len-1]==Z_T('\\'))) rd[len-1]=0;
#ifdef ZIP_STD
    if (!ZFileExists(rd)) lumkdir(rd);
#else
    if (!ZFileExists(rd)) CreateDirectory(rd,0);
#endif
  }
  if (*dir==0) return;
  const ZTCHAR *lastslash=dir, *c=lastslash;
  while (*c!=0) {if (*c==Z_T('/') || *c==Z_T('\\')) lastslash=c; c++;}
  const ZTCHAR *name=lastslash;
  if (lastslash!=dir)
  { ZTCHAR tmp[_Z_MAX_PATH]; memcpy(tmp,dir,sizeof(ZTCHAR)*(lastslash-dir));
    tmp[lastslash-dir]=0;
    EnsureDirectory(rootdir,tmp);
    name++;
  }
  ZTCHAR cd[_Z_MAX_PATH]; *cd=0; if (rootdir!=0) z_tcsncpy(cd,rootdir,_Z_MAX_PATH); cd[_Z_MAX_PATH-1]=0;
  size_t len=z_tcslen(cd); z_tcsncpy(cd+len,dir,_Z_MAX_PATH-len); cd[_Z_MAX_PATH-1]=0;
#ifdef ZIP_STD
  if (!ZFileExists(cd)) lumkdir(cd);
#else
  if (!ZFileExists(cd))
  { CreateDirectory(cd,0);
  }
#endif
}



ZZRESULT TUnzip::Unzip(int index,void *dst,unsigned int len,ZDWORD flags)
{ if (flags!=ZZIP_MEMORY && flags!=ZZIP_FILENAME && flags!=ZZIP_HANDLE) return ZZR_ARGS;
  if (flags==ZZIP_MEMORY)
  { if (index!=currentfile)
    { if (currentfile!=-1) zunzCloseCurrentFile(uf); currentfile=-1;
      if (index>=(int)uf->gi.number_entry) return ZZR_ARGS;
      if (index<(int)uf->num_file) zunzGoToFirstFile(uf);
      while ((int)uf->num_file<index) zunzGoToNextFile(uf);
      zunzOpenCurrentFile(uf,password); currentfile=index;
    }
    bool reached_eof;
    int res = zunzReadCurrentFile(uf,dst,len,&reached_eof);
    if (res<=0) {zunzCloseCurrentFile(uf); currentfile=-1;}
    if (reached_eof) return ZZR_OK;
    if (res>0) return ZZR_MORE;
    if (res==ZUNZ_PASSWORD) return ZZR_PASSWORD;
    return ZZR_FLATE;
  }
  // otherwise we're writing to a handle or a file
  if (currentfile!=-1) zunzCloseCurrentFile(uf); currentfile=-1;
  if (index>=(int)uf->gi.number_entry) return ZZR_ARGS;
  if (index<(int)uf->num_file) zunzGoToFirstFile(uf);
  while ((int)uf->num_file<index) zunzGoToNextFile(uf);
  ZIPENTRY ze; Get(index,&ze);
  // zipentry=directory is handled specially
#ifdef ZIP_STD
		 bool isdir = S_ISDIR(ze.attr);
#else
  bool isdir = (ze.attr&FILE_ATTRIBUTE_DIRECTORY)!=0;
#endif
  if (isdir)
  { if (flags==ZZIP_HANDLE) return ZZR_OK; // don't do anything
    const ZTCHAR *dir = (const ZTCHAR*)dst;
    bool isabsolute = (dir[0]=='/' || dir[0]=='\\' || (dir[0]!=0 && dir[1]==':'));
    if (isabsolute) EnsureDirectory(0,dir); else EnsureDirectory(rootdir,dir);
    return ZZR_OK;
  }
  // otherwise, we write the zipentry to a file/handle
  ZHANDLE h; ZTCHAR fn[_Z_MAX_PATH]; fn[0]=0;
  if (flags==ZZIP_HANDLE) h=(ZHANDLE)dst;
  else
  { const ZTCHAR *ufn = (const ZTCHAR*)dst;
    // We'll qualify all relative names to our root dir, and leave absolute names as they are
    // ufn="zipfile.txt"  dir=""  name="zipfile.txt"  fn="c:\\currentdir\\zipfile.txt"
    // ufn="dir1/dir2/subfile.txt"  dir="dir1/dir2/"  name="subfile.txt"  fn="c:\\currentdir\\dir1/dir2/subfiles.txt"
    // ufn="\z\file.txt"  dir="\z\"  name="file.txt"  fn="\z\file.txt"
    // This might be a security risk, in the case where we just use the zipentry's name as "ufn", where
    // a malicious zip could unzip itself into c:\windows. Our solution is that GetZipItem (which
    // is how the user retrieve's the file's name within the zip) never returns absolute paths.
    const ZTCHAR *name=ufn; const ZTCHAR *c=name; while (*c!=0) {if (*c=='/' || *c=='\\') name=c+1; c++;}
    ZTCHAR dir[_Z_MAX_PATH]; z_tcsncpy(dir,ufn,_Z_MAX_PATH); if (name==ufn) *dir=0; else dir[name-ufn]=0;
    bool isabsolute = (dir[0]=='/' || dir[0]=='\\' || (dir[0]!=0 && dir[1]==':'));
    if (isabsolute) {z_tsprintf(fn,Z_T("%s%s"),dir,name); EnsureDirectory(0,dir);}
    else {z_tsprintf(fn,Z_T("%s%s%s"),rootdir,dir,name); EnsureDirectory(rootdir,dir);}
    //
#ifdef ZIP_STD
    h = fopen(fn,"wb");
#else
    h = CreateFile(fn,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,ze.attr,NULL);
#endif
  }
  if (h==Z_INVALID_HANDLE_VALUE) return ZZR_NOFILE;
  zunzOpenCurrentFile(uf,password);
  if (unzbuf==0) unzbuf=new char[16384]; ZDWORD haderr=0;
  //

  for (; haderr==0;)
  { bool reached_eof;
    int res = zunzReadCurrentFile(uf,unzbuf,16384,&reached_eof);
    if (res==ZUNZ_PASSWORD) {haderr=ZZR_PASSWORD; break;}
    if (res<0) {haderr=ZZR_FLATE; break;}
#ifdef ZIP_STD
    if (res>0) {size_t writ=fwrite(unzbuf,1,res,h); if (writ<(size_t)res) {haderr=ZZR_WRITE; break;}}
#else
    if (res>0) {ZDWORD writ; BOOL bres=WriteFile(h,unzbuf,res,&writ,NULL); if (!bres) {haderr=ZZR_WRITE; break;}}
#endif
    if (reached_eof) break;
    if (res==0) {haderr=ZZR_FLATE; break;}
  }
  zunzCloseCurrentFile(uf);
#ifdef ZIP_STD
  if (flags!=ZZIP_HANDLE) fclose(h);
  if (*fn!=0) {struct utimbuf ubuf; ubuf.actime=ze.atime; ubuf.modtime=ze.mtime; utime(fn,&ubuf);}
#else
  if (!haderr) SetFileTime(h,&ze.ctime,&ze.atime,&ze.mtime); // may fail if it was a pipe
  if (flags!=ZZIP_HANDLE) CloseHandle(h);
#endif
  if (haderr!=0) return haderr;
  return ZZR_OK;
}

ZZRESULT TUnzip::Close()
{ if (currentfile!=-1) zunzCloseCurrentFile(uf); currentfile=-1;
  if (uf!=0) zunzClose(uf); uf=0;
  return ZZR_OK;
}

ZZRESULT zlasterrorU=ZZR_OK;

unsigned int ZFormatZipMessageU(ZZRESULT code, ZTCHAR *buf,unsigned int len)
{ if (code==ZZR_RECENT) code=zlasterrorU;
  const ZTCHAR *msg=Z_T("unknown zip result code");
  switch (code)
  { case ZZR_OK: msg=Z_T("Success"); break;
    case ZZR_NODUPH: msg=Z_T("Culdn't duplicate handle"); break;
    case ZZR_NOFILE: msg=Z_T("Couldn't create/open file"); break;
    case ZZR_NOALLOC: msg=Z_T("Failed to allocate memory"); break;
    case ZZR_WRITE: msg=Z_T("Error writing to file"); break;
    case ZZR_NOTFOUND: msg=Z_T("File not found in the zipfile"); break;
    case ZZR_MORE: msg=Z_T("Still more data to unzip"); break;
    case ZZR_CORRUPT: msg=Z_T("Zipfile is corrupt or not a zipfile"); break;
    case ZZR_READ: msg=Z_T("Error reading file"); break;
    case ZZR_PASSWORD: msg=Z_T("Correct password required"); break;
    case ZZR_ARGS: msg=Z_T("Caller: faulty arguments"); break;
    case ZZR_PARTIALUNZ: msg=Z_T("Caller: the file had already been partially unzipped"); break;
    case ZZR_NOTMMAP: msg=Z_T("Caller: can only get memory of a memory zipfile"); break;
    case ZZR_MEMSIZE: msg=Z_T("Caller: not enough space allocated for memory zipfile"); break;
    case ZZR_FAILED: msg=Z_T("Caller: there was a previous error"); break;
    case ZZR_ENDED: msg=Z_T("Caller: additions to the zip have already been ended"); break;
    case ZZR_ZMODE: msg=Z_T("Caller: mixing creation and opening of zip"); break;
    case ZZR_NOTINITED: msg=Z_T("Zip-bug: internal initialisation not completed"); break;
    case ZZR_SEEK: msg=Z_T("Zip-bug: trying to seek the unseekable"); break;
    case ZZR_MISSIZE: msg=Z_T("Zip-bug: the anticipated size turned out wrong"); break;
    case ZZR_NOCHANGE: msg=Z_T("Zip-bug: tried to change mind, but not allowed"); break;
    case ZZR_FLATE: msg=Z_T("Zip-bug: an internal error during flation"); break;
  }
  unsigned int mlen=(unsigned int)z_tcslen(msg);
  if (buf==0 || len==0) return mlen;
  unsigned int n=mlen; if (n+1>len) n=len-1;
  z_tcsncpy(buf,msg,n); buf[n]=Z_T('\0');
  return mlen;
}


typedef struct
{ ZDWORD flag;
  TUnzip *unz;
} TUnzipHandleData;

HZIP OpenZipInternal(void *z,unsigned int len,ZDWORD flags, const char *password)
{ TUnzip *unz = new TUnzip(password);
  zlasterrorU = unz->Open(z,len,flags);
  if (zlasterrorU!=ZZR_OK) {delete unz; return 0;}
  TUnzipHandleData *han = new TUnzipHandleData;
  han->flag=1; han->unz=unz; return (HZIP)han;
}
HZIP OpenZipHandle(ZHANDLE h, const char *password) {return OpenZipInternal((void*)h,0,ZZIP_HANDLE,password);}
HZIP OpenZip(const ZTCHAR *fn, const char *password) {return OpenZipInternal((void*)fn,0,ZZIP_FILENAME,password);}
HZIP OpenZip(void *z,unsigned int len, const char *password) {return OpenZipInternal(z,len,ZZIP_MEMORY,password);}


ZZRESULT GetZipItem(HZIP hz, int index, ZIPENTRY *ze)
{ ze->index=0; *ze->name=0; ze->unc_size=0;
  if (hz==0) {zlasterrorU=ZZR_ARGS;return ZZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {zlasterrorU=ZZR_ZMODE;return ZZR_ZMODE;}
  TUnzip *unz = han->unz;
  zlasterrorU = unz->Get(index,ze);
  return zlasterrorU;
}

ZZRESULT FindZipItem(HZIP hz, const ZTCHAR *name, bool ic, int *index, ZIPENTRY *ze)
{ if (hz==0) {zlasterrorU=ZZR_ARGS;return ZZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {zlasterrorU=ZZR_ZMODE;return ZZR_ZMODE;}
  TUnzip *unz = han->unz;
  zlasterrorU = unz->Find(name,ic,index,ze);
  return zlasterrorU;
}

ZZRESULT UnzipItemInternal(HZIP hz, int index, void *dst, unsigned int len, ZDWORD flags)
{ if (hz==0) {zlasterrorU=ZZR_ARGS;return ZZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {zlasterrorU=ZZR_ZMODE;return ZZR_ZMODE;}
  TUnzip *unz = han->unz;
  zlasterrorU = unz->Unzip(index,dst,len,flags);
  return zlasterrorU;
}
ZZRESULT UnzipItemHandle(HZIP hz, int index, ZHANDLE h) {return UnzipItemInternal(hz,index,(void*)h,0,ZZIP_HANDLE);}
ZZRESULT UnzipItem(HZIP hz, int index, const ZTCHAR *fn) {return UnzipItemInternal(hz,index,(void*)fn,0,ZZIP_FILENAME);}
ZZRESULT UnzipItem(HZIP hz, int index, void *z,unsigned int len) {return UnzipItemInternal(hz,index,z,len,ZZIP_MEMORY);}

ZZRESULT SetUnzipBaseDir(HZIP hz, const ZTCHAR *dir)
{ if (hz==0) {zlasterrorU=ZZR_ARGS;return ZZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {zlasterrorU=ZZR_ZMODE;return ZZR_ZMODE;}
  TUnzip *unz = han->unz;
  zlasterrorU = unz->SetUnzipBaseDir(dir);
  return zlasterrorU;
}


ZZRESULT CloseZipU(HZIP hz)
{ if (hz==0) {zlasterrorU=ZZR_ARGS;return ZZR_ARGS;}
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  if (han->flag!=1) {zlasterrorU=ZZR_ZMODE;return ZZR_ZMODE;}
  TUnzip *unz = han->unz;
  zlasterrorU = unz->Close();
  delete unz;
  delete han;
  return zlasterrorU;
}

bool IsZipHandleU(HZIP hz)
{ if (hz==0) return false;
  TUnzipHandleData *han = (TUnzipHandleData*)hz;
  return (han->flag==1);
}

#endif // __UNZIP_H
