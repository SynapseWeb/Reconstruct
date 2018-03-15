
/*

gbmtifh.c - Routines to handle TIFF file headers

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#define	_GBMTIFH_
#include "gbmtifh.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...vgbmtifh\46\h:0:*/
/*...e*/

/*...susefull:0:*/
#define	ifd_malloc()	((IFD *) malloc((size_t) sizeof(IFD)))
#define	ifd_free(ifd)	free((char *) ifd)

#define	ifh_malloc()	((IFH *) malloc((size_t) sizeof(IFH)))
#define	ifh_free(ifh)	free((char *) ifh)

static int sizeof_data_type(gbm_u16 data_type)
	{
	switch ( data_type )
		{
		case D_BYTE:
		case D_SBYTE:
		case D_ASCII:
		case D_UNDEFINED:
			return 1;
		case D_SHORT:
		case D_SSHORT:
			return 2;
		case D_LONG:
		case D_SLONG:
			return 4;
		case D_RATIONAL:
		case D_SRATIONAL:
			return 8;
		case D_FLOAT:
			return 4;
		case D_DOUBLE:
			return 8;
		}
	return 1;
	}

static void tag_free(TAG *tag)
	{
	free(tag->value);
	}

/*
This finds the slot for the new tag. It returns NULL if the limit of
MAX_TAGS tags is curruntly defined.
*/

static TAG *get_tag_slot(gbm_u16 type, IFD *ifd)
	{
	int i;

	if ( ifd->n_tags == MAX_TAGS )
		return NULL;

	for ( i = ifd->n_tags;
	      i > 0 && ifd->tags[i - 1].type >= type;
	      i-- )
		memcpy(ifd->tags + i,
		       ifd->tags + i - 1,
		       sizeof(TAG));

	ifd->n_tags++;

	/* now i == slot, with greater than elements moved up */

	return &(ifd->tags[i]);
	}
/*...e*/

/*...sread_ifh_and_ifd:0:*/
/*...sread_u32:0:*/
#define	make_u32(b1,b2,b3,b4) ( ((gbm_u32)(b1)<<24) | ((gbm_u32)(b2)<<16) | ((gbm_u32)(b3)<<8) | (gbm_u32)(b4) )

static gbm_u32 read_u32(int fd, gbm_boolean motorola)
	{
	gbm_u8 b[4];
	gbm_file_read(fd, b, 4);
	return  ( motorola ) ?
		make_u32(b[0], b[1], b[2], b[3]) :
		make_u32(b[3], b[2], b[1], b[0]) ;
	}
/*...e*/
/*...sread_u16:0:*/
#define	make_u16(b1,b2) ( ((short)(b1)<<8) | (short)(b2) )

static gbm_u16 read_u16(int fd, gbm_boolean motorola)
	{
	gbm_u8 b[2];
	gbm_file_read(fd, b, 2);
	return ( motorola ) ?
		make_u16(b[0], b[1]) :
		make_u16(b[1], b[0]) ;
	}
/*...e*/
/*...sread_rational:0:*/
static void read_rational(int fd, gbm_boolean motorola, rational *r)
	{
	r->numerator   = read_u32(fd, motorola);
	r->denominator = read_u32(fd, motorola);
	}
