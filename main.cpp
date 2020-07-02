#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tConvert 3 hex files to rgb file\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s -r f1 [ -g f2 -b f3] [-o output-file]\n\n", name);
    printf("DESCRIPTION\n");
    printf("\t\t f1, f2, f3   hex file\n");
    printf("\t\t output-file  binary file\n");
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

bool LineToU16(char* line, unsigned short* data)
{
    char* p = strchr(line, ' ');
    if(!p) p = line;
    else p++;
    if(strlen(p) <8)
        return false;
    unsigned int value[2];
    for (int i=0; i<2; i++){
        value[i] = CHAR_TO_INT( char2i(p[0]), char2i(p[1]), char2i(p[2]), char2i(p[3]));
        data[i] = value[i];
        p+=4;

    }
    return true;
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
    int comp;
    unsigned char cflag = 0;
    char line[256];
    //unsigned char data[2];
    unsigned short data[2];
    unsigned short pixel;
    while ((ch = getopt(argc, argv, "r:g:b:o:h?")) != -1)
    {
        switch (ch) {
        case 'o':
            outputfile = optarg;
            break;
        case 'r':
            inputfile[0] = optarg;
            break;
        case 'g':
            inputfile[1] = optarg;
            break;
        case 'b':
            inputfile[2] = optarg;
            break;
        case 'h':
        case '?':
        default:
            usage(argv[0]);
            exit(-1);
        }   
    }


    if ( inputfile[0] == NULL || outputfile == NULL) {
        usage(argv[0]);
        exit(-1);
    }
    for (comp =0; comp < 3; comp ++) {
        if(!inputfile[comp] )
            continue;
        fpin = fopen(inputfile[comp], "rb");
        if (!fpin) {
            fprintf(stderr, "Error to open file %s\n", inputfile[comp]);
            continue;
        }
        fpout = fopen(tempout[comp], "wb");
        if (! fpout) {
            fprintf(stderr, "Error to open temp file %s\n", tempout[comp]);
            fclose(fpin);
            continue;
        }        
        cflag |= (1<<comp);
        while ( fgets(line,sizeof(line), fpin) >0){
            if(LineToU16(line, data))
            //if(LineToChars(line, data))
                fwrite(data, 1, sizeof(data), fpout);
        }
        fclose(fpin);
        fclose(fpout);

    }
    if (cflag == 0) {
        fprintf(stderr, "failed to convert file!!\n");
    }
    for(comp = 0; cflag&(1<<comp); comp++)
    {
        fptmp[comp] = fopen(tempout[comp], "rb");
    }
    fpout = fopen(outputfile, "wb");
    //merge files
    bool go = true;

    while(go) {
        for(comp = 0; fptmp[comp]; comp++)
        {
            if(sizeof(pixel) != fread(&pixel, 1, sizeof(pixel), fptmp[comp])){
                go = false;
                break;
            }

            fwrite(&pixel, 1, sizeof(pixel), fpout);
        }
    }

    for(comp = 0; comp<3; comp++) {
        if(fptmp[comp]) {
            fclose(fptmp[comp]);
        }
        unlink(tempout[comp]);
    }
    if(fpout) fclose(fpout);
	return 0;
}
