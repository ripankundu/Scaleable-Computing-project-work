"""
@author:Ripan Kumar Kundu
M.Sc in Electrical Engineering
University of Rostock
"""
#include <iostream>
#include <cstdlib>
#include <png.h>
#include<omp.h>
#define SIZE 6000
#define NUMBER_OF_THREADS 4
using namespace std;

int CHUNKSIZE = 375;
/* input and output image  pixel value storing 2D array */
unsigned char pointerImageRowBytes[6000][6000];
unsigned char convolvedImagePixel[6000][6000];

/* loop variables */
int i,j;


int stencil_code[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

/* width and height of input and output image */
int width, height;

png_structp png_ptr;
png_infop info_ptr;

/* new pixel value after stencil operation on input image*/
int new_pixel_value;

/* number bits used to store image information */
png_byte bit_depth;

/* pixel value pointer */
png_bytep *column_wise_pixel_value;


/* function declaration for stencil operation */
int FilterImage(unsigned char pointerImageRowBytes[6000][6000], int i, int j);

/* class defination  */
class PNG
{
private:
public:

    /* function declaration for reading image */
    void ReadImage(char const *imagefile);

    /* function declaration for getting pixel value and  processed to stencil operation */
    void GetPixelAndProcess(void);

    /* function declaration for writing image */
    void WriteImage(char const *imagefile);

};

int main(int argc, char** argv)
{
    /* object creation of class */
    PNG object;

    object.ReadImage("test-image.png");
    double start = omp_get_wtime();
    object.GetPixelAndProcess();
    double end = omp_get_wtime();
    cout<<"\n\nExecution Time for Stencil operation is "<<(end-start)<<" seconds\n\n ";
    object.WriteImage("omp.png");

    return 0;
}

/* function defination for stencil operation */
int FilterImage(unsigned char pointerImageRowBytes[6000][6000], int i, int j)
{
    new_pixel_value = 8*pointerImageRowBytes[j][i]-pointerImageRowBytes[j-1][i-1]-pointerImageRowBytes[j][i-1]-pointerImageRowBytes[j+1][i-1]-pointerImageRowBytes[j-1][i]-pointerImageRowBytes[j+ 1][i]-pointerImageRowBytes[j-1][i+1]-pointerImageRowBytes[j][i+1]-pointerImageRowBytes[j+1][i+1];

    if (new_pixel_value>255)

        return new_pixel_value = 255;

    else if (new_pixel_value < 0)

        return new_pixel_value = 0;

    else
        return new_pixel_value;
}

/* member function defination outside the class */
void PNG::ReadImage(char const *imagefile)
{

    FILE *file_pointer ;
    file_pointer = fopen(imagefile, "rb");

    if(file_pointer == NULL)
    {
        cout<<"File does not exist ..."<<endl;
        exit(1);
    }

    /* allocate and initialize a png_struct structure for reading PNG file */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    /* allocate and initialize a png_info structure */
    info_ptr = png_create_info_struct(png_ptr);

    /* initialize input/output for the PNG file */
    png_init_io(png_ptr, file_pointer);

    /* read the PNG image information */
    png_read_info(png_ptr, info_ptr);

    width      = png_get_image_width(png_ptr, info_ptr);
    height     = png_get_image_height(png_ptr, info_ptr);
    bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    /* add a filler byte to given image */
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    png_set_gray_to_rgb(png_ptr);

    /* update png_info structure */
    png_read_update_info(png_ptr, info_ptr);

    column_wise_pixel_value = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for(int j = 0; j < height; j++)
    {
        column_wise_pixel_value[j] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    /* read the entire image into memory */
    png_read_image(png_ptr, column_wise_pixel_value);

    fclose(file_pointer);
}

void PNG::GetPixelAndProcess(void)
{




    #pragma omp parallel shared(pointerImageRowBytes,convolvedImagePixel,CHUNKSIZE) private(i,j) num_threads(NUMBER_OF_THREADS)
    {

        #pragma omp for schedule (static,CHUNKSIZE)
        /* storing each pixel value (column wise) in InputImage */
    for(int j = 0; j < height; j++)
    {
        png_bytep column_value = column_wise_pixel_value[j];
        for(int i = 0; i < width; i++)
        {
            png_bytep px = &(column_value[i * 4]);
            pointerImageRowBytes[j][i]=px[0];
        }
    }
    
      

       #pragma omp for schedule (static,CHUNKSIZE)
         /* perforing stencil operation on InputImage */
    for (int j= 0; j< height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            /*avoiding stencil operation on boundary of image */
            if (i == 0 || j == 0 || i == width-1 || j == height-1)
            {
                /* storing each pixel value of boundary (column wise) in OutputImage */
                convolvedImagePixel[j][i] =pointerImageRowBytes[j][i];
            }
            else
            {
                /* storing each pixel value (column wise) in OutputImage */
                convolvedImagePixel[j][i] = FilterImage(pointerImageRowBytes,i,j);
            }
        }
    }

       #pragma omp for schedule (static,CHUNKSIZE)
        for (int j=0; j<height; j++)
    {
        png_bytep column_value = column_wise_pixel_value[j];
        for (int i = 0; i < width; i++)
        {
            png_bytep px = &(column_value[i*4]);

            /* storing each new pixel value for RGB color */
            px[0]= convolvedImagePixel[j][i];
            px[1] = px[0];
            px[2] = px[0];
        }

    }


}

}

/* member function defination outside the class */
void PNG:: WriteImage(char const *file_name)
{
    FILE *file_pointer = fopen(file_name, "wb");
    if(!file_pointer) abort();

    /* allocate and initialize a png_struct structure for writing PNG file */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) abort();

    /* allocate and initialize a png_info structure */
  info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr) abort();
    if (setjmp(png_jmpbuf(png_ptr))) abort();

    /* initialize input/output for the PNG file */
    png_init_io(png_ptr, file_pointer);


    /* set the PNG_IHDR chunk information */
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width, height,
        bit_depth,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    /* write PNG information to file */
    png_write_info(png_ptr, info_ptr);

    /* write the given image data */
    png_write_image(png_ptr, column_wise_pixel_value);

    /* write the end of a PNG file */
    png_write_end(png_ptr, NULL);


    for(int j = 0; j < height; j++)
    {
        free(column_wise_pixel_value[j]);
    }
    free(column_wise_pixel_value);
    fclose(file_pointer);
}
