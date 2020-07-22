#include "tiff.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

char mashine_endian(void)
{
    int a = 1;
    int *p = &a;
    if(*((char *)p) == a){
        return 1; // little endian
    }else{
        return 0; // big endian
    }
}

char Tiff_read(unsigned char *Image,unsigned char *Path)
{
    FILE *fd;
    unsigned char tmp[2],buff,dev_endian = mashine_endian(),file_endian,TypeSize,Image_Start_TypeSize;
    unsigned int DE_n=0,i=0,DE_val_size=0,channel=0,wid,hei,sum,row,clo,dep;
    unsigned long IFD_Start=0,Image_Start;
    size_t count;
    IFH _IFH;
    IFD _IFD;

    fd = fopen(Path,"rb");
    count = fread(&(_IFH.endian),short_size,1,fd);
    if(count != 1) { return -1;}
    if(_IFH.endian == 0x4d4d){
        file_endian = 0;
    }else{
        file_endian = 1;
    }
    count = fread(&(_IFH.magic),short_size,1,fd);
    if(count != 1) { return -1;}
    count = fread(&(_IFH.ifd_offset),ulong_size ,1,fd);
    if(count != 1) { return -1;}
    if(file_endian != dev_endian) {
        _IFH.endian = MC_GET_SHORT(_IFH.endian);
        _IFH.magic = MC_GET_SHORT(_IFH.magic);
        _IFH.ifd_offset = MC_GET_LONG(_IFH.ifd_offset);
    }

    IFD_Start = _IFH.ifd_offset;

    printf("Endian,Magic:%x-%x\n",_IFH.endian,_IFH.magic);
    printf("IFD_Start Offset:%d\n",IFD_Start);

    fseek(fd,IFD_Start,SEEK_SET);
    count = fread(&(_IFD.n),short_size,1,fd);
    if(count != 1) { return -1;}
    if(file_endian != dev_endian) {_IFD.n = MC_GET_SHORT(_IFD.n);}
    DE_n = _IFD.n;
    printf("Number Of DE:%d\n",DE_n);
    _IFD.p = (DE *)malloc(sizeof(DE)*DE_n);
    for(i=0;i<DE_n;i++){
        count = fread(&((_IFD.p+i)->tag),short_size,1,fd);
        if(count != 1) { return -1;}
        count = fread(&((_IFD.p+i)->type),short_size,1,fd);
        if(count != 1) { return -1;}
        count = fread(&((_IFD.p+i)->size),ulong_size ,1,fd);
        if(count != 1) { return -1;}
        count = fread((&(_IFD.p+i)->val_offset),ulong_size ,1,fd);
        if(count != 1) { return -1;}
        if(file_endian != dev_endian) {
            // printf("Ori_Val[Tag-Type-Size]:[%x-%x-%x]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size);
            (_IFD.p+i)->tag = MC_GET_SHORT((_IFD.p+i)->tag);
            (_IFD.p+i)->type = MC_GET_SHORT((_IFD.p+i)->type);
            (_IFD.p+i)->size = MC_GET_LONG((_IFD.p+i)->size);
            // printf("Res_Val[Tag-Type-Size]:[%x-%x-%x]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size);
            switch((_IFD.p+i)->type){
                case 1: TypeSize = 1;
                case 2: DE_val_size = (_IFD.p+i)->size; break;
                case 3: TypeSize = 2; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                case 4: TypeSize = 4; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                case 5: TypeSize = 8; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                default: TypeSize = 1; break;
            }
            if(DE_val_size == 1){
                (_IFD.p+i)->val_offset = MC_GET_CHAR((_IFD.p+i)->val_offset);
            }else if(DE_val_size == 2){
                (_IFD.p+i)->val_offset = MC_GET_SHORT((_IFD.p+i)->val_offset);
            }else if(DE_val_size >= 4){
                (_IFD.p+i)->val_offset = MC_GET_LONG((_IFD.p+i)->val_offset);
            }
        }else{
            switch((_IFD.p+i)->type){
                case 1: TypeSize = 1;
                case 2: DE_val_size = (_IFD.p+i)->size; break;
                case 3: TypeSize = 2; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                case 4: TypeSize = 4; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                case 5: TypeSize = 8; DE_val_size = (_IFD.p+i)->size * TypeSize; break;
                default: TypeSize = 1; break;
            }
        }
        // printf("Tag Value:%d\n",(_IFD.p+i)->tag);
        if(256 == (_IFD.p+i)->tag){ // 图像宽度存储位置
            wid = (_IFD.p+i)->val_offset;
            printf("DE-Width of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(257 == (_IFD.p+i)->tag){ // 图像高度存储位置
            hei = (_IFD.p+i)->val_offset;
            printf("DE-Height of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(258 == (_IFD.p+i)->tag){ // 图像每个通道灰度等级 eg:RGB[8,8,8]
            printf("DE-BitsPerSample of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(259 == (_IFD.p+i)->tag){ // 图像采用的压缩算法 1-NoCompression 2-CCITT ...
            printf("DE-Compression of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(262 == (_IFD.p+i)->tag){ // 表示图像的种类: 0-WhiteisZero 1-BlackisZero 2-RGBImg 3-PaletteColor ...
            if((_IFD.p+i)->val_offset == 2){
                channel = 3;
            }else{
                channel = 1;
            }
            printf("DE-PhotometricInyerpretation of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(273 == (_IFD.p+i)->tag){ // 图像数据起始地址存储位置相对于文件开始的位置val的保存位置
            Image_Start = (_IFD.p+i)->val_offset;
            Image_Start_TypeSize = TypeSize;
            printf("DE-StripOffsets of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(274 == (_IFD.p+i)->tag){ // 图像坐标系方式: 1[left top] 2[right top] 3[bottom right] ...
            printf("DE-Orientation of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(277 == (_IFD.p+i)->tag){ // 表示图像的格式为3[RGB]或1[bilevel,grayscale,palette-color] image
            printf("DE-SamplePerPixel of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(278 == (_IFD.p+i)->tag){ // 表示每一个Strip内包含的图像的行数 eg:Img[w][h][w] --> RowsPerStrip=1
            printf("DE-RowsPerStrip of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(279 == (_IFD.p+i)->tag){ // 表示每一个Strip的Bytes大小 eg:Img[w][h][d] --> StripByteCounts=w*d
            printf("DE-StripByteCounts of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
        if(284 == (_IFD.p+i)->tag){ // 全彩图像每一个像素点排列方式: 1[RGB...RGB]-交叉 2[[R...R],[G...G],[B...B]]-平铺
            printf("DE-PlanarConfiguration of Img-[Tag:%d Type:%d Size:%d Offset:%d]\n",(_IFD.p+i)->tag,(_IFD.p+i)->type,(_IFD.p+i)->size,(_IFD.p+i)->val_offset);
            continue;
        }
    }

    fseek(fd,Image_Start,SEEK_SET);
    fread(&Image_Start,Image_Start_TypeSize,1,fd);  // Image_Start:图像数据起始地址存储位置相对于文件开始的位置
    if(Image_Start_TypeSize == 2 && file_endian != dev_endian){
        Image_Start = MC_GET_SHORT(Image_Start);
    }else if(Image_Start_TypeSize == 4 && file_endian != dev_endian){
        Image_Start = MC_GET_LONG(Image_Start);
    }
    printf("The image data start address:%d\n",Image_Start);
    fseek(fd,Image_Start,SEEK_SET);
    for(row=0;row<hei;row++){
        for(clo=0;clo<wid;clo++){
            for(dep=0;dep<3;dep++){
                count = fread(&buff,sizeof(unsigned char),1,fd);
                if(count != 1) { return -1;}
                *(Image + row*wid*channel + clo*channel + dep) = buff;
            }
        }
    }
    free(_IFD.p);
    _IFD.p = NULL;
    fclose(fd);
    return 0;
}

char Tiff_write(unsigned char const *Image,unsigned int w,unsigned int h,unsigned char channel, unsigned char *Path)
{
    IFH _IFH;
    IFD _IFD;
    FILE * fd;
    fd = fopen(Path,"wb+");
    unsigned long block_size = w*channel, sum = h*block_size;
    unsigned long i,j,k;
    /*********************************************
     * XResolution means:Width(Unit:Pixle) / Width(Unit:inch) 
     * YResolution means:Heigth(Unit:Pixle) / Heigth(Unit:inch)
     * For Example The Image:
     * PixelSize:640x480 PhysicSize:8.9x6.7(inch^2)
     * XResolution=640/8.9=72ppi  YResolution=480/6.7=72ppi (ppi:pixel per inch)
     * While store the XResolution and YResolution into the TIFF File,
     * the fraction should store in the first long type mem,
     * the denominator should store in the second long type mem.
    *********************************************/
    // Offset=w*h*d + 8(eg:Img[1000][1000][3] --> 3000008)
    // RGB  Color:W H BitsPerSample Compression Photometric StripOffset Orientation SamplePerPixle RowsPerStrip StripByteCounts PlanarConfiguration
    // Gray Color:W H BitsPerSample Compression Photometric StripOffset Orientation SamplePerPixle RowsPerStrip StripByteCounts XResolution YResoulution PlanarConfiguration ResolutionUnit
    // DE_N ID:   0 1 2             3           4           5           6           7              8            9               10          11           12                  13

    short DE_tag[DE_N]={256,257,258,259,262,273,274,277,278,279,284};
    short DE_type[DE_N]={3,  3,  3,  3,  3,  4,  3,  3,  3,  4,  3};
    unsigned long DE_size[DE_N]={1,1,3,1,1,h,1,1,1,h,1};
    unsigned long DE_val_offset[DE_N]={w,h,sum+146,1,2,sum+152+4*h,1,3,1,sum+152,1}; // 138+8=146
    short RGB_Scale[3] = {8,8,8};

    short GrayDE_tag[GrayDE_N]={256,257,258,259,262,273,274,277,278,279,282,283,284,296};
    short GrayDE_type[GrayDE_N]={3,  3,  3,  3,  3,  4,  3,  3,  3,  4,  5,  5,  3,  3};
    unsigned long GrayDE_size[GrayDE_N]={1,1,1,1,1,h,1,1,1,h,1,1,1,1};
    unsigned long GrayDE_val_offset[GrayDE_N]={w,h,8,1,1,sum+198+4*h,1,1,1,sum+198,sum+182,sum+190,1,2}; // 198 198 182 190

    unsigned long StripOffset = 8,PhyWidth=1,PhyHeight=1,PixelWidth=72,PixelHeight=72;

    _IFH.endian = 0x4949;
    _IFH.magic = 42;
    _IFH.ifd_offset = sum + 8;

    fseek(fd,0,SEEK_SET);
    fwrite(&(_IFH.endian),short_size,1,fd);
    fwrite(&(_IFH.magic),short_size,1,fd);
    fwrite(&(_IFH.ifd_offset),ulong_size,1,fd);

    if(channel==3){
        _IFD.n = DE_N;
        _IFD.p = (DE *)malloc(sizeof(DE)*_IFD.n);
        _IFD.next_ifd_offset = 0;
        for(i=0;i<DE_N;i++){
            (_IFD.p+i)->tag = DE_tag[i];
            (_IFD.p+i)->type = DE_type[i];
            (_IFD.p+i)->size = DE_size[i];
            (_IFD.p+i)->val_offset = DE_val_offset[i];
        }

        fseek(fd,_IFH.ifd_offset,SEEK_SET);
        fwrite(&(_IFD.n),short_size,1,fd);
        for(i=0;i<_IFD.n;i++){
            fwrite(&((_IFD.p+i)->tag),short_size,1,fd);
            fwrite(&((_IFD.p+i)->type),short_size,1,fd);
            fwrite(&((_IFD.p+i)->size),ulong_size,1,fd);
            fwrite(&((_IFD.p+i)->val_offset),ulong_size,1,fd);
        }
        fwrite(&(_IFD.next_ifd_offset),ulong_size,1,fd);

        printf("%d-%d\n",(_IFD.p+2)->val_offset,(_IFD.p+2)->size); // 3000146-3
        fseek(fd,(_IFD.p+2)->val_offset,SEEK_SET); // Setup the RGB grayscale for RGB image[8,8,8]
        for(i=0;i<(_IFD.p+2)->size;i++){
            fwrite(RGB_Scale+i,short_size,1,fd);
        }

        fseek(fd,(_IFD.p+5)->val_offset,SEEK_SET);
        printf("%d-%d\n",(_IFD.p+5)->val_offset,(_IFD.p+5)->size); // 3004152-1000
        for(i=0;i<(_IFD.p+5)->size;i++){
            fwrite(&StripOffset,ulong_size,1,fd); // For Small TIFF Need to change the data-type to short.
            // printf("%d-%d-%d-%d\n",StripOffset,block_size,(_IFD.p+5)->size,i);
            StripOffset += block_size;
        }

        printf("%d-%d\n",(_IFD.p+9)->val_offset,(_IFD.p+9)->size);
        fseek(fd,(_IFD.p+9)->val_offset,SEEK_SET); // Insert the block_size for every StripOffset
        for(i=0;i<(_IFD.p+9)->size;i++){
            fwrite(&block_size,ulong_size,1,fd);
        }
        fseek(fd,8,SEEK_SET); // Insert the image data
        for(i=0;i<h;i++){
            for(j=0;j<w;j++){
                for(k=0;k<channel;k++){
                    fwrite(Image+i*block_size+j*channel+k,uchar_szie,1,fd);
                }
            }
        }
    }else{
        _IFD.n = GrayDE_N;
        _IFD.p = (DE *)malloc(sizeof(DE)*_IFD.n);
        _IFD.next_ifd_offset = 0;
        for(i=0;i<GrayDE_N;i++){
            (_IFD.p+i)->tag = GrayDE_tag[i];
            (_IFD.p+i)->type = GrayDE_type[i];
            (_IFD.p+i)->size = GrayDE_size[i];
            (_IFD.p+i)->val_offset = GrayDE_val_offset[i];
        }

        fseek(fd,_IFH.ifd_offset,SEEK_SET);
        fwrite(&(_IFD.n),short_size,1,fd); // 写入DE目录的大小，一般为11个
        for(i=0;i<_IFD.n;i++){ // 依次写入文件的每个tag type size offset
            fwrite(&((_IFD.p+i)->tag),short_size,1,fd);
            fwrite(&((_IFD.p+i)->type),short_size,1,fd);
            fwrite(&((_IFD.p+i)->size),ulong_size,1,fd);
            fwrite(&((_IFD.p+i)->val_offset),ulong_size,1,fd);
        }
        fwrite(&(_IFD.next_ifd_offset),ulong_size,1,fd); // 下一个图像文件的入口位置，一般对于单一文件，这里为0

        fseek(fd,(_IFD.p+5)->val_offset,SEEK_SET);
        printf("%d-%d\n",(_IFD.p+5)->val_offset,(_IFD.p+5)->size); // 3004152-1000
        for(i=0;i<(_IFD.p+5)->size;i++){
            fwrite(&StripOffset,ulong_size,1,fd); // For Small TIFF Need to change the data-type to short.
            // printf("%d-%d-%d-%d\n",StripOffset,block_size,(_IFD.p+5)->size,i);
            StripOffset += block_size;
        }

        fseek(fd,(_IFD.p+10)->val_offset,SEEK_SET); // XResolution
        fwrite(&PixelWidth,ulong_size,1,fd); // fraction w-pixel
        fwrite(&PhyWidth,ulong_size,1,fd); // denominator PhyWidth-inch
        fseek(fd,(_IFD.p+11)->val_offset,SEEK_SET); // YResolution
        fwrite(&PixelHeight,ulong_size,1,fd); // fraction h-pixel
        fwrite(&PhyHeight,ulong_size,1,fd); // denominator PhyHeight-inch
        
        printf("%d-%d\n",(_IFD.p+9)->val_offset,(_IFD.p+9)->size);
        fseek(fd,(_IFD.p+9)->val_offset,SEEK_SET); // Insert the block_size for every StripOffset
        for(i=0;i<(_IFD.p+9)->size;i++){
            fwrite(&block_size,ulong_size,1,fd);
        }
        fseek(fd,8,SEEK_SET); // Insert the image data
        for(i=0;i<h;i++){
            for(j=0;j<w;j++){
                fwrite(Image+i*block_size+j,uchar_szie,1,fd);
            }
        }
    }
    free(_IFD.p);
    _IFD.p = NULL;
    fclose(fd);
    return 0;
}