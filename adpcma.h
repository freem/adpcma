/* adpcma.h - header file for adpcm-a sample encoder */
#ifndef _ADPCMA_H_
#define _ADPCMA_H_

#define RELEASE_NUM 3

#define BUFFERSIZE 1024
static const char *greets = "the author wishes to send greetz (in no particular order) to:\nBlack_Jack, Fred/FRONT, kuk, l_oliviera, HPMAN, furrtek, ElBarto, Jeff Kurtz,\nRazoola, tcdev, smkdan, and the MAME development team.\n\nenjoy!\n";

/*** tables ***/

/* usual ADPCM table (16 * 1.1^N)
 * (and by "usual" I guess they mean the shortened OKI ADPCM table )*/
static int step_size[49] = {
	   16,  17,   19,   21,   23,   25,   28,
	   31,  34,   37,   41,   45,   50,   55,
	   60,  66,   73,   80,   88,   97,  107,
	  118, 130,  143,  157,  173,  190,  209,
	  230, 253,  279,  307,  337,  371,  408,
	  449, 494,  544,  598,  658,  724,  796,
	  876, 963, 1060, 1166, 1282, 1411, 1552
};

/* different from the usual ADPCM table */
static int step_adj[16] = {
	-1, -1, -1, -1, 2, 5, 7, 9,
	-1, -1, -1, -1, 2, 5, 7, 9
};

static int jedi_table[49*16];

/*** global variables ***/

/* decoding */
static int		adpcm_accum		= 0;	/* ADPCM accumulator; initial condition must be 0 */
static int		adpcm_decstep	= 0;	/* ADPCM decoding step; initial condition must be 0 */

/* encoding */
static short	diff;
static short	step;
static int		predictsample;
static int		index;					/* Index into step_size table */
static int		prevsample		= 0;	/* previous sample; initial condition must be 0 */
static int		previndex		= 0;	/* previous index; initial condition must be 0 */

/* function prototypes */
static void Init_ADPCMATable(void);
static short ADPCMA_Decode(unsigned char code);
void usage(void);

#endif