/*...e*/
/*...sread_tag:0:*/
static int read_tag(int fd, gbm_boolean motorola, TAG *tag)
	{
	int i, s, n;
	long len;
	long seek_to, old_pos;

	tag->type      = read_u16(fd, motorola);
	tag->data_type = read_u16(fd, motorola);
	tag->length    = read_u32(fd, motorola);

	if ( tag->type & 0x8000 )
		/* proprietry tag */
		{
		gbm_file_lseek(fd, 4L, SEEK_CUR);	/* skip data */
		return TE_OK;				/* assumed ok */
		}

	n   = (int) tag->length;

/*...sbugfix for UBU\39\s writing of ColorMap tag:8:*/
/* UBU writes out a length feild of 256 when it should write 768 */

if ( tag->type == T_COLORMAP && (n / 3) * 3 != n )
	n *= 3;
/*...e*/

	s   = sizeof_data_type(tag->data_type);
	len = s * n;

	if ( len > 4 )
		/* will have to seek for data */
		{
		seek_to = (long) read_u32(fd, motorola);
		old_pos = gbm_file_lseek(fd, 0L, SEEK_CUR);
		gbm_file_lseek(fd, seek_to, SEEK_SET);
		}

	if ( (tag->value = malloc((size_t) len)) == NULL )
		return TE_MEM;

	switch ( tag->data_type )
		{
		case D_BYTE:
		case D_SBYTE:
			gbm_file_read(fd, tag->value, n);
			break;
		case D_ASCII:
			gbm_file_read(fd, tag->value, n);
			break;
		case D_SHORT:
		case D_SSHORT:
			{
			gbm_u16 *gbm_u16_ptr = (gbm_u16 *) tag->value;
			for ( i = 0; i < n; i++ )
				*gbm_u16_ptr++ = read_u16(fd, motorola);
			}
			break;
		case D_LONG:
		case D_SLONG:
			{
			gbm_u32 *gbm_u32_ptr = (gbm_u32 *) tag->value;
			for ( i = 0; i < n; i++ )
				*gbm_u32_ptr++ = read_u32(fd, motorola);
			}
			break;
		case D_RATIONAL:
		case D_SRATIONAL:
			{
			rational *rational_ptr = (rational *) tag->value;
			for ( i = 0; i < n; i++ )
				read_rational(fd, motorola, rational_ptr++);
			}
			break;
		case D_FLOAT:
			/* Skip 4 gbm_u8 IEEE floating point */
			gbm_file_lseek(fd, 4 * len, SEEK_CUR);
			break;
		case D_DOUBLE:
			/* Skip 8 gbm_u8 IEEE double precision floating point */
			gbm_file_lseek(fd, 8 * len, SEEK_CUR);
			break;
		default:
			gbm_file_read(fd, tag->value, (int) len);
			break;
		}

	if ( len > 4 )
		gbm_file_lseek(fd, old_pos, SEEK_SET);
	else if ( len < 4 )
		gbm_file_lseek(fd, 4L - len, SEEK_CUR);	/* advance past gap */

	return TE_OK;
	}
/*...e*/
/*...sread_ifd:0:*/
/*
For the time being we will assume there is only one IFD in
a given TIFF file. When this code was written, the author
knew of no software packages that support multiple IFDs.
*/

/*...sclean_up_ifd:0:*/
static void clean_up_ifd(IFD *ifd, int n)
	{
	int i;
	TAG *tag;
	for ( i = 0; i < n; i++ )
		{
		tag = &(ifd->tags[i]);
		if ( !(tag->type & 0x8000) )	/* its not read in */
			tag_free(tag);
		}
	ifd_free(ifd);
	}
/*...e*/

static int read_ifd(int fd, gbm_boolean motorola, IFD **ifd_return)
	{
	IFD *ifd;
	int i, ecode;

	if ( (ifd = ifd_malloc()) == NULL )
		return TE_MEM;

	/* ensure we can handle all the tags */

	if ( (ifd->n_tags = read_u16(fd, motorola)) > MAX_TAGS )
		{
		ifd_free(ifd); return TE_N_TAGS;
		}

	/* get the tags */

	for ( i = 0; i < ifd->n_tags; i++ )
		if ( (ecode = read_tag(fd, motorola, &(ifd->tags[i]))) != TE_OK )
			{
			clean_up_ifd(ifd, i);
			return ecode;
			}

	*ifd_return = ifd;

	return TE_OK;
	}
/*...e*/
/*...sskip_ifd:0:*/
/* Returns GBM_TRUE if there is another IFD afterwards */

