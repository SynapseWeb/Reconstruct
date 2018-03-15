/*

gbmtifh.h - Interface to TIFF file handling stuff

Added #defines for many more tags.
Added #defines for most of the new TIFF 6.0 tags.
Added #defines for the new tag field types.
Reading numeric tags will now accept signed field types too.

*/

#define	TE_OK		0
#define	TE_MEM		1
#define	TE_VERSION	2
#define	TE_N_TAGS	3
#define	TE_TAG_TYPE	4
#define	TE_N_IFD	5

/*
Image tags used in TIFF files. The types given with each one is what they
usually are. A program should not assume that this is always the case.
The most sensible way of handling things that could be short or long is to
use the numeric_tag() predicate and the value_of_tag() functions.
*/

/*...stags:0:*/
#define	T_NEWSUBFILETYPE	254	/* data is long */
#define	T_SUBFILETYPE		255	/* data is short */
#define	T_IMAGEWIDTH		256	/* data is a short */
#define	T_IMAGELENGTH		257	/* data is a short */
#define	T_BITSPERSAMPLE		258	/* data is a short */
#define	T_COMPRESSION		259	/* data is a short */
#define	T_PHOTOMETRIC		262	/* data is short */
#define	T_THRESHOLDING		263	/* data is short */
#define	T_CELLWIDTH		264
#define	T_CELLLENGTH		265
#define	T_FILLORDER		266	/* data is short */
#define	T_DOCNAME		269	/* data is ASCII */
#define	T_DESCRIPTION		270	/* data is ASCII */
#define	T_MAKE			271	/* data is ASCII */
#define	T_MODEL			272	/* data is ASCII */
#define	T_STRIPOFFSETS		273	/* data is short or long */
#define	T_ORIENTATION		274	/* data is short */
#define	T_SAMPLESPERPIXEL	277	/* data is short */
#define	T_ROWSPERSTRIP		278	/* data is short or long */
#define	T_STRIPBYTECOUNTS	279	/* data is short */
#define	T_MINSAMPLEVALUE	280	/* data is short or long */
#define	T_MAXSAMPLEVALUE	281	/* data is short or long */
#define	T_XRESOLUTION		282	/* data is rational */
#define	T_YRESOLUTION		283	/* data is rational */
#define	T_PLANARCONFIG		284	/* data is short */
#define	T_PAGENAME		285	/* data is ASCII */
#define	T_XPOSITION		286
#define	T_YPOSITION		287
#define	T_FREEOFFSETS		288
#define	T_FREEBYTECOUNTS	289
#define	T_GRAYRESPONSEUNIT	290	/* data is a short */
#define	T_GRAYRESPONSECURVE	291	/* data is a set of shorts */
#define	T_GROUP3OPTIONS		292	/* data is a long */
#define	T_GROUP4OPTIONS		293	/* data is a long */
#define	T_RESOLUTIONUNIT	296	/* data is short */
#define	T_PAGENUMBER		297	/* data is 2 shorts */
#define	T_COLORRESPONSECURVES	301	/* data is 3 sets of shorts */
#define	T_SOFTWARE		305	/* data is ASCII */
#define	T_DATETIME		306
#define	T_ARTIST		315	/* data is ASCII */
#define	T_HOSTCOMPUTER		316	/* data is ASCII */
#define	T_PREDICTOR		317	/* data is a short */
#define	T_WHITEPOINT		318
#define	T_PRIMARYCHROMA		319	/* data us 6 rationals */
#define	T_COLORMAP		320	/* data is 3 sets of shorts */
#define	T_HALFTONEHINTS		321	/* data is 2 shorts */
#define	T_TILEWIDTH		322	/* data is 1 short */
#define	T_TILELENGTH		323	/* data is 1 short */
#define	T_TILEOFFSETS		324	/* data is # tiles long */
#define	T_TILEBYTECOUNTS	325	/* data is # tiles long */
#define	T_INKSET		332	/* data is 1 short */
#define	T_INKNAMES		333	/* data is ASCII */
#define	T_NUMBEROFINKS		334	/* data is 1 short */
#define	T_DOTRANGE		336	/* data is 2 or 2 * samples per pixel gbm_u8s or shorts */
#define	T_TARGETPRINTER		337	/* data is ASCII */
#define	T_EXTRASAMPLES		338	/* data is 1 short */
#define	T_SAMPLEFORMAT		339	/* data is samples per pixel shorts */
#define	T_SMINSAMPLEVALUE	340	/* data is samples per pixel ? */
#define	T_SMAXSAMPLEVALUE	341	/* data is samples per pixel ? */
#define	T_TRANSFERRANGE		342	/* data is 6 shorts */
#define	T_JPEGPROC		512	/* data is 1 short */
#define	T_JPEGINTERCHANGEFMT	513	/* data is 1 long */
#define	T_JPEGINTERCHANGEFMTLEN	514	/* data is 1 long */
#define	T_JPEGRESTARTINTERVAL	515	/* data is 1 short */
#define	T_JPEGLOSSLESSPRED	517	/* data is samples per pixel shorts */
#define	T_JPEGPOINTTRANSFORMS	518	/* data is samples per pixel shorts */
#define	T_JPEGOTABLES		519	/* data is samples per pixel longs */
#define	T_JPEGDCTABLES		520	/* data is samples per pixel longs */
#define	T_JPEGACTABLES		521	/* data is samples per pixel longs */
#define	T_COEFFICIENTS		529	/* data is 3 rationals */
#define	T_SUBSAMPLING		530	/* data is 2 shorts */
#define	T_REFERENCEBLACKWHITE	532	/* data is 6 rationals */
/*...e*/

