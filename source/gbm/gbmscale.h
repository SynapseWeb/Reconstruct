/*

gbmscale.h - Interface to scaling code

*/

#ifndef GBMSCALE_H
#define	GBMSCALE_H

extern GBM_ERR gbm_simple_scale(
	const gbm_u8 *s, int sw, int sh,
	      gbm_u8 *d, int dw, int dh,
	int bpp
	);

#endif
