/* libtiff/tif_config.h.in.  IMLIB - Modified for IM.  */

/* Define if building universal (internal helper macro) */
#undef AC_APPLE_UNIVERSAL_BUILD

/* Support CCITT Group 3 & 4 algorithms */
#define CCITT_SUPPORT  /* IMLIB */

/* Pick up YCbCr subsampling info from the JPEG data stream to support files
   lacking the tag (default enabled). */
#define CHECK_JPEG_YCBCR_SUBSAMPLING  /* IMLIB */

/* enable partial strip reading for large strips (experimental) */
#undef CHUNKY_STRIP_READ_SUPPORT

/* Support C++ stream API (requires C++ compiler) */
#undef CXX_SUPPORT

/* Treat extra sample as alpha (default enabled). The RGBA interface will
   treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many
   packages produce RGBA files but don't mark the alpha properly. */
#define DEFAULT_EXTRASAMPLE_AS_ALPHA  /* IMLIB */

/* enable deferred strip/tile offset/size loading (experimental) */
#undef DEFER_STRILE_LOAD

/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H   /* IMLIB */

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H    /* IMLIB */

/* Define to 1 if you have the `floor' function. */
#define HAVE_FLOOR      /* IMLIB */

/* Define to 1 if you have the `getopt' function. */
#undef HAVE_GETOPT

/* Define to 1 if you have the <GLUT/glut.h> header file. */
#undef HAVE_GLUT_GLUT_H

/* Define to 1 if you have the <GL/glut.h> header file. */
#undef HAVE_GL_GLUT_H

/* Define to 1 if you have the <GL/glu.h> header file. */
#undef HAVE_GL_GLU_H

/* Define to 1 if you have the <GL/gl.h> header file. */
#undef HAVE_GL_GL_H

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#define HAVE_IEEEFP     /* IMLIB */

/* Define to 1 if the system has the type `int16'. */
#undef HAVE_INT16

/* Define to 1 if the system has the type `int32'. */
#undef HAVE_INT32

/* Define to 1 if the system has the type `int8'. */
#undef HAVE_INT8

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <io.h> header file. */
#undef HAVE_IO_H

/* Define to 1 if you have the `isascii' function. */
#undef HAVE_ISASCII

/* Define to 1 if you have the `jbg_newlen' function. */
#undef HAVE_JBG_NEWLEN

/* Define to 1 if you have the `lfind' function. */
#undef HAVE_LFIND

/* Define to 1 if you have the `c' library (-lc). */
#undef HAVE_LIBC

/* Define to 1 if you have the `m' library (-lm). */
#undef HAVE_LIBM

/* Define to 1 if you have the <limits.h> header file. */
#undef HAVE_LIMITS_H

/* Define to 1 if you have the <malloc.h> header file. */
#undef HAVE_MALLOC_H

/* Define to 1 if you have the `memmove' function. */
#undef HAVE_MEMMOVE

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the `memset' function. */
#undef HAVE_MEMSET

/* Define to 1 if you have the `mmap' function. */
#undef HAVE_MMAP

/* Define to 1 if you have the <OpenGL/glu.h> header file. */
#undef HAVE_OPENGL_GLU_H

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
#undef HAVE_OPENGL_GL_H

/* Define to 1 if you have the `pow' function. */
#undef HAVE_POW

/* Define if you have POSIX threads libraries and header files. */
#undef HAVE_PTHREAD

/* Define to 1 if you have the <search.h> header file. */
#undef HAVE_SEARCH_H

/* Define to 1 if you have the `setmode' function. */
#undef HAVE_SETMODE

/* Define to 1 if you have the `sqrt' function. */
#undef HAVE_SQRT

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H

/* Define to 1 if you have the `strcasecmp' function. */
#undef HAVE_STRCASECMP

/* Define to 1 if you have the `strchr' function. */
#undef HAVE_STRCHR

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H    /* IMLIB */

/* Define to 1 if you have the `strrchr' function. */
#undef HAVE_STRRCHR

/* Define to 1 if you have the `strstr' function. */
#undef HAVE_STRSTR

/* Define to 1 if you have the `strtol' function. */
#undef HAVE_STRTOL

/* Define to 1 if you have the `strtoul' function. */
#undef HAVE_STRTOUL

/* Define to 1 if you have the `strtoull' function. */
#undef HAVE_STRTOULL

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H   /* IMLIB */

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Use nonstandard varargs form for the GLU tesselator callback */
#undef HAVE_VARARGS_GLU_TESSCB

/* Define to 1 if you have the <windows.h> header file. */
#undef HAVE_WINDOWS_H

/* Native cpu byte order: 1 if big-endian (Motorola) or 0 if little-endian
   (Intel) */
#undef HOST_BIGENDIAN

/* Set the native cpu bit order (FILLORDER_LSB2MSB or FILLORDER_MSB2LSB) */
#define HOST_FILLORDER FILLORDER_LSB2MSB  /* IMLIB - NOT used, but must be defined */