typedef struct { gbm_u32 numerator, denominator; } rational;

#define	D_BYTE		1		/* data is unsigned 8 bit */
#define	D_ASCII		2		/* data is ASCIIZ string */
#define	D_SHORT		3		/* data is unsigned 16 bit */
#define	D_LONG		4		/* data is unsigned 32 bit */
#define	D_RATIONAL	5		/* data is 2 LONGs */
#define	D_SBYTE		6		/* data is signed 8 bit */
#define	D_UNDEFINED	7		/* data 8 bit anything */
#define	D_SSHORT	8		/* data is signed 16 bit */
#define	D_SLONG		9		/* data is signed 32 bit */
#define	D_SRATIONAL	10		/* data is 2 SLONGs */
#define	D_FLOAT		11		/* data is 4-byte IEEE format */
#define	D_DOUBLE	12		/* data is 8-byte IEEE format */

typedef struct
	{
	gbm_u16 type;
	gbm_u16 data_type;
	gbm_u32	length;
	void	*value;
	} TAG;

#define	MAX_TAGS	200

typedef struct
	{
	gbm_u16 n_tags;
	TAG     tags [MAX_TAGS];
	} IFD;

typedef struct
	{
	gbm_u16 byte_order;
	gbm_u16 version_no;
	IFD     *ifd;
	} IFH;

#ifndef _GBMTIFH_

/*
Proposed method for loading a TIFF file :-
	1) Open file.
	2) Use read_ifh_and_ifd() to get header.
	   If an error occurs returned error code not TE_OK.
	3) Use locate_tag() and value_of_tag() etc. to test presence and
	   validity of tags.
	4) In particular use value_of_tag_n() to get at T_STRIPOFFSETS,
	   to extract the offsets into the file where the raw data is.
	5) Read the raw data.
	6) Close the file.
	7) Free the IFH structure using free_ifh().
*/

extern int read_ifh_and_ifd(int fd, int n_ifds_to_skip, IFH **ifh_return);
extern gbm_boolean numeric_tag(TAG *tag);
extern long value_of_tag(TAG *tag);
extern long value_of_tag_n(TAG *tag, int n);
extern TAG *locate_tag(IFD *ifd, gbm_u16 type);
extern void free_ifh(IFH *ifh);

/*
Proposed method for saving a TIFF file :-
	1) Open file
	   If fails use tiff_errno.
	2) Use make_ifh() to create an empty IFH (and its IFD).
	   If fails then out of memory.
	3) Setup all the tags - those that you don't know set to 0.
	   If fails then out of memory.
	4) Use write_ifh_and_ifd() to write the IFH.
	   If fails look at tiff_errno.
	5) Write all the raw data to the file, using ftell() to
	   keep a record of where strips start and how long they are.
	6) In particular update tags T_STRIPOFFSETS and T_STRIPBYTECOUNTS.
	7) Use write_ifd() to rewrite the IFD.
	8) Close the file.
	9) Use free_ifh() to deallocate the IFH.

It is important to realise that a second write of the IFD is necessary since
the StripOffsets and StripByteCounts tags cannot be known until the compressed
data has actually been written. Although the TIFF spec allows us to write the
compressed data BEFORE we write the IFD, I find that some programs (notably
the IBM utility IDUCNVT.EXE) insist that the IFD is before the data).
*/

extern IFH *make_ifh(void);
extern gbm_boolean add_byte_tag(IFD *ifd, gbm_u16 type, gbm_u8 *value, int n);
extern gbm_boolean add_ascii_tag(IFD *ifd, gbm_u16 type, char *value);
extern gbm_boolean add_short_tag(IFD *ifd, gbm_u16 type, gbm_u16 *value, int n);
extern gbm_boolean add_long_tag(IFD *ifd, gbm_u16 type, gbm_u32 *value, int n);
extern gbm_boolean add_rational_tag(IFD *ifd, gbm_u16 type, rational *value, int n);
extern gbm_boolean write_ifh_and_ifd(IFH *ifh, int fd);
extern void update_byte_tag(IFD *ifd, gbm_u16 type, const gbm_u8 *value);
extern void update_ascii_tag(IFD *ifd, gbm_u16 type, const char *value);
extern void update_short_tag(IFD *ifd, gbm_u16 type, const gbm_u16 *value);
extern void update_long_tag(IFD *ifd, gbm_u16 type, const gbm_u32 *value);
extern void update_rational_tag(IFD *ifd, gbm_u16 type, const rational *value);
extern gbm_boolean update_ifd(IFD *ifd, int fd);

#endif
