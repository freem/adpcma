/*
 * adpcma
 * a command line ADPCM-A sample encoder for Neo-Geo games.
 *
 * References:
 * MAME source code (emu/sound/fm.c)
 * MVSTracker source code (Ivan Mackintosh, et al)
 * HPMan or whomever wrote the C# code on https://wiki.neogeodev.org/index.php?title=ADPCM_codecs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "adpcma.h"

/* PCM, Signed 16 bit Little Endian, 18500Hz Mono 16bits/sample */

/* initialize the jedi_table */
static void Init_ADPCMATable(void){
	int step, nib;

	for(step=0; step<49; step++){
		/* loop over all nibbles and compute the difference */
		for(nib=0; nib<16; nib++){
			int value = (2*(nib & 0x07) + 1) * step_size[step]/8;
			jedi_table[step*16 + nib] = (nib&0x08) ? -value : value;
		}
	}
}

/* decode ADPCM-A value */
static short ADPCMA_Decode(unsigned char code){
	adpcm_accum += jedi_table[adpcm_decstep+code];

	/* extend 12-bit signed int */
	if(adpcm_accum & ~0x7FF){		/* accum > 2047 */
		adpcm_accum |= ~0xFFF;
	}
	else{
		adpcm_accum &= 0xFFF;
	}

	adpcm_decstep += step_adj[code & 7] * 16;

	/* perform limit and return */
	if(adpcm_decstep < 0){
		adpcm_decstep = 0;
	}
	if(adpcm_decstep > (48*16)){
		adpcm_decstep = 48*16;
	}

	return (short)adpcm_accum;
}

/* encode value for ADPCM-A */
static unsigned char ADPCMA_Encode(short sample){
	short tempstep;
	unsigned char code;

	predictsample = prevsample;
	index = previndex;
	step = step_size[index];

	diff = sample - predictsample;

	if(diff >= 0){
		code = 0;
	}
	else{
		code = 8;
		diff = -diff;
	}

	tempstep = step;
	if(diff >= tempstep){
		code |= 4;
		diff -= tempstep;
	}

	tempstep >>= 1;
	if(diff >= tempstep){
		code |= 2;
		diff -= tempstep;
	}

	tempstep >>= 1;
	if(diff >= tempstep){ code |= 1; }

	predictsample = ADPCMA_Decode(code);

	index += step_adj[code];

	if(index < 0){ index = 0; }
	if(index > 48){ index = 48; }

	prevsample = predictsample;
	previndex = index;

	return code;
}

/* print usage */
void usage(void){
	printf("adpcma - A command line YM2610 ADPCM-A sample encoder. (release %d)\n",RELEASE_NUM);
	puts("usage: adpcma (infile) [outfile]");
	puts("");
	puts("Input files must be 18500Hz 16bit mono Little Endian PCM.");
	puts("[outfile] is optional and defaults to 'out.pcma' if omitted.");
	puts("");
	puts("Please see the readme for more information.");
	puts("If the readme did not come with the program, hit the person you got it from.");
}