static gbm_boolean skip_ifd(int fd, gbm_boolean motorola)
	{
	short n_tags = read_u16(fd, motorola);
	long offset_ifd;
	gbm_file_lseek(fd, 12L * n_tags, SEEK_CUR);
	offset_ifd = read_u32(fd, motorola);
	if ( offset_ifd == 0L )
		return GBM_FALSE;
	gbm_file_lseek(fd, offset_ifd, SEEK_SET);
	return GBM_TRUE;
	}
/*...e*/

int read_ifh_and_ifd(int fd, int n_ifds_to_skip, IFH **ifh_return)
	{
	IFH *ifh;
	long offset_ifd;
	gbm_boolean motorola;
	int ecode;

	if ( (ifh = ifh_malloc()) == NULL )
		return TE_MEM;

	gbm_file_read(fd, (char *) &(ifh->byte_order), sizeof(short));
	motorola = ( ifh->byte_order == ('M' << 8) + 'M' );

	/* Apparently, the following number has great univeral significance! */
	/* See the TIFF 5.0 spec. for details! */

	if ( (ifh->version_no = read_u16(fd, motorola)) != 42 )
		{
		ifh_free(ifh); return TE_VERSION;
		}

	offset_ifd = (long) read_u32(fd, motorola);
	gbm_file_lseek(fd, offset_ifd, SEEK_SET);
	while ( n_ifds_to_skip-- > 0 )
		if ( !skip_ifd(fd, motorola) )
			return TE_N_IFD;

	if ( (ecode = read_ifd(fd, motorola, &(ifh->ifd))) != TE_OK )
		{
		ifh_free(ifh); return ecode;
		}

	*ifh_return = ifh;

	return TE_OK;
	}
/*...e*/
/*...slocate_tag:0:*/
TAG *locate_tag(IFD *ifd, gbm_u16 type)
	{
	int i;
	for ( i = 0; i < ifd->n_tags; i++ )
		if ( ifd->tags[i].type == type )
			return &(ifd->tags[i]);
	return NULL;
	}
/*...e*/
/*...snumeric_tag:0:*/
gbm_boolean numeric_tag(TAG *tag)
	{
	gbm_u16 t = tag->data_type;
	return t == D_BYTE  ||
	       t == D_SHORT || t == D_SSHORT ||
	       t == D_LONG  || t == D_SLONG  ;
	}
/*...e*/
/*...svalue_of_tag_n:0:*/
/*
For a numeric tag, return the value of the nth item in it.
Upto the caller to know that tag is signed or unsigned.
Careful with sign/zero extending.
Careful to avoid assuming sizeof(long) = sizeof(gbm_s32).
*/

long value_of_tag_n(TAG *tag, int n)
	{
	switch ( tag->data_type )
		{
		case D_BYTE:
			{
			gbm_u8 *p = (gbm_u8 *) tag->value;
			return (long) (unsigned long) p[n];
			}
		case D_SBYTE:
			{
			gbm_s8 *p = (gbm_s8 *) tag->value;
			return (long) p[n];
			}
		case D_SHORT:
			{
			gbm_u16 *p = (gbm_u16 *) tag->value;
			return (long) (unsigned long) p[n];
			}
		case D_SSHORT:
			{
			gbm_s16 *p = (gbm_s16 *) tag->value;
			return (long) p[n];
			}
		case D_LONG:
			{
			gbm_u32 *p = (gbm_u32 *) tag->value;
			return (long) (unsigned long) p[n];
			}
		case D_SLONG:
			{
			gbm_s32 *p = (gbm_s32 *) tag->value;
			return (long) p[n];
			}
		}
	return 0L;
	}
/*...e*/
/*...svalue_of_tag:0:*/
/*
For a numeric tag, return the value of the 1st value in it.
This is usefull for tags that typically only have 1 value anyway.
*/

long value_of_tag(TAG *tag)
	{
	return value_of_tag_n(tag, 0);
	}
/*...e*/
/*...sfree_ifh:0:*/
void free_ifh(IFH *ifh)
	{
	IFD *ifd = ifh->ifd;
	clean_up_ifd(ifd, ifd->n_tags);
	ifh_free(ifh);
	}
