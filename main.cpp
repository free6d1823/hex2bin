#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "gdc_config_seq_planar_rgb444.h"

static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tConvert 3 hex files to rgb file\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s -r f1 [ -g f2 -b f3] [-o output-file]\n\n", name);
    printf("\t%s -i f1  -j f2  [-o output-file]\n\n", name);
 
    printf("DESCRIPTION\n");
    printf("\t\t -r f1, f2, f3 hex file, combine R16,G16,B16 data to RGB48 binary file\n");
    printf("\t\t -i f1 -j f2   interleave f1 f2 (16 bit) hex to bin file\n");
    printf("\n\n");
}

int char2i(char data)
{
    if(data >= '0' && data <= '9')
        return data - '0';
    if(data >= 'a' && data <= 'f')
        return data - 'a' + 10;
    if (data >= 'A' && data <= 'F')
        return data - 'A' +10;
    return 0;
}
#define BIT_SHIFT 6 //14-bit to 8-bit
#define CHAR_TO_INT(a,b,c,d) ( (a<<12) + (b<<8) + (c<<4) + d)
bool LineToChars(char* line, unsigned char* data)
{
    char* p = strchr(line, ' ');
    if(!p) p = line;
    else p++;
    if(strlen(p) <8)
        return false;
    int value[2];
    for (int i=0; i<2; i++){
        value[i] = CHAR_TO_INT( char2i(p[0]), char2i(p[1]), char2i(p[2]), char2i(p[3]));
        data[i] = (unsigned char)((value[i] >> BIT_SHIFT)  &0xff);
        p+=4;

    }
    return true;
}

//[ADDRESS] 00ab00cd
int LineToU16(char* line, unsigned short* data)
{
    char* p = strchr(line, ' ');
    if(!p) p = line;
    else p++;
    
    int words = strlen(p)/4;
    if(words > 2)
    	words = 2; //takes max 2 words
    else if (words <= 0) 
        return 0;
    unsigned int value[2];
    for (int i=0; i<words; i++){
        value[i] = CHAR_TO_INT( char2i(p[0]), char2i(p[1]), char2i(p[2]), char2i(p[3]));
        data[i] = value[i];
        p+=4;

    }
    return words;
}
int main(int argc, char* argv[])
{
    int ch;
    char* outputfile = NULL;
    char* inputfile[3] ={ NULL, NULL, NULL};
    FILE* fpin = NULL;
    FILE* fpout = NULL;
    FILE* fptmp[3] = {NULL, NULL, NULL};
    const char* tempout[3] = {"r1.y", "r2.y", "r3.y"};
    int comp = 0;
    int i;
    char line[256];
    //unsigned char data[2];
    unsigned short data[2];
    unsigned short pixel;
    enum MODE{
	MODE_GRAY, //one file 8 byte
	MODE_RGB,
	MODE_INTERLEAVE16
    };
    MODE mode = MODE_GRAY;
    while ((ch = getopt(argc, argv, "i:j:r:g:b:o:h?")) != -1)
    {
        switch (ch) {
        case 'o':
            outputfile = optarg;
            break;
        case 'r':
            inputfile[0] = optarg;
	    mode = MODE_GRAY;
            break;
        case 'g':
            inputfile[1] = optarg;
	    mode = MODE_RGB;
            break;
        case 'b':
            inputfile[2] = optarg;
            break;
	case 'i':
	    inputfile[0] = optarg;
	    mode = MODE_INTERLEAVE16;
	    break;
	case 'j':
	    inputfile[1] = optarg;
	    break;
        case 'h':
        case '?':
        default:
            usage(argv[0]);
            exit(-1);
        }   
    }
//h to bin	
	if(outputfile != NULL) {
	int length = sizeof(planar_rgb444_1920x1080_seq);
		fpout = fopen(outputfile, "wb");
	printf("write %s, %p %d bytes\n", outputfile, planar_rgb444_1920x1080_seq, 	length);
	
	unsigned char* pBuffer = (unsigned char* )malloc(length);
	memcpy(pBuffer, planar_rgb444_1920x1080_seq, length);
	printf("write buffer %p\n", pBuffer);
		fwrite(pBuffer, 1, length, fpout);
		fclose(fpout);
    printf("start: %x %x %x %x, last 4: %x,%x,%x,%x\n", 
    pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], 
    pBuffer[length-4], pBuffer[length-3], pBuffer[length-2], pBuffer[length-1]);
		exit(0);
	}

    if ( inputfile[0] == NULL || outputfile == NULL) {
        usage(argv[0]);
        exit(-1);
    }
    //sanity check
    switch (mode) {
	    case MODE_GRAY:
	    	comp = 1;
		break;
	case MODE_RGB:
		if (inputfile[1] != NULL && inputfile[2] !=NULL)
			comp = 3;
		break;
	case MODE_INTERLEAVE16:
		if (inputfile[1] != NULL)
			comp = 2;
		break;
	default:
		break;
    }
    if (comp <= 0) {
        usage(argv[0]);
        exit(-1);
    }
    for (i =0; i < comp; i ++) {
        fpin = fopen(inputfile[i], "rb");
        if (!fpin) {
            fprintf(stderr, "Error to open file %s\n", inputfile[i]);
            continue;
        }
        fpout = fopen(tempout[i], "wb");
        if (! fpout) {
            fprintf(stderr, "Error to open temp file %s\n", tempout[i]);
            fclose(fpin);
            continue;
        }        
        while ( fgets(line,sizeof(line), fpin) >0){
        	int nLen = LineToU16(line, data); //return numbers of U16
            if(nLen > 0){
                fwrite(data, nLen, sizeof(unsigned short), fpout);
            	printf("R: %s %x\n", line, data[0]);
            }

        }
        fclose(fpin);
        fclose(fpout);

    }
    for(i = 0;  i< comp; i++)
    {
        fptmp[i] = fopen(tempout[i], "rb");
    }
    fpout = fopen(outputfile, "wb");
    //merge files
    bool go = true;
	int len = 0;
    while(go) {
		for(i = 0; i< comp; i++)
		{
			//read until anyone in file is EOF
			if(sizeof(pixel) != fread(&pixel, 1, sizeof(pixel), fptmp[i])){
			go = false;
			break;
			}

			fwrite(&pixel, 1, sizeof(pixel), fpout);
			len += sizeof(pixel);
		}
    }

    for(i = 0; i<comp; i++) {
        if(fptmp[i]) {
            fclose(fptmp[i]);
        }
        //unlink(tempout[i]);
    }
    if(fpout) 
    	fclose(fpout);
    printf("Output file %s, %d bytes.\n", outputfile, len);
	return 0;
}
