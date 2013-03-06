wb_usr = {
  contact = "im@tecgraf.puc-rio.br",
  title_bgcolor = "#3366CC",
  copyright_link = "http://www.tecgraf.puc-rio.br",
  search_link = "http://www.tecgraf.puc-rio.br/im",
  start_size = "180",
  langs = {"en"},
  copyright_name = "Tecgraf/PUC-Rio",
  file_title = "im",          
  start_open = "1"
} 

wb_usr.messages = {
  en = {
    bar_title = "IM - Version 3.8",
    title = "IM - An Imaging Tool",
  }
} 

wb_usr.tree = 
{
  name= {en= "IM"},
  link= "home.html",
  folder=
  {
    {
      name= {en= "Product"},
      link= "prod.html",
      folder=
      {
        { name= {en= "Overview"}, link= "prod.html#overview" },
        { name= {en= "Availability"}, link= "prod.html#available" },
        { name= {en= "Support"}, link= "prod.html#support" },
        { name= {en= "Credits"}, link= "prod.html#thanks" },
        { name= {en= "Documentation"}, link= "prod.html#docs" },
        { link= "", name= {en= "" } },
        { name= {en= "Copyright/License"}, link= "copyright.html" },
        { name= {en= "Download"}, link= "download.html",
          folder=
          {
            {
              name= {en= "Library Download Tips"},
              link= "download_tips.html"
            }
          }
        },
        { name= {nl= "CVS"}, link= "cvs.html" },
        { name= {en= "History"}, link= "history.html" },
        { name= {en= "To Do"}, link= "to_do.html" },
        { name= {en= "Comparing"}, link= "toolkits.html" }
      }
    },
    {
      name= {en= "Guide"},
      link= "guide.html",
      folder=
      {
        { name= {en= "Getting Started"}, link= "guide.html#startup" },
        { name= {en= "Building Applications"}, link= "guide.html#buildapp" },
        { name= {en= "Building the Library"}, link= "guide.html#buildlib" },
        { name= {en= "CD Compatibility"}, link= "guide.html#CD" },
        { name= {en= "OpenGL Compatibility"}, link= "guide.html#opengl" },
        { name= {en= "IM 2.x Compatibility"}, link= "guide.html#compat" },
        { name= {en= "Migrating OLD Code" }, link= "guide.html#migra"   },
        { name= {en= "Names Convention"}, link= "guide.html#names" },
        { name= {en= "C x C++ Usage"}, link= "guide.html#cpp" },
        { link= "", name= {en= "" } },
        { name= {en= "Building in Linux"}, link= "building.html" },
        { name= {en= "Building in Windows"}, link= "buildwin.html" },
        { name= {en= "Samples"}, link= "samples.html" },
        { name= {en= "Lua Binding"}, link= "imlua.html" }
      }
    },
    { link= "", name= {en= "" } },
    { 
      link= "representation.html", 
      name= {en= "Representation" },
      folder=
      {
        { 
          name= {en= "Guide" },       
          link= "rep_guide.html",
          folder=
          {
            { link= "rep_guide.html#raw", name= {en= "Raw Data Buffer" } },
            { link= "rep_guide.html#imImage", name= {en= "imImage" } }
          }
        },
        { 
          name= {en= "Samples" },       
          link= "rep_samples.html",
          folder=
          {
            { link= "rep_samples.html#im_info", name= {en= "Information" } },
            { link= "rep_samples.html#im_view", name= {en= "View" } }
          }
        },
        { 
          link= "doxygen/group__imagerep.html", 
          name= {en= "Reference" },
          folder=
          {
            { 
              link= "doxygen/group__imgclass.html", name= {en= "imImage" },
              showdoc= "yes",
              folder=
              {
                { link= "doxygen/group__convert.html", name= {en= "Conversion" } }
              }
            },
            { link= "doxygen/group__imageutil.html", name= {en= "Raw Data Utilities" } },
            { link= "doxygen/group__cnvutil.html", name= {en= "Raw Data Conversion" } },
            { link= "doxygen/group__colormodeutl.html", name= {en= "Color Mode Utilities" } }
          }
        }  
      }
    },  
    { 
      link= "storage.html", 
      name= {en= "Storage" }, 
      folder=
      {
        { 
          name= {en= "Guide" },       
          link= "storage_guide.html", 
          folder=
          {
            { name= {en= "Reading" },         link= "storage_guide.html#read"    },
            { name= {en= "Writing" },         link= "storage_guide.html#write"   },
            { name= {en= "About File Formats"}, link= "storage_guide.html#formats" },
            { name= {en= "New File Formats"}, link= "storage_guide.html#filesdk" },
            { name= {en= "Memory I/O and Others"}, link= "storage_guide.html#binfilemem" }
          }
        },
        { 
          name= {en= "Samples" },       
          link= "storage_samples.html", 
          folder=
          {
            { link= "storage_samples.html#im_info", name= {en= "Information" } },
            { link= "storage_samples.html#im_copy", name= {en= "Copy" } }
          }
        },
        { 
          link= "doxygen/group__format.html", 
          name= {en= "File Formats" }, 
          folder=
          {
            { link= "doxygen/group__raw.html", name= {en= "RAW - RAW File" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__bmp.html", name= {en= "BMP - Windows Device Independent Bitmap" } },
            { link= "doxygen/group__gif.html", name= {en= "GIF - Graphics Interchange Format" } },
            { link= "doxygen/group__ico.html", name= {en= "ICO - Windows Icon" } },
            { link= "doxygen/group__jpeg.html", name= {en= "JPEG - JPEG File Interchange Format" } },
            { link= "doxygen/group__krn.html", name= {en= "KRN - IM Kernel File Format" } },
            { link= "doxygen/group__led.html", name= {en= "LED - IUP image in LED" } },
            { link= "doxygen/group__pcx.html", name= {en= "PCX - ZSoft Picture" } },
            { link= "doxygen/group__png.html", name= {en= "PNG - Portable Network Graphic Format" } },
            { link= "doxygen/group__pnm.html", name= {en= "PNM - Netpbm Portable Image Map" } },
            { link= "doxygen/group__ras.html", name= {en= "RAS - Sun Raster File" } },
            { link= "doxygen/group__sgi.html", name= {en= "SGI - Silicon Graphics Image File Format" } },
            { link= "doxygen/group__tga.html", name= {en= "TGA - Truevision Graphics Adapter File" } },
            { link= "doxygen/group__tiff.html", name= {en= "TIFF - Tagged Image File Format" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__avi.html", name= {en= "AVI - Windows Audio-Video Interleaved RIFF" } },
            { link= "doxygen/group__jp2.html", name= {en= "JP2 - JPEG-2000 JP2 File Format" } },
            { link= "doxygen/group__wmv.html", name= {en= "WMV - Windows Media Video Format" } },
            { link= "doxygen/group__ecw.html", name= {en= "ECW - ERMapper ECW File Format" } },
          }
        },
        { 
          link= "doxygen/group__file.html", 
          name= {en= "Reference" }, 
          folder=
          {
            { link= "doxygen/group__imgfile.html", name= {en= "imImage Storage" } },
            { link= "doxygen/group__filesdk.html", name= {en= "File Format SDK" } }
          }
        }  
      }
    },  
    { 
      link= "capture.html", 
      name= {en= "Capture" },
      folder=
      {
        { 
          name= {en= "Guide" },       
          link= "capture_guide.html", 
          folder=
          {
            { link= "capture_guide.html#Using", name= {en= "Using" } },
            { link= "capture_guide.html#Building", name= {en= "Building" } }
          }
        },
        { 
          name= {en= "Samples" },       
          link= "capture_samples.html",
          folder=
          {
            { link= "capture_samples.html#glut_capture", name= {en= "GLUT Capture" } },
            { link= "capture_samples.html#iupglcap", name= {en= "IUP OpenGL Capture" } }
          }
        },
        { 
          link= "doxygen/group__capture.html", 
          name= {en= "Reference" },
          folder=
          {
            { link= "doxygen/group__winattrib.html", name= {en= "Windows Attributes" } }
          }
        }
      }
    },
    { 
      link= "processing.html", 
      name= {en= "Processing" }, 
      folder=
      {
        { 
          name= {en= "Guide" },       
          link= "proc_guide.html", 
          folder=
          {
            { link= "proc_guide.html#using", name= {en= "Using" } },
            { link= "proc_guide.html#new", name= {en= "New Operations" } },
            { link= "proc_guide.html#count", name= {en= "Counters" } }
          }
        },
        { 
          name= {en= "Samples" },       
          link= "proc_samples.html",
          folder=
          {
            { link= "proc_samples.html#proc_fourier", name= {en= "Fourier Transform" } },
            { link= "proc_samples.html#houghlines", name= {en= "Hough Lines" } },
            { link= "proc_samples.html#analysis", name= {en= "Image Analysis" } }
          }
        },
        { 
          link= "doxygen/group__process.html", 
          name= {en= "Reference" }, 
          folder=
          {
            { link= "doxygen/group__render.html", name= {en= "Synthetic Image Render" } },
            { link= "doxygen/group__procconvert.html", name= {en= "Image Conversion" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__resize.html", name= {en= "Image Resize" } },
            { link= "doxygen/group__geom.html", name= {en= "Geometric Operations" } },
            { link= "doxygen/group__quantize.html", name= {en= "Additional Image Quantization Operations" } },
            { link= "doxygen/group__colorproc.html", name= {en= "Color Processing Operations" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__histo.html", name= {en= "Histogram Based Operations" } },
            { link= "doxygen/group__threshold.html", name= {en= "Threshold Operations" } },
            { link= "doxygen/group__arithm.html", name= {en= "Arithmetic Operations" } },
            { link= "doxygen/group__logic.html", name= {en= "Logical Arithmetic Operations" } },
            { link= "doxygen/group__tonegamut.html", name= {en= "Tone Gamut Operations" } },
            { link= "doxygen/group__point.html", name= {en= "Point Based Custom Operations" } },
            { link= "doxygen/group__imageenhance.html", name= {en= "Image Enhance Utilities in Lua" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__convolve.html", name= {en= "Convolution Operations" } },
            { link= "doxygen/group__kernel.html", name= {en= "Predefined Kernels" } },
            { link= "doxygen/group__rank.html", name= {en= "Rank Convolution Operations" } },
            { link= "doxygen/group__morphbin.html", name= {en= "Morphology Operations for Binary Images" } },
            { link= "doxygen/group__morphgray.html", name= {en= "Morphology Operations for Gray Images" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__fourier.html", name= {en= "Fourier Transform Operations" } },
            { link= "doxygen/group__transform.html", name= {en= "Other Domain Transform Operations" } },
            { link= "doxygen/group__effects.html", name= {en= "Special Effects" } },
            { link= "doxygen/group__remotesens.html", name= {en= "Remote Sensing Operations" } },
            { link= "doxygen/group__openmp.html", name= {en= "OpenMP Utilities" } },
            { link= "",                        name= {en= "" } },
            { link= "doxygen/group__stats.html", name= {en= "Image Statistics" } },
            { link= "doxygen/group__analyze.html", name= {en= "Image Analysis" } }
          }
        }  
      }
    },  
    { link= "", name= {en= "" } },
    { 
      link= "doxygen/modules.html", 
      name= {en= "Additional Reference" },
      folder=
      {
        { 
          link= "doxygen/group__util.html", 
          name= {en= "Utilities" }, 
          folder=
          {
            { link= "doxygen/group__lib.html", name= {en= "Library Management" } },
            { link= "doxygen/group__imlua.html", name= {en= "Lua Binding" } },
            { link= "", name= {en= "" } },
            { link= "doxygen/group__colorutl.html", name= {en= "Color Utilities" } },
            { 
              link= "doxygen/group__color.html", 
              name= {en= "Color Manipulation" }, 
              showdoc= "yes",
              folder=
              {
                { link= "doxygen/group__hsi.html", name= {en= "HSI Color Coordinate System Conversions" } }
              }
            },  
            { link= "doxygen/group__palette.html", name= {en= "Palette Generators" } },
            { link= "", name= {en= "" } },
            { link= "doxygen/group__bin.html", name= {en= "Binary Data Utilities" } },
            { link= "doxygen/group__cpx.html", name= {en= "Complex Numbers" } },
            { link= "doxygen/group__datatypeutl.html", name= {en= "Data Type Utilities" } },
            { link= "doxygen/group__math.html", name= {en= "Math Utilities" } },
            { link= "doxygen/group__str.html", name= {en= "String Utilities" } },
            { link= "", name= {en= "" } },
            { link= "doxygen/group__binfile.html", name= {en= "Binary File Access" } },
            { link= "doxygen/group__compress.html", name= {en= "Data Compression Utilities" } },
            { link= "doxygen/group__counter.html", name= {en= "Counter" } },
            { link= "doxygen/group__dib.html", name= {en= "Windows DIB" } }
          }
        },  
        { link= "", name= {en= "" } },
        { 
          link= "doxygen/annotated.html", 
          name= {en= "Structures" },
          folder=
          {
            { link= "doxygen/struct__imBinMemoryFileName.html", name= {en= "imBinMemoryFileName" } },
            { link= "doxygen/struct__imDib.html", name= {en= "imDib" } },
            { link= "doxygen/struct__imFile.html", name= {en= "imFile" } },
            { link= "doxygen/struct__imImage.html", name= {en= "imImage" } },
            { link= "doxygen/struct__imStats.html", name= {en= "imStats" } },
            { link= "", name= {en= "" } },
            { link= "doxygen/classimAttribArray.html", name= {en= "imAttribArray" } },
            { link= "doxygen/classimAttribTable.html", name= {en= "imAttribTable" } },
            { link= "doxygen/classimBinFileBase.html", name= {en= "imBinFileBase" } },
            { link= "doxygen/classimCapture.html", name= {en= "imCapture" } },
            { link= "doxygen/classimcfloat.html", name= {en= "imcfloat" } },
            { link= "doxygen/classimFileFormatBase.html", name= {en= "imFileFormatBase" } },
            { link= "doxygen/classimFormat.html", name= {en= "imFormat" } },
            { link= "doxygen/classimImageFile.html", name= {en= "imImageFile" } },
          }
        },  
        { 
          link= "doxygen/files.html", 
          name= {en= "Includes" }, 
          folder=
          {
            { link= "doxygen/im_8h.html", name= {en= "im.h" } },
            { link= "doxygen/im__attrib_8h.html", name= {en= "im_attrib.h" } },
            { link= "doxygen/im__attrib__flat_8h.html", name= {en= "im_attrib_flat.h" } },
            { link= "doxygen/im__binfile_8h.html", name= {en= "im_binfile.h" } },
            { link= "doxygen/im__capture_8h.html", name= {en= "im_capture.h" } },
            { link= "doxygen/im__color_8h.html", name= {en= "im_color.h" } },
            { link= "doxygen/im__colorhsi_8h.html", name= {en= "im_colorhsi.h" } },
            { link= "doxygen/im__complex_8h.html", name= {en= "im_complex.h" } },
            { link= "doxygen/im__convert_8h.html", name= {en= "im_convert.h" } },
            { link= "doxygen/im__counter_8h.html", name= {en= "im_counter.h" } },
            { link= "doxygen/im__dib_8h.html", name= {en= "im_dib.h" } },
            { link= "doxygen/im__file_8h.html", name= {en= "im_file.h" } },
            { link= "doxygen/im__format_8h.html", name= {en= "im_format.h" } },
            { link= "doxygen/im__format__all_8h.html", name= {en= "im_format_all.h" } },
            { link= "doxygen/im__format__avi_8h.html", name= {en= "im_format_avi.h" } },
            { link= "doxygen/im__format__ecw_8h.html", name= {en= "im_format_ecw.h" } },
            { link= "doxygen/im__format__jp2_8h.html", name= {en= "im_format_jp2.h" } },
            { link= "doxygen/im__format__raw_8h.html", name= {en= "im_format_raw.h" } },
            { link= "doxygen/im__format__wmv_8h.html", name= {en= "im_format_wmv.h" } },
            { link= "doxygen/im__image_8h.html", name= {en= "im_image.h" } },
            { link= "doxygen/im__kernel_8h.html", name= {en= "im_kernel.h" } },
            { link= "doxygen/im__lib_8h.html", name= {en= "im_lib.h" } },
            { link= "doxygen/im__math_8h.html", name= {en= "im_math.h" } },
            { link= "doxygen/im__math__op_8h.html", name= {en= "im_math_op.h" } },
            { link= "doxygen/im__palette_8h.html", name= {en= "im_palette.h" } },
            { link= "doxygen/im__plus_8h.html", name= {en= "im_plus.h" } },
            { link= "doxygen/im__process__ana_8h.html", name= {en= "im_process_ana.h" } },
            { link= "doxygen/im__process__glo_8h.html", name= {en= "im_process_glo.h" } },
            { link= "doxygen/im__process__loc_8h.html", name= {en= "im_process_loc.h" } },
            { link= "doxygen/im__process__pnt_8h.html", name= {en= "im_process_pnt.h" } },
            { link= "doxygen/im__raw_8h.html", name= {en= "im_raw.h" } },
            { link= "doxygen/im__util_8h.html", name= {en= "im_util.h" } },
            { link= "doxygen/imlua_8h.html", name= {en= "imlua.h" } },
            { link= "doxygen/old__im_8h.html", name= {en= "old_im.h" } }
          }
        },
        { 
          link= "doxygen/globals.html", 
          name= {en= "Globals" }, 
          folder=
          {
            { link= "doxygen/globals_func.html", name= {en= "Functions" } },
            { link= "doxygen/globals_type.html", name= {en= "Typedefs" } },
            { link= "doxygen/globals_enum.html", name= {en= "Enumerations" } },
            { link= "doxygen/globals_eval.html", name= {en= "Enumeration Values" } },
            { link= "doxygen/globals_defs.html", name= {en= "Defines" } },
          }
        }  
      }
    }
  }
} 