/*...e*/
/*...smake_ifh:0:*/
/*
Creates an empty IFH set up for the image.
Also creates an IFD as part of the IFH.
Use add_?_tag() routines to add tags to IFH's IFD.
*/

IFH *make_ifh(void)
	{
	IFH *ifh;
	IFD *ifd;
	if ( (ifh = ifh_malloc()) == NULL )
		return NULL;
	if ( (ifh->ifd = ifd = ifd_malloc()) == NULL )
		{
		ifh_free(ifh);
		return NULL;
		}
	ifh->byte_order = (gbm_u16) ( ('I' << 8) + 'I' );
	ifh->version_no = (gbm_u16) 42;
	ifd->n_tags = 0;
	return ifh;
	}
/*...e*/
/*...sadd_byte_tag:0:*/
gbm_boolean add_byte_tag(IFD *ifd, gbm_u16 type, gbm_u8 *value, int n)
	{
	gbm_u8 *byte_ptr;
	TAG *tag;
	if ( (byte_ptr = (gbm_u8 *) malloc((size_t) (n * sizeof(gbm_u8)))) == NULL )
		return GBM_FALSE;
	if ( (tag = get_tag_slot(type, ifd)) == NULL )
		return GBM_FALSE;
	tag->type      = type;
	tag->data_type = D_BYTE;
	tag->length    = (gbm_u32) n;
	if ( value != NULL )
		memcpy(tag->value = (char *) byte_ptr,
		       value,
		       n * sizeof(gbm_u8));
	return GBM_TRUE;
	}
/*...e*/
/*...sadd_ascii_tag:0:*/
gbm_boolean add_ascii_tag(IFD *ifd, gbm_u16 type, char *value)
	{
	char *ascii_ptr;
	TAG *tag;
	int n;
	n = strlen(value) + 1;
	if ( (ascii_ptr = (char *) malloc((size_t) n)) == NULL )
		return GBM_FALSE;
	if ( (tag = get_tag_slot(type, ifd)) == NULL )
		return GBM_FALSE;
	tag->type      = type;
	tag->data_type = D_ASCII;
	tag->length    = (gbm_u32) n;
	strcpy(tag->value = ascii_ptr, value);
	return GBM_TRUE;
	}
/*...e*/
/*...sadd_short_tag:0:*/
gbm_boolean add_short_tag(IFD *ifd, gbm_u16 type, gbm_u16 *value, int n)
	{
	short *short_ptr;
	TAG *tag;
	if ( (short_ptr = (short *) malloc((size_t) (n * sizeof(short)))) == NULL )
		return GBM_FALSE;
	if ( (tag = get_tag_slot(type, ifd)) == NULL )
		return GBM_FALSE;
	tag->type      = type;
	tag->data_type = D_SHORT;
	tag->length    = (gbm_u32) n;
	if ( value != NULL )
		memcpy(tag->value = (char *) short_ptr,
		       value,
		       n * sizeof(gbm_u16));
	return GBM_TRUE;
	}
/*...e*/
/*...sadd_long_tag:0:*/
gbm_boolean add_long_tag(IFD *ifd, gbm_u16 type, gbm_u32 *value, int n)
	{
	gbm_u32 *long_ptr;
	TAG *tag;
	if ( (long_ptr = (gbm_u32 *) malloc((size_t) (n * sizeof(gbm_u32)))) == NULL )
		return GBM_FALSE;
	if ( (tag = get_tag_slot(type, ifd)) == NULL )
		return GBM_FALSE;
	tag->type      = type;
	tag->data_type = D_LONG;
	tag->length    = (gbm_u32) n;
	if ( value != NULL )
		memcpy(tag->value = (char *) long_ptr,
		       value,
		       n * sizeof(gbm_u32));
	return GBM_TRUE;
	}