int main(int argc, char *argv[]){
	FILE *inFile;
	FILE *outFile;
	char *outFilename;

	int i;
	unsigned long totalWritten = 0;
	unsigned short padBytes = 0;
	size_t numBytes = 0;

	/* Variables used for .wav-related tasks */
	unsigned short wavFile = 0;
	char tempBuf[4];
	unsigned short wavFormatType;
	unsigned short wavChannels;
	unsigned int wavSampleRate;
	unsigned short wavBits;

	/* check for any arguments at all */
	if(argc<2){
		usage();
		exit(EXIT_SUCCESS);
	}
	if(argc>3){
		puts("too many arguments!");
		usage();
		exit(EXIT_FAILURE);
	}
	printf("adpcma - A command line YM2610 ADPCM-A sample encoder. (release %d)\n",RELEASE_NUM);
	puts("------------------------------------------------------------------");

	/* set output filename */
	if(argc==3){	outFilename = argv[2]; }
	else{			outFilename = "out.pcma"; }

	if(strcmp(argv[1],"%")==0 && strcmp(argv[2],"%")==0){
		printf("%s",greets);
		exit(EXIT_SUCCESS);
	}

	/* open input file */
	inFile = fopen(argv[1],"rb");
	if(inFile==NULL){
		printf("Error attempting to open file '%s': ",argv[1]);
		perror("");
		exit(EXIT_FAILURE);
	}
	printf("Successfully opened '%s'.\n",argv[1]);

	/* Determine if this is a .wav file by trying to find the strings "RIFF" and
	 * "WAVE" in the file header. */

	/*
	 * $00-$03	"RIFF"
	 * $04-$07	(File size-8) in bytes
	 * $08-$0B	"WAVE"
	 */
	fread(tempBuf,sizeof(char),4,inFile);
	if(strcmp(tempBuf,"RIFF") == 0){
		fread(tempBuf,sizeof(char),4,inFile);	/* dummy read for file size */
		fread(tempBuf,sizeof(char),4,inFile);
		if(strcmp(tempBuf,"WAVE") == 0){
			/* if both "RIFF" and "WAVE" are found, set wavFile = 1 */
			wavFile = 1;
		}
	}

	/* If we have determined that this is a .wav file, we can do some extra work
	 * to determine if our input is correct. */
	if(wavFile == 1){
		puts("Processing .wav file header...");
		/*
		 * $0C-$0F	"fmt "
		 * $10-$13	Format data length
		 * $14-$15	Format type (PCM is 1, we don't want anything else)
		 * $16-$17	Number of channels (we want 1; 2 makes for slower playback [warn user])
		 * $18-$1B	Sample Rate (we want 18500/$4844, or $4448 in-file)
		 * $1C-$1F	(Sample Rate * Bits per Sample * Channels) / 8
		 * $20-$21	(Bits per Sample * Channels) / 8 (1:8bit mono, 2:8bit stereo/16bit mono, 4:16bit stereo)
		 * $22-$23	Bits per Sample
		 * $24-$27	"data"
		 * $28-$2B	Size of data section
		 */

		fread(tempBuf,sizeof(char),4,inFile);	/* dummy read for "fmt " */
		fread(tempBuf,sizeof(char),4,inFile);	/* dummy read for format data length */

		/* read wav format type */
		fread(&wavFormatType,sizeof(unsigned short),1,inFile);
		if(wavFormatType != 1){
			printf("Unsupported wav format type ID %d; should be 1 (PCM).\n",wavFormatType);
			exit(EXIT_FAILURE);
		}

		/* read number of channels */
		fread(&wavChannels,sizeof(unsigned short),1,inFile);
		if(wavChannels != 1){
			printf("Found %d wav channels; should be 1.\n",wavChannels);
			exit(EXIT_FAILURE);
		}

		/* read sample rate */
		fread(&wavSampleRate,sizeof(unsigned int),1,inFile);
		if(wavSampleRate != 18500){
			printf("File has a sample rate of %d; should be 18500.\n",wavSampleRate);
			exit(EXIT_FAILURE);
		}

		fread(tempBuf,sizeof(char),4,inFile); /* dummy read 1 */
		fread(tempBuf,sizeof(char),2,inFile); /* dummy read 2 */

		/* read bits; we want 16 */
		fread(&wavBits,sizeof(unsigned short),1,inFile);
		if(wavBits != 16){
			printf("File bits are %d; should be 16.\n",wavBits);
			exit(EXIT_FAILURE);
		}

		fread(tempBuf,sizeof(char),4,inFile);	/* dummy read for "data" */
		fread(tempBuf,sizeof(char),4,inFile);	/* dummy read for data length */
		/* you have arrived at your destination. */
	}
	else{
		/* rewind input file */
		rewind(inFile);
	}

	/* program initialization */
	adpcm_accum = 0;
	adpcm_decstep = 0;
	prevsample = 0;
	previndex = 0;

	/* init jedi table */
	Init_ADPCMATable();

	diff = 0;
	step = 0;
	predictsample = 0;
	index = 0;

	/* prepare output file */
	outFile = fopen(outFilename,"wb");
	if(outFile==NULL){
		printf("Error attempting to create output file '%s': ",outFilename);
		perror("");
		exit(EXIT_FAILURE);
	}

	/* perform the dirty work, emphasis on "dirty" */
	printf("Encoding file...\n");
	short buffer12b[BUFFERSIZE/2];
	while(1){
		numBytes = fread(buffer12b,1,BUFFERSIZE,inFile);
		if(numBytes <= 0){ break; }

		/* convert data to 12 bits; similar to MVSTracker code
		 * "[numBytes]/2 as we are working in 16bit WORDs"
		 */
		for(i = 0; i < numBytes/2; i++){
#ifdef _VERBOSE_LOGGING
			printf("[%i] ",i);
#endif

			short *pData = &buffer12b[i];
#ifdef _VERBOSE_LOGGING
			printf("before: %04X\n",*pData);
#endif
			/* downscale to 12 bits */
			*pData >>= 4;
#ifdef _VERBOSE_LOGGING
			printf("after: %04X\n",*pData);
#endif
		}

		/* Generate ADPCM */
		for (i = 0; i < numBytes/2; i+=2){
			unsigned char Val;
#ifdef _VERBOSE_LOGGING
			printf("buffer vals: %04X, %04X\n",buffer12b[i],buffer12b[i+1]);
#endif
			Val = (ADPCMA_Encode(buffer12b[i]) << 4) | ADPCMA_Encode(buffer12b[i+1]);
#ifdef _VERBOSE_LOGGING
			printf("encoded value: %02X\n",Val);
#endif
			putc(Val,outFile);
			++totalWritten;
		}
#ifdef _VERBOSE_LOGGING
		puts("------------------------------------------------------------------");
#endif
	}
	puts("");
	fclose(inFile);
	printf("%lu bytes written to '%s'.\n",totalWritten,outFilename);

	/* perform padding with 0x80 if needed */
	if(totalWritten%256!=0){
		padBytes = 256-totalWritten%256;
		printf("Padding file with %u bytes to reach 256 byte boundary (%lu bytes)...\n",padBytes,totalWritten+(padBytes));
		for(i=0;i<padBytes;i++){
			fputc(0x80,outFile);
		}
	}
	puts("Encoding and padding process completed successfully.");

	fclose(outFile);
	return EXIT_SUCCESS;
}