/* Support ISO JBIG compression (requires JBIG-KIT library) */
#undef JBIG_SUPPORT

/* 8/12 bit libjpeg dual mode enabled */
#undef JPEG_DUAL_MODE_8_12

/* Support JPEG compression (requires IJG JPEG library) */
#define JPEG_SUPPORT     /* IMLIB */

/* 12bit libjpeg primary include file with path */
#undef LIBJPEG_12_PATH

/* Support LogLuv high dynamic range encoding */
#define LOGLUV_SUPPORT   /* IMLIB */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#undef LT_OBJDIR

/* Support LZMA2 compression */
#undef LZMA_SUPPORT

/* Support LZW algorithm */
#define LZW_SUPPORT      /* IMLIB */

/* Support Microsoft Document Imaging format */
#define MDI_SUPPORT 1    /* IMLIB */

/* Support NeXT 2-bit RLE algorithm */
#define NEXT_SUPPORT     /* IMLIB */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#undef NO_MINUS_C_MINUS_O

/* Support Old JPEG compresson (read-only) */
#define OJPEG_SUPPORT    /* IMLIB */

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Support Macintosh PackBits algorithm */
#define PACKBITS_SUPPORT                      /* IMLIB */

/* Support Pixar log-format algorithm (requires Zlib) */
#define PIXARLOG_SUPPORT                      /* IMLIB */

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#undef PTHREAD_CREATE_JOINABLE

/* The size of `signed int', as computed by sizeof. */
#define SIZEOF_SIGNED_INT 4                   /* IMLIB */

/* The size of `signed long', as computed by sizeof. */
#undef SIZEOF_SIGNED_LONG

/* The size of `signed long long', as computed by sizeof. */
#undef SIZEOF_SIGNED_LONG_LONG

/* The size of `signed short', as computed by sizeof. */
#undef SIZEOF_SIGNED_SHORT

/* The size of `unsigned char *', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_CHAR_P

/* The size of `unsigned int', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_INT

/* The size of `unsigned long', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_LONG

/* The size of `unsigned long long', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_LONG_LONG

/* The size of `unsigned short', as computed by sizeof. */
#undef SIZEOF_UNSIGNED_SHORT

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Support strip chopping (whether or not to convert single-strip uncompressed
   images to mutiple strips of specified size to reduce memory usage) */
#undef STRIPCHOP_DEFAULT

/* Default size of the strip in bytes (when strip chopping enabled) */
#undef STRIP_SIZE_DEFAULT

/* Enable SubIFD tag (330) support */
#define SUBIFD_SUPPORT                        /* IMLIB */

/* Support ThunderScan 4-bit RLE algorithm */
#define THUNDER_SUPPORT                       /* IMLIB */

/* Signed 16-bit type */
#define TIFF_INT16_T short                    /* IMLIB */

/* Signed 32-bit type formatter */
#define TIFF_INT32_FORMAT

/* Signed 32-bit type */
#define TIFF_INT32_T  int                     /* IMLIB */

/* Signed 64-bit type formatter */
#undef TIFF_INT64_FORMAT

/* Signed 64-bit type */
#define TIFF_INT64_T long long int            /* IMLIB */

/* Signed 8-bit type */
#define TIFF_INT8_T char                      /* IMLIB */

/* Pointer difference type formatter */
#undef TIFF_PTRDIFF_FORMAT

/* Pointer difference type */
#undef TIFF_PTRDIFF_T

/* Signed size type formatter */
#define TIFF_SSIZE_FORMAT "%Iu"               /* IMLIB */

/* Signed size type */
#define TIFF_SSIZE_T size_t                   /* IMLIB */

/* Unsigned 16-bit type */
#define TIFF_UINT16_T unsigned short          /* IMLIB */

/* Unsigned 32-bit type formatter */
#undef TIFF_UINT32_FORMAT

/* Unsigned 32-bit type */
#define TIFF_UINT32_T  unsigned int           /* IMLIB */

/* Unsigned 64-bit type formatter */
#define TIFF_UINT64_FORMAT  "%llu"            /* IMLIB */

/* Unsigned 64-bit type */
#define TIFF_UINT64_T unsigned long long int  /* IMLIB */

/* Unsigned 8-bit type */
#define TIFF_UINT8_T unsigned char            /* IMLIB */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#undef TM_IN_SYS_TIME

/* define to use win32 IO system */
#undef USE_WIN32_FILEIO

/* Version number of package */
#undef VERSION

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif
/* IMLIB - WORDS_BIGENDIAN defined in config.mak for IM */

/* Define to 1 if the X Window System is missing or not being used. */
#undef X_DISPLAY_MISSING

/* Support Deflate compression */
#define ZIP_SUPPORT         /* IMLIB */

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */     /* IMLIB */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline         /* IMLIB */
#endif

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */     /* IMLIB */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */    /* IMLIB */

/* To avoid the inclusion of <windows.h> */
#define AVOID_WIN32_FILEIO 1     /* IMLIB */