/*...e*/
/*...sadd_rational_tag:0:*/
gbm_boolean add_rational_tag(IFD *ifd, gbm_u16 type, rational *value, int n)
	{
	rational *rational_ptr;
	TAG *tag;
	if ( (rational_ptr = (rational *) malloc((size_t) (n * sizeof(rational)))) == NULL )
		return GBM_FALSE;
	if ( (tag = get_tag_slot(type, ifd)) == NULL )
		return GBM_FALSE;
	tag->type      = type;
	tag->data_type = D_RATIONAL;
	tag->length    = (gbm_u32) n;
	if ( value != NULL )
		memcpy(tag->value = (char *) rational_ptr,
		       value,
		       n * sizeof(rational));
	return GBM_TRUE;
	}
/*...e*/
/*...swrite_ifh_and_ifd:0:*/
/*...spad:0:*/
static gbm_boolean pad(int fd, int n)
	{
	static char padding[] = { 0, 0, 0, 0 };
	return gbm_file_write(fd, padding, n) == n;
	}
/*...e*/
/*...swrite_u16:0:*/
static gbm_boolean write_u16(int fd, gbm_u16 w)
	{
	gbm_u8 b[2];
	b[0] = (gbm_u8)                   (w & 0x00ffU)      ;
	b[1] = (gbm_u8) ((unsigned short) (w & 0xff00U) >> 8);
	return gbm_file_write(fd, b, 2) == 2;
	}
/*...e*/
/*...swrite_u32:0:*/
static gbm_boolean write_u32(int fd, gbm_u32 d)
	{
	gbm_u8 b[4];
	b[0] = (gbm_u8)  (d & 0x000000ffUL);
	b[1] = (gbm_u8) ((d & 0x0000ff00UL) >>  8);
	b[2] = (gbm_u8) ((d & 0x00ff0000UL) >> 16);
	b[3] = (gbm_u8) ((d & 0xff000000UL) >> 24);
	return gbm_file_write(fd, b, 4) == 4;
	}
/*...e*/
/*...swrite_rational:0:*/
static gbm_boolean write_rational(int fd, const rational *rational)
	{
	return write_u32(fd, rational->numerator  ) &&
	       write_u32(fd, rational->denominator) ;
	}
/*...e*/
/*...swrite_tag:0:*/
static gbm_boolean write_tag(int fd, TAG *tag, long *offset_upto)
	{
	gbm_boolean ok;
	int s, i, n, len;
	long offset_return_to;

	ok = write_u16(fd, tag->type) &&
	     write_u16(fd, tag->data_type) &&
	     write_u32(fd, tag->length);

	if ( !ok )
		return GBM_FALSE;

	/* if we can fit the tag into 4 gbm_u8s, do so */
	/* else we will have to allocate some disc space */

	s = sizeof_data_type(tag->data_type);
	n = (int) tag->length;
	len = s * n;

	if ( len > 4 )
		{
		if ( !write_u32(fd, *offset_upto) )
			return GBM_FALSE;
		offset_return_to = gbm_file_lseek(fd, 0L, SEEK_CUR);
		gbm_file_lseek(fd, *offset_upto, SEEK_SET);
		}		

	/* actually write the tag */

	switch ( tag->data_type )
		{
		case D_BYTE:
		case D_ASCII:
			if  ( gbm_file_write(fd, tag->value, n) != n )
				return GBM_FALSE;
			break;
		case D_SHORT:
			{
			const gbm_u16 *gbm_u16_ptr = (const gbm_u16 *) tag->value;
			for ( i = 0; i < n; i++ )
				if ( !write_u16(fd, *gbm_u16_ptr++) )
					return GBM_FALSE;
			}
			break;
		case D_LONG:
			{
			const gbm_u32 *gbm_u32_ptr = (const gbm_u32 *) tag->value;
			for ( i = 0; i < n; i++ )
				if ( !write_u32(fd, *gbm_u32_ptr++) )
					return GBM_FALSE;
			}
			break;
		case D_RATIONAL:
			{
			const rational *rational_ptr = (const rational *) tag->value;
			for ( i = 0; i < n; i++ )
				if ( !write_rational(fd, rational_ptr++) )
					return GBM_FALSE;
			}
			break;
		}

	if ( len > 4 )
		{
		if ( (*offset_upto = gbm_file_lseek(fd, 0L, SEEK_CUR)) & 1L )
			/* pad to make next offset even */
			{
			if ( !pad(fd, 1) )
				return GBM_FALSE;
			(*offset_upto)++;
			}
		gbm_file_lseek(fd, offset_return_to, SEEK_SET);
		}
	else if ( len < 4 )
		if ( !pad(fd, 4 - len) )
			return GBM_FALSE;
	return GBM_TRUE;
	}
