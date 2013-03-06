/* IM 3 sample that loads an image and saves it in a memory buffer, then saves the buffer at once.

  Needs "im.lib".

  Usage: memfile <input_file_name> <output_file_name> <output_format>

    Example: memfile flower.jpg test.jpg JPEG
*/

#include <im.h>
#include <im_image.h>
#include <im_format_jp2.h>
#include <im_binfile.h>

#include <stdio.h>
#include <stdlib.h>

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

unsigned char* GetMemBuffer(imImage* image, int *size, const char* format)
{
  int error = IM_ERR_NONE;
  imFile* pMemoryFile = NULL;
  int oldMode = imBinFileSetCurrentModule(IM_MEMFILE); 

  /// This structure must exist for the lifetime of the memory file
  imBinMemoryFileName MemFileName;

  /// Setting this to null indicates that the buffer will be dynamically allocated
  MemFileName.buffer = NULL;

  /// The initial buffer size
  MemFileName.size = 1000;

  /// This constant sets the growth rate of the buffer
  MemFileName.reallocate = 2.0f;

  /// Allocate the memory file using the given format
  pMemoryFile = imFileNew((const char*)&MemFileName, format, &error); 

  /// The mode needs to be active only for the imFileOpen/imFileNew call
  imBinFileSetCurrentModule(oldMode); 

  if (!pMemoryFile)
  {
    if (MemFileName.buffer) free(MemFileName.buffer);
    PrintError(error);
    return NULL;
  }

  /// Save the imImage to the memory file
  error = imFileSaveImage(pMemoryFile, image);

  if (error == IM_ERR_NONE)
  {
    /// Obtain the number of bytes actually used
    *size = imBinFileSize((imBinFile*)imFileHandle(pMemoryFile, 0));
  }
  else
    PrintError(error);

  /// Close the memory file now
  imFileClose(pMemoryFile);

  return MemFileName.buffer;
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    printf("Invalid number of arguments.\n");
    return 0;
  }

  imFormatRegisterJP2();

  // Loads the image from file
  int error;
  imImage* image = imFileImageLoadBitmap(argv[1], 0, &error);
  if (!image)
  {
    PrintError(error);
    return 0;
  }

  int size;
  unsigned char* buffer = GetMemBuffer(image, &size, argv[3]);
  if (buffer)
  {
    imBinFile* bfile = imBinFileNew(argv[2]);
    imBinFileWrite(bfile, buffer, size, 1);
    imBinFileClose(bfile);
    free(buffer);
  }

  imImageDestroy(image);

  return 1;
}
