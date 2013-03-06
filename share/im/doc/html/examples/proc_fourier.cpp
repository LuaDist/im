/* IM 3 sample that calculates the Forward FFT, 
  process in the domain frequency, 
  and calculates the Inverse FFT.

  Needs "im.lib" and "im_fftw.lib".

  Usage: proc_fourier <input_file_name> <output_file_name> <output_format>

    Example: proc_fourier test.tif test_proc.tif TIFF
*/

#include <im.h>
#include <im_image.h>
#include <im_process.h>
#include <im_convert.h>
#include <im_complex.h>

#include <stdio.h>

void FreqDomainProc(imImage* fft_image)
{
  // a loop for all the color planes
  for (int d = 0; d < fft_image->depth; d++)
  {
    imcfloat* data = (imcfloat*)fft_image->data[d];

    for (int y = 0; y < fft_image->height; y++)
    {
      for (int x = 0; x < fft_image->width; x++)
      {
        // Do something
        // Remeber that the zero frequency is at the center
        int offset = y * fft_image->width + x;

        data[offset].imag = 0; // notice in the result that the imaginary part has an important hole.
      }
    }
  }
}

void PrintError(int error)
{
  switch (error)
  {
  case IM_ERR_OPEN:
    printf("Error Opening File.\n");
    break;
  case IM_ERR_MEM:
    printf("Insuficient memory.\n");
    break;
  case IM_ERR_ACCESS:
    printf("Error Accessing File.\n");
    break;
  case IM_ERR_DATA:
    printf("Image type not Suported.\n");
    break;
  case IM_ERR_FORMAT:
    printf("Invalid Format.\n");
    break;
  case IM_ERR_COMPRESS:
    printf("Invalid or unsupported compression.\n");
    break;
  default:
    printf("Unknown Error.\n");
  }
}

imImage* LoadImage(const char* file_name)
{
  int error;
  imFile* ifile = imFileOpen(file_name, &error);
  if (!ifile) 
  {
    PrintError(error);
    return 0;
  }
  
  imImage* image = imFileLoadImage(ifile, 0, &error);  // load the first image in the file.
  if (!image)
    PrintError(error);
    
  imFileClose(ifile);  

  return image;
}

void SaveImage(imImage* image, const char* file_name, const char* format)
{
  int error;
  imFile* ifile = imFileNew(file_name, format, &error);
  if (!ifile)
  {
    PrintError(error);
    return;
  }
  
  error = imFileSaveImage(ifile, image);
  if (error != IM_ERR_NONE)
    PrintError(error);

  imFileClose(ifile);  
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    printf("Invalid number of arguments.\n");
    return 0;
  }

  // Loads the image from file
  imImage* image = LoadImage(argv[1]);
  if (!image)
    return 0;

  // Creates a new image similar of the original but with complex data type.
  // FFTW does not requires that the image size is a power of 2.
  imImage* fft_image = imImageCreate(image->width, image->height, image->color_space, IM_CFLOAT);
  if (!image)
    return 0;

  // Forward FFT
  imProcessFFTW(image, fft_image);

  // The user processing
  FreqDomainProc(fft_image);

  // The inverse is still a complex image
  imImage* ifft_image = imImageClone(fft_image);
  if (!image)
    return 0;

  // Inverse FFT
  imProcessIFFTW(fft_image, ifft_image);

  // Converts the complex image to the same type of the original image 
  // so we can reuse its buffer
  // (usually will be a bitmap image so we can also view the result)
  if (image->data_type != IM_CFLOAT)
  {
    // This function will scan for min and max values before converting the data type
    // There wiil be no gamma conversion, use abssolute values, and only the real part will be considered.
    imConvertDataType(ifft_image, image, IM_CPX_REAL, IM_GAMMA_LINEAR, 1, IM_CAST_MINMAX);
  }

  SaveImage(image, argv[2], argv[3]);

  imImageDestroy(image);
  imImageDestroy(fft_image);
  imImageDestroy(ifft_image);

  return 1;
}