/*...e*/
/*...swrite_ifd:0:*/
/*
Given an IFD, write it out to disc.
Also patch the IFH (which we know will be at the start of the file).
In writing out a tag we may need some more disc space other than
that for the IFD table. This occurs when a field is larger than
4 gbm_u8s. What we do is to keep a pointer to the next free space
(after the table) and write_tag() will advance it if it uses any
extra space.
*/

gbm_boolean	write_ifd(int fd, IFD *ifd)
	{
	int i, n;
	long offset_upto;

	if ( !write_u16(fd, (short) (n = ifd->n_tags)) )
		return GBM_FALSE;

	/* write out tags */

	offset_upto = gbm_file_lseek(fd, 0L, SEEK_CUR) + n * 12L + 4L;
		/* leave space for each tag plus next IFD ptr */

	for ( i = 0; i < n; i++ )
		if ( !write_tag(fd, &(ifd->tags[i]), &offset_upto) )
			return GBM_FALSE;

	/* done writing out the IFD, now put null next IFD pointer */

	if ( !write_u32(fd, 0L) )
		return GBM_FALSE;
	
	gbm_file_lseek(fd, offset_upto, SEEK_SET);

	return GBM_TRUE;
	}
/*...e*/

gbm_boolean write_ifh_and_ifd(IFH *ifh, int fd)
	{
	return write_u16(fd, ifh->byte_order) &&
	       write_u16(fd, ifh->version_no) &&
	       write_u32(fd, 8L) &&
	       write_ifd(fd, ifh->ifd);
	}
/*...e*/
/*...supdate_byte_tag:0:*/
void update_byte_tag(IFD *ifd, gbm_u16 type, const gbm_u8 *value)
	{
	TAG *tag = locate_tag(ifd, type);
	int n = (int) tag->length;
	memcpy(tag->value, value, n * sizeof(gbm_u8));
	}
/*...e*/
/*...supdate_ascii_tag:0:*/
void update_ascii_tag(IFD *ifd, gbm_u16 type, const char *value)
	{
	TAG *tag = locate_tag(ifd, type);
	int n = (int) tag->length;
	memcpy(tag->value, value, n);
	}
/*...e*/
/*...supdate_short_tag:0:*/
void update_short_tag(IFD *ifd, gbm_u16 type, const gbm_u16 *value)
	{
	TAG *tag = locate_tag(ifd, type);
	int n = (int) tag->length;
	memcpy(tag->value, value, n * sizeof(gbm_u16));
	}
/*...e*/
/*...supdate_long_tag:0:*/
void update_long_tag(IFD *ifd, gbm_u16 type, const gbm_u32 *value)
	{
	TAG *tag = locate_tag(ifd, type);
	int n = (int) tag->length;
	memcpy(tag->value, value, n * sizeof(gbm_u32));
	}
/*...e*/
/*...supdate_rational_tag:0:*/
void update_rational_tag(IFD *ifd, gbm_u16 type, const rational *value)
	{
	TAG *tag = locate_tag(ifd, type);
	int n = (int) tag->length;
	memcpy(tag->value, value, n * sizeof(rational));
	}
/*...e*/
/*...supdate_ifd:0:*/
/*
Go back to the IFD, and rewrite it.
*/

gbm_boolean update_ifd(IFD *ifd, int fd)
	{
	gbm_file_lseek(fd, 8L, SEEK_SET);
	return write_ifd(fd, ifd);
	}
/*...e*/
