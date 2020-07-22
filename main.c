#include <stdio.h>
#include <stdlib.h>
#include <stdfix.h>
#include <string.h>
#include <math.h>
#include "tiff.h"
#include "image_alg.h"

int main(void)
{
    unsigned char *PATH = "C:/Users/Administrator/Desktop/mamiao/Alg_Cpp/image/A.tiff";
    unsigned char *PATH_1 = "C:/Users/Administrator/Desktop/mamiao/Alg_Cpp/image/A1.bin";
    unsigned char *PATH_2 = "C:/Users/Administrator/Desktop/mamiao/Alg_Cpp/image/A1.tiff";
    unsigned char *Img=(unsigned char *)malloc(sizeof(unsigned char)*3000000);
    Tiff_read(Img,PATH);

    FILE *fd;
    int i,j,k;
    fd = fopen(PATH_1,"wb+");
    fseek(fd,0,SEEK_SET); // Insert the image data
    for(i=0;i<1000;i++){
        for(j=0;j<1000;j++){
            for(k=0;k<3;k++){
                fwrite(Img+i*3000+j*3+k,sizeof(unsigned char),1,fd);
            }
        }
    }
    Tiff_write(Img,1000,1000,3,PATH_2);
    printf("Finished!\n");
    while(1);
    return 0;
}