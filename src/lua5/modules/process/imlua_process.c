/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#include <memory.h>
#include <math.h>
#include <stdlib.h>

#include "im.h"
#include "im_image.h"
#include "im_process.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"

#ifdef _OPENMP
#include <omp.h>
#endif


/* NOTE: This can breaks on multithread ONLY if using multiple states. */
/* Used ONLY in im.ProcessRenderOp and im.ProcessRenderCondOp. */
static lua_State *g_State = NULL;


/*****************************************************************************\
 Local Utilities
\*****************************************************************************/
static void imlua_errorcfloat(lua_State *L, int index)
{
  luaL_argerror(L, index, "image data type can NOT be cfloat");
}

#define imlua_checknotcfloat(_L, _a, _i) if ((_i)->data_type == IM_CFLOAT) \
                                           imlua_errorcfloat(_L, _a)

static int imlua_unpacktable(lua_State *L, int index)
{
  int i, n = imlua_getn(L, index);

  for (i = 0; i < n; i++)
    lua_rawgeti(L, index, i+1);

  return n;
}

static void imlua_checkhistogramtype(lua_State *L, int index, imImage* image)
{
  if (image->data_type != IM_BYTE && 
      image->data_type != IM_SHORT &&
      image->data_type != IM_USHORT)
    luaL_argerror(L, index, "image data type must be byte, short or ushort");
}

/*****************************************************************************\
 Image Statistics Calculations
\*****************************************************************************/

/*****************************************************************************\
 im.CalcRMSError(image1, image2)
\*****************************************************************************/
static int imluaCalcRMSError (lua_State *L)
{
  imImage* image1 = imlua_checkimage(L, 1);
  imImage* image2 = imlua_checkimage(L, 2);

  imlua_match(L, image1, image2);

  lua_pushnumber(L, imCalcRMSError(image1, image2));
  return 1;
}

/*****************************************************************************\
 im.CalcSNR(src_image, noise_image)
\*****************************************************************************/
static int imluaCalcSNR (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* noise_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, noise_image);

  lua_pushnumber(L, imCalcSNR(src_image, noise_image));
  return 1;
}

/*****************************************************************************\
 im.CalcCountColors(src_image)
\*****************************************************************************/
static int imluaCalcCountColors (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);

  if (imColorModeDepth(src_image->color_space) > 1)
  {
    if (src_image->color_space == IM_CMYK)
      luaL_argerror(L, 1, "color space can not be CMYK");

    imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  }
  else
    imlua_checkhistogramtype(L, 1, src_image);

  lua_pushnumber(L, imCalcCountColors(src_image));
  return 1;
}

/*****************************************************************************\
 im.CalcHistogram(src_image, plane, cumulative)
\*****************************************************************************/
static int imluaCalcHistogram (lua_State *L)
{
  int hcount;
  unsigned long *histo;
  imImage* src_image = imlua_checkimage(L, 1);
  int plane = luaL_checkint(L, 2);
  int cumulative = lua_toboolean(L, 3);

  imlua_checkhistogramtype(L, 1, src_image);

  histo = imHistogramNew(src_image->data_type, &hcount);

  imCalcHistogram(src_image, histo, plane, cumulative);

  imlua_newarrayulong(L, histo, hcount, 0);
  imHistogramRelease(histo);
  return 1;
}

/*****************************************************************************\
 im.CalcGrayHistogram(src_image, cumulative)
\*****************************************************************************/
static int imluaCalcGrayHistogram (lua_State *L)
{
  int hcount;
  unsigned long *histo;
  imImage* src_image = imlua_checkimage(L, 1);
  int cumulative = lua_toboolean(L, 2);

  imlua_checkhistogramtype(L, 1, src_image);

  if (src_image->color_space >= IM_CMYK)
    luaL_argerror(L, 1, "color space can be RGB, Gray, Binary or Map only");

  histo = imHistogramNew(src_image->data_type, &hcount);

  imCalcGrayHistogram(src_image, histo, cumulative);
  imlua_newarrayulong(L, histo, hcount, 0);

  imHistogramRelease(histo);

  return 1;
}

static void imlua_pushStats(lua_State *L, imStats* stats, int depth)
{
  if (depth == 1)
  {
    lua_newtable(L);
    lua_pushstring(L, "max");      lua_pushnumber(L, stats->max);      lua_rawset(L, -3);
    lua_pushstring(L, "min");      lua_pushnumber(L, stats->min);      lua_rawset(L, -3);
    lua_pushstring(L, "positive"); lua_pushnumber(L, stats->positive); lua_rawset(L, -3);
    lua_pushstring(L, "negative"); lua_pushnumber(L, stats->negative); lua_rawset(L, -3);
    lua_pushstring(L, "zeros");    lua_pushnumber(L, stats->zeros);    lua_rawset(L, -3);
    lua_pushstring(L, "mean");     lua_pushnumber(L, stats->mean);     lua_rawset(L, -3);
    lua_pushstring(L, "stddev");   lua_pushnumber(L, stats->stddev);   lua_rawset(L, -3);
  }
  else
  {
    int d;

    lua_newtable(L);

    for (d = 0; d < depth; d++)
    {
      imlua_pushStats(L, &stats[d], 1);
      lua_rawseti(L, -2, d);
    }
  }
}

/*****************************************************************************\
 im.CalcImageStatistics(src_image)
\*****************************************************************************/
static int imluaCalcImageStatistics (lua_State *L)
{
  imStats stats[4];
  imImage *image = imlua_checkimage(L, 1);

  if (image->data_type == IM_CFLOAT)
    luaL_argerror(L, 1, "data type can NOT be of type cfloat");

  imCalcImageStatistics(image, stats);

  imlua_pushStats(L, stats, image->depth);
  return 1;
}

/*****************************************************************************\
 im.CalcHistogramStatistics(src_image)
\*****************************************************************************/
static int imluaCalcHistogramStatistics (lua_State *L)
{
  imStats stats[4];
  imImage *image = imlua_checkimage(L, 1);

  imlua_checkhistogramtype(L, 1, image);

  imCalcHistogramStatistics(image, stats);

  imlua_pushStats(L, stats, image->depth);
  return 1;
}

/*****************************************************************************\
 im.CalcHistoImageStatistics
\*****************************************************************************/
static int imluaCalcHistoImageStatistics (lua_State *L)
{
  int* median;
  int* mode;

  imImage *image = imlua_checkimage(L, 1);

  imlua_checkhistogramtype(L, 1, image);

  median = (int*)malloc(sizeof(int)*image->depth);
  mode = (int*)malloc(sizeof(int)*image->depth);

  imCalcHistoImageStatistics(image, median, mode);

  imlua_newarrayint (L, median, image->depth, 0);
  imlua_newarrayint (L, mode, image->depth, 0);

  free(median);
  free(mode);

  return 2;
}

static int imluaCalcPercentMinMax(lua_State *L)
{
  int min, max;

  imImage *image = imlua_checkimage(L, 1);
  float percent = (float)luaL_checknumber(L, 2);
  int ignore_zero = lua_toboolean(L, 3);

  imlua_checkhistogramtype(L, 1, image);

  imCalcPercentMinMax(image, percent, ignore_zero, &min, &max);

  lua_pushinteger(L, min);
  lua_pushinteger(L, max);
  return 2;
}


/*****************************************************************************\
 Image Analysis
\*****************************************************************************/

/*****************************************************************************\
 im.AnalyzeFindRegions(src_image, dst_image, connect, touch_border)
\*****************************************************************************/
static int imluaAnalyzeFindRegions (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);
  int touch_border = lua_toboolean(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_USHORT);

  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");
  lua_pushnumber(L, imAnalyzeFindRegions(src_image, dst_image, connect, touch_border));
  return 1;
}

static int iGetMax(imImage* image)
{
  int max = 0;
  int i;

  imushort* data = (imushort*)image->data[0];
  for (i = 0; i < image->count; i++)
  {
    if (*data > max)
      max = *data;

    data++;
  }

  return max;
}

static int imlua_checkregioncount(lua_State *L, int narg, imImage* image)
{
  if (lua_isnoneornil(L, narg)) return iGetMax(image);
  else return (int)luaL_checknumber(L, narg);
}


/*****************************************************************************\
 im.AnalyzeMeasureArea(image, [count])
\*****************************************************************************/
static int imluaAnalyzeMeasureArea (lua_State *L)
{
  int count;
  int *area;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  area = (int*) malloc(sizeof(int) * count);

  imAnalyzeMeasureArea(image, area, count);

  imlua_newarrayint(L, area, count, 0);
  free(area);

  return 1;
}

/*****************************************************************************\
 im.AnalyzeMeasurePerimArea(image)
\*****************************************************************************/
static int imluaAnalyzeMeasurePerimArea (lua_State *L)
{
  int count;
  float *perimarea;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  perimarea = (float*) malloc(sizeof(float) * count);

  imAnalyzeMeasurePerimArea(image, perimarea);

  imlua_newarrayfloat (L, perimarea, count, 0);
  free(perimarea);

  return 1;
}

/*****************************************************************************\
 im.AnalyzeMeasureCentroid(image, [area], [count])
\*****************************************************************************/
static int imluaAnalyzeMeasureCentroid (lua_State *L)
{
  int count;
  float *cx, *cy;
  int *area;

  imImage* image = imlua_checkimage(L, 1);
  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 3, image);

  /* minimize leak when error, checking array after other checks */
  area = imlua_toarrayintopt(L, 2, &count, 0);

  cx = (float*) malloc (sizeof(float) * count);
  cy = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasureCentroid(image, area, count, cx, cy);

  imlua_newarrayfloat(L, cx, count, 0);
  imlua_newarrayfloat(L, cy, count, 0);

  if (area)
    free(area);
  free(cx);
  free(cy);

  return 2;
}

/*****************************************************************************\
 im.AnalyzeMeasurePrincipalAxis(image, [area], [cx], [cy])
\*****************************************************************************/
static int imluaAnalyzeMeasurePrincipalAxis (lua_State *L)
{
  int count;
  float *cx, *cy;
  int *area;
  float *major_slope, *major_length, *minor_slope, *minor_length;

  imImage* image = imlua_checkimage(L, 1);
  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 5, image);

  /* minimize leak when error, checking array after other checks */
  area = imlua_toarrayintopt(L, 2, &count, 0);  
  cx = imlua_toarrayfloatopt(L, 3, NULL, 0);
  cy = imlua_toarrayfloatopt(L, 4, NULL, 0);

  major_slope = (float*) malloc (sizeof(float) * count);
  major_length = (float*) malloc (sizeof(float) * count);
  minor_slope = (float*) malloc (sizeof(float) * count);
  minor_length = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasurePrincipalAxis(image, area, cx, cy, count, major_slope, major_length, minor_slope, minor_length);

  imlua_newarrayfloat(L, major_slope, count, 0);
  imlua_newarrayfloat(L, major_length, count, 0);
  imlua_newarrayfloat(L, minor_slope, count, 0);
  imlua_newarrayfloat(L, minor_length, count, 0);

  if (area)
    free(area);
  if (cx)
    free(cx);
  if (cy)
    free(cy);

  free(major_slope);
  free(major_length);
  free(minor_slope);
  free(minor_length);

  return 4;
}

/*****************************************************************************\
 im.AnalyzeMeasureHoles
\*****************************************************************************/
static int imluaAnalyzeMeasureHoles (lua_State *L)
{
  int holes_count, count;
  int connect;
  int *area = NULL;
  float *perim = NULL;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  connect = luaL_checkint(L, 2);
  count = imlua_checkregioncount(L, 3, image);

  area = (int*) malloc (sizeof(int) * count);
  perim = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasureHoles(image, connect, &holes_count, area, perim);

  lua_pushnumber(L, holes_count);
  imlua_newarrayint(L, area, holes_count, 0);
  imlua_newarrayfloat(L, perim, holes_count, 0);

  if (area)
    free(area);
  if (perim)
    free(perim);

  return 3;
}

/*****************************************************************************\
 im.AnalyzeMeasurePerimeter(image, [count])
\*****************************************************************************/
static int imluaAnalyzeMeasurePerimeter (lua_State *L)
{
  int count;
  float *perim;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  perim = (float*) malloc(sizeof(float) * count);

  imAnalyzeMeasurePerimeter(image, perim, count);

  imlua_newarrayfloat(L, perim, count, 0);

  free(perim);

  return 1;
}

/*****************************************************************************\
 im.ProcessPerimeterLine(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessPerimeterLine (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image, dst_image);

  imProcessPerimeterLine(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessRemoveByArea(src_image, dst_image, connect, start_size, end_size, inside)
\*****************************************************************************/
static int imluaProcessRemoveByArea (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);
  int start_size = luaL_checkint(L, 4);
  int end_size = luaL_checkint(L, 5);
  int inside = lua_toboolean(L, 6);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");

  imProcessRemoveByArea(src_image, dst_image, connect, start_size, end_size, inside);
  return 0;
}

/*****************************************************************************\
 im.ProcessFillHoles(src_image, dst_image, connect)
\*****************************************************************************/
static int imluaProcessFillHoles (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");

  imProcessFillHoles(src_image, dst_image, connect);
  return 0;
}

static void imlua_checkhoughsize(lua_State *L, imImage* image, imImage* hough_image, int param)
{
#define IMSQR(_x) (_x*_x)
  int hough_rmax;
  if (hough_image->width != 180)
    luaL_argerror(L, param, "invalid image width");

  hough_rmax = (int)(sqrt((double)(IMSQR(image->width) + IMSQR(image->height)))/2.0);
  if (hough_image->height != 2*hough_rmax+1)
    luaL_argerror(L, param, "invalid image height");
}

/*****************************************************************************\
 im.ProcessHoughLines(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessHoughLines (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_INT);
  imlua_checkhoughsize(L, src_image, dst_image, 2);

  lua_pushboolean(L, imProcessHoughLines(src_image, dst_image));
  return 0;
}

/*****************************************************************************\
 im.ProcessHoughLinesDraw(src_image, hough, hough_points, dst_image)
\*****************************************************************************/
static int imluaProcessHoughLinesDraw (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* hough_points = imlua_checkimage(L, 3);
  imImage* dst_image = imlua_checkimage(L, 4);
  imImage* hough = NULL;

  if (lua_isuserdata(L, 2)) /* optional */
  {
    hough = imlua_checkimage(L, 2);
    imlua_checktype(L, 2, hough, IM_GRAY, IM_INT);
    imlua_checkhoughsize(L, src_image, hough, 2);
  }

  if (src_image->color_space != IM_GRAY && 
      src_image->color_space != IM_MAP &&
      src_image->color_space != IM_RGB)
    luaL_argerror(L, 1, "image must be RGB, Map or Gray");
  imlua_checkdatatype(L, 1, src_image, IM_BYTE);

  imlua_checkcolorspace(L, 3, hough_points, IM_BINARY);
  imlua_checkhoughsize(L, src_image, hough_points, 3);
  imlua_match(L, src_image, dst_image);

  lua_pushnumber(L, imProcessHoughLinesDraw(src_image, hough, hough_points, dst_image));
  return 0;
}

/*****************************************************************************\
 im.ProcessDistanceTransform(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessDistanceTransform (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checkdatatype(L, 2, dst_image, IM_FLOAT);
  imlua_matchsize(L, src_image, dst_image);

  imProcessDistanceTransform(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessRegionalMaximum(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessRegionalMaximum (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_FLOAT);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessRegionalMaximum(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 Image Resize
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessReduce(src_image, dst_image, order)
\*****************************************************************************/
static int imluaProcessReduce (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int order = luaL_checkint(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1), 3, "invalid order, must be 0 or 1");

  lua_pushboolean(L, imProcessReduce(src_image, dst_image, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessResize(src_image, dst_image, order)
\*****************************************************************************/
static int imluaProcessResize (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int order = luaL_checkint(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 3, "invalid order, must be 0, 1 or 3");

  lua_pushboolean(L, imProcessResize(src_image, dst_image, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessReduceBy4(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessReduceBy4 (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L,
    dst_image->width == (src_image->width / 2) &&
    dst_image->height == (src_image->height / 2), 3, "destiny image size must be source image width/2, height/2");

  imProcessReduceBy4(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCrop(src_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessCrop (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int xmin = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, xmin >= 0 && xmin < src_image->width, 3, "xmin must be >= 0 and < width");
  luaL_argcheck(L, ymin >= 0 && ymin < src_image->height, 4, "ymin must be >= 0 and < height");
  luaL_argcheck(L, dst_image->width <= (src_image->width - xmin), 2, "destiny image size must be smaller than source image width-xmin");
  luaL_argcheck(L, dst_image->height <= (src_image->height - ymin), 2, "destiny image size must be smaller than source image height-ymin");

  imProcessCrop(src_image, dst_image, xmin, ymin);
  return 0;
}

/*****************************************************************************\
 im.ProcessInsert(src_image, region_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessInsert (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* region_image = imlua_checkimage(L, 2);
  imImage* dst_image = imlua_checkimage(L, 3);
  int xmin = luaL_checkint(L, 4);
  int ymin = luaL_checkint(L, 5);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, xmin >= 0 && xmin < src_image->width, 3, "xmin must be >= 0 and < width");
  luaL_argcheck(L, ymin >= 0 && ymin < src_image->height, 3, "ymin must be >= 0 and < height");

  imProcessInsert(src_image, region_image, dst_image, xmin, ymin);
  return 0;
}

/*****************************************************************************\
 im.ProcessAddMargins(src_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessAddMargins (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int xmin = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, dst_image->width >= (src_image->width + xmin), 2, "destiny image size must be greatter or equal than source image width+xmin, height+ymin");
  luaL_argcheck(L, dst_image->height >= (src_image->height + ymin), 2, "destiny image size must be greatter or equal than source image width+xmin, height+ymin");

  imProcessAddMargins(src_image, dst_image, xmin, ymin);
  return 0;
}



/*****************************************************************************\
 Geometric Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessCalcRotateSize
\*****************************************************************************/
static int imluaProcessCalcRotateSize (lua_State *L)
{
  int new_width, new_height;

  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);

  imProcessCalcRotateSize(width, height, &new_width, &new_height, cos0, sin0);
  lua_pushnumber(L, new_width);
  lua_pushnumber(L, new_height);
  return 2;
}

/*****************************************************************************\
 im.ProcessRotate
\*****************************************************************************/
static int imluaProcessRotate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);
  int order = luaL_checkint(L, 5);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 5, "invalid order, must be 0, 1 or 3");

  lua_pushboolean(L, imProcessRotate(src_image, dst_image, cos0, sin0, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotateRef
\*****************************************************************************/
static int imluaProcessRotateRef (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);
  int x = luaL_checkint(L, 5);
  int y = luaL_checkint(L, 6);
  int to_origin = lua_toboolean(L, 7);
  int order = luaL_checkint(L, 8);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 5, "invalid order, must be 0, 1, or 3");

  lua_pushboolean(L, imProcessRotateRef(src_image, dst_image, cos0, sin0, x, y, to_origin, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotate90
\*****************************************************************************/
static int imluaProcessRotate90 (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int dir = lua_toboolean(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, dst_image->width == src_image->height && dst_image->height == src_image->width, 2, "destiny width and height must have the source height and width");
  luaL_argcheck(L, (dir == -1 || dir == 1), 3, "invalid dir, can be -1 or 1 only");

  imProcessRotate90(src_image, dst_image, dir);
  return 0;
}

/*****************************************************************************\
 im.ProcessRotate180
\*****************************************************************************/
static int imluaProcessRotate180 (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessRotate180(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessMirror
\*****************************************************************************/
static int imluaProcessMirror (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessMirror(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessFlip
\*****************************************************************************/
static int imluaProcessFlip (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessFlip(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessInterlaceSplit
\*****************************************************************************/
static int imluaProcessInterlaceSplit (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image1 = imlua_checkimage(L, 2);
  imImage *dst_image2 = imlua_checkimage(L, 3);

  imlua_matchcolor(L, src_image, dst_image1);
  imlua_matchcolor(L, src_image, dst_image2);
  luaL_argcheck(L, dst_image1->width == src_image->width && dst_image2->width == src_image->width, 2, "destiny width must be equal to source width");

  if (src_image->height%2)
  {
    int dst_height1 = src_image->height/2 + 1;
    luaL_argcheck(L, dst_image1->height == dst_height1, 2, "destiny1 height must be equal to source height/2+1 if height odd");
  }
  else
    luaL_argcheck(L, dst_image1->height == src_image->height/2, 2, "destiny1 height must be equal to source height/2 if height even");

  luaL_argcheck(L, dst_image2->height == src_image->height/2, 2, "destiny2 height must be equal to source height/2");

  imProcessInterlaceSplit(src_image, dst_image1, dst_image2);
  return 0;
}

/*****************************************************************************\
 im.ProcessRadial
\*****************************************************************************/
static int imluaProcessRadial (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float k1 = (float) luaL_checknumber(L, 3);
  int order = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 4, "invalid order");

  lua_pushboolean(L, imProcessRadial(src_image, dst_image, k1, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessSwirl
\*****************************************************************************/
static int imluaProcessSwirl(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float k1 = (float) luaL_checknumber(L, 3);
  int order = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 4, "invalid order, can be 0, 1 or 3");

  lua_pushboolean(L, imProcessSwirl(src_image, dst_image, k1, order));
  return 1;
}


/*****************************************************************************\
 Morphology Operations for Gray Images
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessGrayMorphConvolve
\*****************************************************************************/
static int imluaProcessGrayMorphConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int ismax = lua_toboolean(L, 4);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, kernel, IM_INT);
  imlua_matchsize(L, src_image, kernel);

  lua_pushboolean(L, imProcessGrayMorphConvolve(src_image, dst_image, kernel, ismax));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphErode
\*****************************************************************************/
static int imluaProcessGrayMorphErode (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphErode(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphDilate
\*****************************************************************************/
static int imluaProcessGrayMorphDilate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphDilate(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphOpen
\*****************************************************************************/
static int imluaProcessGrayMorphOpen (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphOpen(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphClose
\*****************************************************************************/
static int imluaProcessGrayMorphClose (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphClose(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphTopHat
\*****************************************************************************/
static int imluaProcessGrayMorphTopHat (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphTopHat(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphWell
\*****************************************************************************/
static int imluaProcessGrayMorphWell (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphWell(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphGradient
\*****************************************************************************/
static int imluaProcessGrayMorphGradient (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphGradient(src_image, dst_image, kernel_size));
  return 1;
}



/*****************************************************************************\
 Morphology Operations for Binary Images
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessBinMorphConvolve
\*****************************************************************************/
static int imluaProcessBinMorphConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int hit_white = lua_toboolean(L, 4);
  int iter = luaL_checkint(L, 5);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, kernel, IM_INT);
  imlua_matchsize(L, src_image, kernel);

  lua_pushboolean(L, imProcessBinMorphConvolve(src_image, dst_image, kernel, hit_white, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphErode
\*****************************************************************************/
static int imluaProcessBinMorphErode (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphErode(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphDilate
\*****************************************************************************/
static int imluaProcessBinMorphDilate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphDilate(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphOpen
\*****************************************************************************/
static int imluaProcessBinMorphOpen (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphOpen(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphClose
\*****************************************************************************/
static int imluaProcessBinMorphClose (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphClose(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphOutline
\*****************************************************************************/
static int imluaProcessBinMorphOutline (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphOutline(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphThin
\*****************************************************************************/
static int imluaProcessBinMorphThin (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  imProcessBinMorphThin(src_image, dst_image);
  return 0;
}



/*****************************************************************************\
 Rank Convolution Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessMedianConvolve
\*****************************************************************************/
static int imluaProcessMedianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessMedianConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRangeConvolve
\*****************************************************************************/
static int imluaProcessRangeConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRangeConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankClosestConvolve
\*****************************************************************************/
static int imluaProcessRankClosestConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankClosestConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankMaxConvolve
\*****************************************************************************/
static int imluaProcessRankMaxConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankMaxConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankMinConvolve
\*****************************************************************************/
static int imluaProcessRankMinConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankMinConvolve(src_image, dst_image, kernel_size));
  return 1;
}


/*****************************************************************************\
 Convolution Operations
\*****************************************************************************/

static void imlua_checkkernel(lua_State *L, imImage* kernel, int index)
{
  imlua_checkcolorspace(L, index, kernel, IM_GRAY);
  luaL_argcheck(L, kernel->data_type == IM_INT || kernel->data_type == IM_FLOAT, index, "kernel data type can be int or float only");
}

/*****************************************************************************\
 im.ProcessConvolve
\*****************************************************************************/
static int imluaProcessConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolve(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveDual
\*****************************************************************************/
static int imluaProcessConvolveDual (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel1 = imlua_checkimage(L, 3);
  imImage *kernel2 = imlua_checkimage(L, 4);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel1, 3);
  imlua_checkkernel(L, kernel2, 4);

  lua_pushboolean(L, imProcessConvolveDual(src_image, dst_image, kernel1, kernel2));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveRep
\*****************************************************************************/
static int imluaProcessConvolveRep (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int count = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolveRep(src_image, dst_image, kernel, count));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveSep
\*****************************************************************************/
static int imluaProcessConvolveSep (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolveSep(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessCompassConvolve
\*****************************************************************************/
static int imluaProcessCompassConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessCompassConvolve(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotateKernel
\*****************************************************************************/
static int imluaProcessRotateKernel (lua_State *L)
{
  imProcessRotateKernel(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 im.ProcessDiffOfGaussianConvolve
\*****************************************************************************/
static int imluaProcessDiffOfGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev1 = (float) luaL_checknumber(L, 3);
  float stddev2 = (float) luaL_checknumber(L, 4);

  if (src_image->data_type == IM_BYTE || src_image->data_type == IM_USHORT)
  {
    imlua_matchcolor(L, src_image, dst_image);
    imlua_checkdatatype(L, 2, dst_image, IM_INT);
  }
  else
    imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessDiffOfGaussianConvolve(src_image, dst_image, stddev1, stddev2));
  return 1;
}

/*****************************************************************************\
 im.ProcessLapOfGaussianConvolve
\*****************************************************************************/
static int imluaProcessLapOfGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  if (src_image->data_type == IM_BYTE || src_image->data_type == IM_USHORT)
  {
    imlua_matchcolor(L, src_image, dst_image);
    imlua_checkdatatype(L, 2, dst_image, IM_INT);
  }
  else
    imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessLapOfGaussianConvolve(src_image, dst_image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessMeanConvolve
\*****************************************************************************/
static int imluaProcessMeanConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessMeanConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessBarlettConvolve
\*****************************************************************************/
static int imluaProcessBarlettConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBarlettConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGaussianConvolve
\*****************************************************************************/
static int imluaProcessGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGaussianConvolve(src_image, dst_image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessPrewittConvolve
\*****************************************************************************/
static int imluaProcessPrewittConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessPrewittConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessSplineEdgeConvolve
\*****************************************************************************/
static int imluaProcessSplineEdgeConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessSplineEdgeConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessSobelConvolve
\*****************************************************************************/
static int imluaProcessSobelConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessSobelConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessZeroCrossing
\*****************************************************************************/
static int imluaProcessZeroCrossing (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, src_image->data_type == IM_INT || src_image->data_type == IM_FLOAT, 1, "image data type can be int or float only");
  imlua_match(L, src_image, dst_image);

  imProcessZeroCrossing(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCanny
\*****************************************************************************/
static int imluaProcessCanny (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessCanny(src_image, dst_image, stddev);
  return 0;
}

/*****************************************************************************\
 im.ProcessUnsharp
\*****************************************************************************/
static int imluaProcessUnsharp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float p1 = (float)luaL_checknumber(L, 3);
  float p2 = (float)luaL_checknumber(L, 4);
  float p3 = (float)luaL_checknumber(L, 5);

  imlua_match(L, src_image, dst_image);

  imProcessUnsharp(src_image, dst_image, p1, p2, p3);
  return 0;
}

/*****************************************************************************\
 im.ProcessSharp
\*****************************************************************************/
static int imluaProcessSharp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float p1 = (float)luaL_checknumber(L, 3);
  float p2 = (float)luaL_checknumber(L, 4);

  imlua_match(L, src_image, dst_image);

  imProcessSharp(src_image, dst_image, p1, p2);
  return 0;
}

/*****************************************************************************\
 im.ProcessSharpKernel
\*****************************************************************************/
static int imluaProcessSharpKernel(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *kernel = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  float p1 = (float)luaL_checknumber(L, 4);
  float p2 = (float)luaL_checknumber(L, 5);

  imlua_match(L, src_image, dst_image);

  imProcessSharpKernel(src_image, kernel, dst_image, p1, p2);
  return 0;
}

/*****************************************************************************\
 im.GaussianStdDev2Repetitions
\*****************************************************************************/
static int imluaGaussianKernelSize2StdDev(lua_State *L)
{
  lua_pushnumber(L, imGaussianKernelSize2StdDev((int)luaL_checknumber(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.GaussianStdDev2KernelSize
\*****************************************************************************/
static int imluaGaussianStdDev2KernelSize (lua_State *L)
{
  lua_pushnumber(L, imGaussianStdDev2KernelSize((float)luaL_checknumber(L, 1)));
  return 1;
}



/*****************************************************************************\
 Arithmetic Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessUnArithmeticOp
\*****************************************************************************/
static int imluaProcessUnArithmeticOp (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int op = luaL_checkint(L, 3);

  imlua_matchcolorspace(L, src_image, dst_image);

  //TODO

  imProcessUnArithmeticOp(src_image, dst_image, op);
  return 0;
}

static int (imluaUnOpFunc)(float src_value, float *dst_value, float* params, void* userdata, int x, int y, int d)
{
  int n, ret = 0;
  lua_State *L = userdata;

  lua_pushvalue(L, 3);  /* func is passed in the stack */
  lua_pushnumber(L, src_value);
  n = imlua_unpacktable(L, 4);  /* params is passed in the stack */
  lua_pushvalue(L, 5);  /* userdata is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, d);

  lua_call(L, 1+n+4, 1);

  if (!lua_isnil(L, -1))
  {
    *dst_value = (float)luaL_checknumber(L, -1);
    ret = 1;
  }
  lua_pop(L, 1);

  (void)params;
  return ret;
}

static int imluaProcessUnaryPointOp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  const char *op_name = luaL_optstring(L, 6, NULL);

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  imlua_checknotcfloat(L, 1, src_image);
  imlua_checknotcfloat(L, 1, dst_image);
  imlua_matchsize(L, src_image, dst_image);
  if (src_image->depth != dst_image->depth)
    luaL_error(L, "images must have the same depth");
  luaL_checktype(L, 3, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);
  /* no need to check the userdata at 5 */

  lua_pushboolean(L, imProcessUnaryPointOp(src_image, dst_image, imluaUnOpFunc, NULL, L, op_name));

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

static int imluaUnColorOpFunc(const float* src_value, float* dst_value, float* params, void* userdata, int x, int y)
{
  int d, n, ret = 0;
  lua_State *L = userdata;
  int src_depth = (int)params[0];
  int dst_depth = (int)params[1];

  lua_pushvalue(L, 3);  /* func is passed in the stack */
  for (d = 0; d < src_depth; d++)
    lua_pushnumber(L, src_value[d]);
  n = imlua_unpacktable(L, 4);  /* params is passed in the stack */
  lua_pushvalue(L, 5);  /* userdata is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);

  lua_call(L, src_depth+n+3, dst_depth);

  if (!lua_isnil(L, -dst_depth))
  {
    for (d = 0; d < dst_depth; d++)
      dst_value[d] = (float)luaL_checknumber(L, d-dst_depth);
    ret = 1;
  }
  lua_pop(L, dst_depth);

  return ret;
}

static int imluaProcessUnaryPointColorOp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  const char *op_name = luaL_optstring(L, 6, NULL);
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  int dst_depth = dst_image->has_alpha? dst_image->depth+1: dst_image->depth;
  float params[2];

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  params[0] = (float)src_depth;
  params[1] = (float)dst_depth;

  imlua_checknotcfloat(L, 1, src_image);
  imlua_checknotcfloat(L, 1, dst_image);
  imlua_matchsize(L, src_image, dst_image);
  luaL_checktype(L, 3, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);
  /* no need to check the userdata at 5 */

  lua_pushboolean(L, imProcessUnaryPointColorOp(src_image, dst_image, imluaUnColorOpFunc, params, L, op_name));

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

static int imluaMultiOpFunc(const float* src_value, float *dst_value, float* params, void* userdata, int x, int y, int d)
{
  lua_State *L = userdata;
  int ret = 0, n, i, 
    src_count = (int)params[0];

  lua_pushvalue(L, 3);  /* func is passed in the stack */
  for (i = 0; i < src_count; i++)
    lua_pushnumber(L, src_value[i]);
  n = imlua_unpacktable(L, 4);  /* params is passed in the stack */
  lua_pushvalue(L, 5);  /* userdata is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, d);

  lua_call(L, src_count+n+4, 1);

  if (!lua_isnil(L, -1))
  {
    *dst_value = (float)luaL_checknumber(L, -1);
    ret = 1;
  }
  lua_pop(L, 1);

  return ret;
}

static int imluaProcessMultiPointOp(lua_State *L)
{
  int src_count;
  imImage **src_image_list;
  imImage *dst_image = imlua_checkimage(L, 2);
  const char *op_name = luaL_optstring(L, 6, NULL);
  float params[1];

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  imlua_checknotcfloat(L, 1, dst_image);
  luaL_checktype(L, 3, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);
  /* no need to check the userdata at 5 */

  /* minimize leak when error, checking array after other checks */
  src_image_list = imlua_toarrayimage(L, 1, &src_count, 1);
  if (src_image_list[0]->data_type == IM_CFLOAT)
  {
    free(src_image_list);
    imlua_errorcfloat(L, 1);
  }

  if (!imImageMatchSize(src_image_list[0], dst_image))
  {
    free(src_image_list);
    imlua_errormatchsize(L);  
  }

  if (src_image_list[0]->depth != dst_image->depth)
  {
    free(src_image_list);
    luaL_error(L, "source and destiny images must have the same depth");
  }

  params[0] = (float)src_count;

  lua_pushboolean(L, imProcessMultiPointOp((const imImage**)src_image_list, src_count, dst_image, imluaMultiOpFunc, params, L, op_name));

  free(src_image_list);

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

static int imluaMultiColorOpFunc(float* src_value, float *dst_value, float* params, void* userdata, int x, int y)
{
  lua_State *L = userdata;
  int n, d, m, i, ret = 0, 
    src_count = (int)params[0],
    src_depth = (int)params[1],
    dst_depth = (int)params[2];

  lua_pushvalue(L, 3);  /* func is passed in the stack */
  m = src_depth*src_count;
  for (i = 0; i < m; i++)
    lua_pushnumber(L, src_value[i]);
  n = imlua_unpacktable(L, 4);  /* params is passed in the stack */
  lua_pushvalue(L, 5);  /* userdata is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);

  lua_call(L, m+n+3, dst_depth);

  if (!lua_isnil(L, -dst_depth))
  {
    for (d = 0; d < dst_depth; d++)
      dst_value[d] = (float)luaL_checknumber(L, d-dst_depth);
    ret = 1;
  }
  lua_pop(L, dst_depth);

  return ret;
}

static int imluaProcessMultiPointColorOp(lua_State *L)
{
  int src_count, src_depth, dst_depth;
  imImage **src_image_list;
  imImage *dst_image = imlua_checkimage(L, 2);
  const char *op_name = luaL_optstring(L, 6, NULL);
  float params[3];

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  imlua_checknotcfloat(L, 1, dst_image);
  luaL_checktype(L, 3, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);
  /* no need to check the userdata at 5 */

  /* minimize leak when error, checking array after other checks */
  src_image_list = imlua_toarrayimage(L, 1, &src_count, 1);
  if (src_image_list[0]->data_type == IM_CFLOAT)
  {
    free(src_image_list);
    imlua_errorcfloat(L, 1);
  }

  if (!imImageMatchSize(src_image_list[0], dst_image))
  {
    free(src_image_list);
    imlua_errormatchsize(L);  
  }

  src_depth = src_image_list[0]->has_alpha? src_image_list[0]->depth+1: src_image_list[0]->depth;
  dst_depth = dst_image->has_alpha? dst_image->depth+1: dst_image->depth;

  params[0] = (float)src_count;
  params[1] = (float)src_depth;
  params[2] = (float)dst_depth;

  lua_pushboolean(L, imProcessMultiPointColorOp((const imImage**)src_image_list, src_count, dst_image, imluaMultiColorOpFunc, params, L, op_name));

  free(src_image_list);

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

/*****************************************************************************\
 im.ProcessArithmeticOp
\*****************************************************************************/
static int imluaProcessArithmeticOp (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_matchsize(L, src_image1, dst_image);
  imlua_matchsize(L, src_image2, dst_image);

  switch (src_image1->data_type)
  {
  case IM_BYTE:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is byte, destiny image data type can be byte, short, ushort, int and float only.");
    break;
  case IM_SHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is short, destiny image data type can be short, ushort, int and float only.");
    break;
  case IM_USHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is ushort, destiny image data type can be short, ushort, int and float only.");
    break;
  case IM_INT:
    luaL_argcheck(L,
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is int, destiny image data type can be int and float only.");
    break;
  case IM_FLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_FLOAT,
      2, "source image is float, destiny image data type can be float only.");
    break;
  case IM_CFLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_CFLOAT,
      2, "source image is cfloat, destiny image data type can be cfloat only.");
    break;
  }

  imProcessArithmeticOp(src_image1, src_image2, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessArithmeticConstOp
\*****************************************************************************/
static int imluaProcessArithmeticConstOp (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  float src_const = (float) luaL_checknumber(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_matchsize(L, src_image, dst_image);

  switch (src_image->data_type)
  {
  case IM_BYTE:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is byte, destiny image data type can be byte, short, ushort, int and float only.");
    break;
  case IM_SHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is short, destiny image data type can be byte, short, ushort, int and float only.");
    break;
  case IM_USHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is ushort, destiny image data type can be byte, short, ushort, int and float only.");
    break;
  case IM_INT:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_SHORT ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is int, destiny image data type can be byte, short, ushort, int and float only.");
    break;
  case IM_FLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_FLOAT,
      2, "source image is float, destiny image data type can be float only.");
    break;
  case IM_CFLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_CFLOAT,
      2, "source image is cfloat, destiny image data type can be cfloat only.");
    break;
  }

  imProcessArithmeticConstOp(src_image, src_const, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBlendConst
\*****************************************************************************/
static int imluaProcessBlendConst (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  float alpha = (float) luaL_checknumber(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessBlendConst(src_image1, src_image2, dst_image, alpha);
  return 0;
}

/*****************************************************************************\
 im.ProcessBlend
\*****************************************************************************/
static int imluaProcessBlend (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *alpha_image = imlua_checkimage(L, 3);
  imImage *dst_image = imlua_checkimage(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);
  imlua_matchdatatype(L, src_image1, alpha_image);

  imProcessBlend(src_image1, src_image2, alpha_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCompose
\*****************************************************************************/
static int imluaProcessCompose(lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessCompose(src_image1, src_image2, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitComplex
\*****************************************************************************/
static int imluaProcessSplitComplex (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image1 = imlua_checkimage(L, 2);
  imImage *dst_image2 = imlua_checkimage(L, 3);
  int polar = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_CFLOAT);
  imlua_checkdatatype(L, 2, dst_image1, IM_FLOAT);
  imlua_checkdatatype(L, 3, dst_image2, IM_FLOAT);
  imlua_matchcolorspace(L, src_image, dst_image1);
  imlua_matchcolorspace(L, src_image, dst_image2);

  imProcessSplitComplex(src_image, dst_image1, dst_image2, polar);
  return 0;
}

/*****************************************************************************\
 im.ProcessMergeComplex
\*****************************************************************************/
static int imluaProcessMergeComplex (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int polar = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image1, IM_FLOAT);
  imlua_checkdatatype(L, 2, src_image2, IM_FLOAT);
  imlua_checkdatatype(L, 3, dst_image, IM_CFLOAT);
  imlua_matchcolorspace(L, src_image1, src_image2);
  imlua_matchcolorspace(L, src_image1, dst_image);

  imProcessMergeComplex(src_image1, src_image2, dst_image, polar);
  return 0;
}

/*****************************************************************************\
 im.ProcessMultipleMean
\*****************************************************************************/
static int imluaProcessMultipleMean (lua_State *L)
{
  int src_image_count;
  imImage* *src_image_list;
  imImage* dst_image = imlua_checkimage(L, 2);

  /* minimize leak when error, checking array after other checks */
  src_image_list = imlua_toarrayimage(L, 1, &src_image_count, 1);

  if (!imImageMatch(src_image_list[0], dst_image))
  {
    free(src_image_list);
    imlua_errormatch(L);
  }

  imProcessMultipleMean((const imImage**)src_image_list, src_image_count, dst_image);

  free(src_image_list);
  return 0;
}

/*****************************************************************************\
 im.ProcessMultipleStdDev
\*****************************************************************************/
static int imluaProcessMultipleStdDev (lua_State *L)
{
  int src_image_count;
  imImage* *src_image_list;
  imImage* mean_image = imlua_checkimage(L, 2);
  imImage* dst_image = imlua_checkimage(L, 3);

  /* minimize leak when error, checking array after other checks */
  src_image_list = imlua_toarrayimage(L, 1, &src_image_count, 1);
  
  if (!imImageMatch(src_image_list[0], dst_image) ||
      !imImageMatch(mean_image, dst_image))
  {
    free(src_image_list);
    imlua_errormatch(L);
  }

  imProcessMultipleStdDev((const imImage**)src_image_list, src_image_count, mean_image, dst_image);

  free(src_image_list);
  return 0;
}

/*****************************************************************************\
 im.ProcessAutoCovariance
\*****************************************************************************/
static int imluaProcessAutoCovariance (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *mean_image = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image, mean_image);
  imlua_matchcolorspace(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, dst_image, IM_FLOAT);

  lua_pushboolean(L, imProcessAutoCovariance(src_image, mean_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessMultiplyConj
\*****************************************************************************/
static int imluaProcessMultiplyConj (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessMultiplyConj(src_image1, src_image2, dst_image);
  return 0;
}


/*****************************************************************************\
 Additional Image Quantization Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessQuantizeRGBUniform
\*****************************************************************************/
static int imluaProcessQuantizeRGBUniform (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int dither = lua_toboolean(L, 3);

  imlua_checktype(L, 1, src_image, IM_RGB, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_MAP);
  imlua_matchsize(L, src_image, dst_image);

  imProcessQuantizeRGBUniform(src_image, dst_image, dither);
  return 0;
}

/*****************************************************************************\
 im.ProcessQuantizeGrayUniform
\*****************************************************************************/
static int imluaProcessQuantizeGrayUniform (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int grays = luaL_checkint(L, 3);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessQuantizeGrayUniform(src_image, dst_image, grays);
  return 0;
}


/*****************************************************************************\
 Histogram Based Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessExpandHistogram
\*****************************************************************************/
static int imluaProcessExpandHistogram (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checkhistogramtype(L, 1, src_image);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, src_image->color_space == IM_RGB || src_image->color_space == IM_GRAY, 1, "color space can be RGB or Gray only");
  luaL_argcheck(L, dst_image->color_space == IM_RGB || dst_image->color_space == IM_GRAY, 2, "color space can be RGB or Gray only");

  imProcessExpandHistogram(src_image, dst_image, percent);
  return 0;
}

/*****************************************************************************\
 im.ProcessEqualizeHistogram
\*****************************************************************************/
static int imluaProcessEqualizeHistogram (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkhistogramtype(L, 1, src_image);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, src_image->color_space == IM_RGB || src_image->color_space == IM_GRAY, 1, "color space can be RGB or Gray only");
  luaL_argcheck(L, dst_image->color_space == IM_RGB || dst_image->color_space == IM_GRAY, 2, "color space can be RGB or Gray only");

  imProcessEqualizeHistogram(src_image, dst_image);
  return 0;
}



/*****************************************************************************\
 Color Processing Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessSplitYChroma
\*****************************************************************************/
static int imluaProcessSplitYChroma (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *y_image = imlua_checkimage(L, 2);
  imImage *chroma_image = imlua_checkimage(L, 3);

  imlua_checktype(L, 1, src_image, IM_RGB, IM_BYTE);
  imlua_checktype(L, 2, y_image, IM_GRAY, IM_BYTE);
  imlua_checktype(L, 3, chroma_image, IM_RGB, IM_BYTE);
  imlua_matchsize(L, src_image, y_image);
  imlua_matchsize(L, src_image, chroma_image);

  imProcessSplitYChroma(src_image, y_image, chroma_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitHSI
\*****************************************************************************/
static int imluaProcessSplitHSI (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *h_image = imlua_checkimage(L, 2);
  imImage *s_image = imlua_checkimage(L, 3);
  imImage *i_image = imlua_checkimage(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_RGB);
  luaL_argcheck(L, src_image->data_type == IM_BYTE || src_image->data_type == IM_FLOAT, 1, "data type can be float or byte only");
  imlua_checktype(L, 2, h_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 3, s_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 4, i_image, IM_GRAY, IM_FLOAT);
  imlua_matchsize(L, src_image, h_image);
  imlua_matchsize(L, src_image, s_image);
  imlua_matchsize(L, src_image, i_image);

  imProcessSplitHSI(src_image, h_image, s_image, i_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessMergeHSI
\*****************************************************************************/
static int imluaProcessMergeHSI (lua_State *L)
{
  imImage *h_image = imlua_checkimage(L, 1);
  imImage *s_image = imlua_checkimage(L, 2);
  imImage *i_image = imlua_checkimage(L, 3);
  imImage *dst_image = imlua_checkimage(L, 4);

  imlua_checktype(L, 1, h_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 2, s_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 3, i_image, IM_GRAY, IM_FLOAT);
  imlua_checkcolorspace(L, 4, dst_image, IM_RGB);
  luaL_argcheck(L, dst_image->data_type == IM_BYTE || dst_image->data_type == IM_FLOAT, 4, "data type can be float or byte only");
  imlua_matchsize(L, dst_image, h_image);
  imlua_matchsize(L, dst_image, s_image);
  imlua_matchsize(L, dst_image, i_image);

  imProcessMergeHSI(h_image, s_image, i_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitComponents(src_image, { r, g, b} )
\*****************************************************************************/
static int imluaProcessSplitComponents (lua_State *L)
{
  int i, src_depth, dst_count;
  imImage **dst_image_list;
  imImage *src_image = imlua_checkimage(L, 1);

  /* minimize leak when error, checking array after other checks */
  dst_image_list = imlua_toarrayimage(L, 2, &dst_count, 1);

  src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  if (dst_count != src_depth)
  {
    free(dst_image_list);
    luaL_error(L, "number of destiny images must match the depth of the source image");
  }

  for (i = 0; i < src_depth; i++)
  {
    if (dst_image_list[i]->color_space != IM_GRAY)
    {
      free(dst_image_list);
      imlua_argerrorcolorspace(L, 2, IM_GRAY);
    }
  }

  if (!imImageMatchDataType(src_image, dst_image_list[0]))
  {
    free(dst_image_list);
    imlua_errormatchdatatype(L);
  }

  imProcessSplitComponents(src_image, dst_image_list);

  free(dst_image_list);

  return 0;
}

/*****************************************************************************\
 im.ProcessMergeComponents({r, g, b}, rgb)
\*****************************************************************************/
static int imluaProcessMergeComponents (lua_State *L)
{
  int i, dst_depth, src_count;
  imImage **src_image_list;
  imImage *dst_image = imlua_checkimage(L, 2);

  dst_depth = dst_image->has_alpha? dst_image->depth+1: dst_image->depth;

  /* minimize leak when error, checking array after other checks */
  src_image_list = imlua_toarrayimage(L, 1, &src_count, 1);
  if (src_count != dst_depth)
  {
    free(src_image_list);
    luaL_error(L, "number of source images must match the depth of the destination image");
  }

  for (i = 0; i < dst_depth; i++)
  {
    if (src_image_list[i]->color_space != IM_GRAY)
    {
      free(src_image_list);
      imlua_argerrorcolorspace(L, 2, IM_GRAY);
    }
  }

  if (!imImageMatchDataType(src_image_list[0], dst_image))
  {
    free(src_image_list);
    imlua_errormatchdatatype(L);
  }

  imProcessMergeComponents((const imImage**)src_image_list, dst_image);

  free(src_image_list);

  return 0;
}

/*****************************************************************************\
 im.ProcessNormalizeComponents
\*****************************************************************************/
static int imluaProcessNormalizeComponents (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkdatatype(L, 2, dst_image, IM_FLOAT);
  imlua_matchcolorspace(L, src_image, dst_image);

  imProcessNormalizeComponents(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessReplaceColor
\*****************************************************************************/
static int imluaProcessReplaceColor (lua_State *L)
{
  int src_count, dst_count;
  float *src_color, *dst_color;
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  /* minimize leak when error, checking array after other checks */
  src_color = imlua_toarrayfloat(L, 3, &src_count, 1);
  if (src_count != src_image->depth)
  {
    free(src_color);
    luaL_argerror(L, 3, "the colors must have the same number of components of the images");
  }

  dst_color = imlua_toarrayfloat(L, 4, &dst_count, 1);
  if (dst_count != src_image->depth)
  {
    free(src_color);
    free(dst_color);
    luaL_argerror(L, 4, "the colors must have the same number of components of the images");
  }

  imProcessReplaceColor(src_image, dst_image, src_color, dst_color);

  free(src_color);
  free(dst_color);
  return 0;
}

static int imluaProcessSetAlphaColor(lua_State *L)
{
  int src_count;
  float *src_color;
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float dst_alpha = (float)luaL_checknumber(L, 4);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_checknotcfloat(L, 2, dst_image);
  imlua_matchsize(L, src_image, dst_image);

  /* minimize leak when error, checking array after other checks */
  src_color = imlua_toarrayfloat(L, 3, &src_count, 1);
  if (src_count != src_image->depth)
  {
    free(src_color);
    luaL_argerror(L, 3, "the color must have the same number of components of the source image");
  }

  imProcessSetAlphaColor(src_image, dst_image, src_color, dst_alpha);

  free(src_color);
  return 0;
}


/*****************************************************************************\
 Logical Arithmetic Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessBitwiseOp
\*****************************************************************************/
static int imluaProcessBitwiseOp (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  luaL_argcheck(L, (src_image1->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessBitwiseOp(src_image1, src_image2, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitwiseNot
\*****************************************************************************/
static int imluaProcessBitwiseNot (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image, dst_image);

  imProcessBitwiseNot(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitMask(src_image, dst_image, mask, op)
\*****************************************************************************/
static int imluaProcessBitMask (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  unsigned char mask = imlua_checkmask(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessBitMask(src_image, dst_image, mask, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitPlane(src_image, dst_image, plane, reset)
\*****************************************************************************/
static int imluaProcessBitPlane (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int plane = luaL_checkint(L, 3);
  int reset = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessBitPlane(src_image, dst_image, plane, reset);
  return 0;
}



/*****************************************************************************\
 Synthetic Image Render
\*****************************************************************************/

static float imluaRenderFunc (int x, int y, int d, float *params)
{
  float ret;
  lua_State *L = g_State;

  lua_pushvalue(L, 2);  /* func is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, d);
  lua_pushvalue(L, 4);  /* params is passed in the stack */

  lua_call(L, 4, 1);

  ret = (float) luaL_checknumber(L, -1);
  lua_pop(L, 1);

  (void)params; 
  return ret;
}

/*****************************************************************************\
 im.ProcessRenderOp(image, function, name, param, plus)
\*****************************************************************************/
static int imluaProcessRenderOp (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  const char *render_name = luaL_checkstring(L, 3);
  int plus = luaL_checkint(L, 5);

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  imlua_checknotcfloat(L, 1, image);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);

  g_State = L;
  lua_pushboolean(L, imProcessRenderOp(image, imluaRenderFunc, (char*) render_name, NULL, plus));
  g_State = NULL;

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

static float imluaRenderCondFunc (int x, int y, int d, int *cond, float *params)
{
  float ret;
  lua_State *L = g_State;

  lua_pushvalue(L, 2);  /* func is passed in the stack */
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, d);
  lua_pushvalue(L, 4);  /* params is passed in the stack */

  lua_call(L, 4, 2);

  *cond = lua_toboolean(L, -1);
  ret = (float) luaL_checknumber(L, -2);
  lua_pop(L, 2);

  (void)params;
  return ret;
}

/*****************************************************************************\
 im.ProcessRenderCondOp(image, function, name, param)
\*****************************************************************************/
static int imluaProcessRenderCondOp (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  const char *render_name = luaL_checkstring(L, 3);

#ifdef _OPENMP
  int old_num_threads = omp_get_num_threads();
  omp_set_num_threads(1);
#endif

  imlua_checknotcfloat(L, 1, image);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  luaL_checktype(L, 4, LUA_TTABLE);

  g_State = L;
  lua_pushboolean(L, imProcessRenderCondOp(image, imluaRenderCondFunc, (char*) render_name, NULL));
  g_State = NULL;

#ifdef _OPENMP
  omp_set_num_threads(old_num_threads);
#endif

  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddSpeckleNoise
\*****************************************************************************/
static int imluaProcessRenderAddSpeckleNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddSpeckleNoise(src_image, dst_image, percent));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddGaussianNoise
\*****************************************************************************/
static int imluaProcessRenderAddGaussianNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float mean = (float) luaL_checknumber(L, 3);
  float stddev = (float) luaL_checknumber(L, 4);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddGaussianNoise(src_image, dst_image, mean, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddUniformNoise
\*****************************************************************************/
static int imluaProcessRenderAddUniformNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float mean = (float) luaL_checknumber(L, 3);
  float stddev = (float) luaL_checknumber(L, 4);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddUniformNoise(src_image, dst_image, mean, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderRandomNoise
\*****************************************************************************/
static int imluaProcessRenderRandomNoise (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  imlua_checknotcfloat(L, 1, image);
  lua_pushboolean(L, imProcessRenderRandomNoise(image));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderConstant(image, value)
\*****************************************************************************/
static int imluaProcessRenderConstant (lua_State *L)
{
  int count;
  float *value;
  imImage *image = imlua_checkimage(L, 1);
  imlua_checknotcfloat(L, 1, image);

  /* minimize leak when error, checking array after other checks */
  value = imlua_toarrayfloat (L, 2, &count, 1);
  if (count != image->depth)
  {
    free(value);
    luaL_argerror(L, 2, "invalid number of planes");
  }

  lua_pushboolean(L, imProcessRenderConstant(image, value));

  free(value);
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderWheel
\*****************************************************************************/
static int imluaProcessRenderWheel (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int int_radius = luaL_checkint(L, 2);
  int ext_radius = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderWheel(image, int_radius, ext_radius));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderCone
\*****************************************************************************/
static int imluaProcessRenderCone (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int radius = luaL_checkint(L, 2);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderCone(image, radius));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderTent
\*****************************************************************************/
static int imluaProcessRenderTent (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderTent(image, width, height));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderRamp
\*****************************************************************************/
static int imluaProcessRenderRamp (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int start = luaL_checkint(L, 2);
  int end = luaL_checkint(L, 3);
  int dir = luaL_checkint(L, 4);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderRamp(image, start, end, dir));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderBox
\*****************************************************************************/
static int imluaProcessRenderBox (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderBox(image, width, height));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderSinc
\*****************************************************************************/
static int imluaProcessRenderSinc (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float xperiod = (float) luaL_checknumber(L, 2);
  float yperiod = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderSinc(image, xperiod, yperiod));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderGaussian
\*****************************************************************************/
static int imluaProcessRenderGaussian (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float stddev = (float) luaL_checknumber(L, 2);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderGaussian(image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderLapOfGaussian
\*****************************************************************************/
static int imluaProcessRenderLapOfGaussian (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float stddev = (float) luaL_checknumber(L, 2);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderLapOfGaussian(image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderCosine
\*****************************************************************************/
static int imluaProcessRenderCosine (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float xperiod = (float) luaL_checknumber(L, 2);
  float yperiod = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderCosine(image, xperiod, yperiod));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderGrid
\*****************************************************************************/
static int imluaProcessRenderGrid (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int x_space = luaL_checkint(L, 2);
  int y_space = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderGrid(image, x_space, y_space));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderChessboard
\*****************************************************************************/
static int imluaProcessRenderChessboard (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int x_space = luaL_checkint(L, 2);
  int y_space = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, 1, image);

  lua_pushboolean(L, imProcessRenderChessboard(image, x_space, y_space));
  return 1;
}



/*****************************************************************************\
 Tone Gamut Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessToneGamut
\*****************************************************************************/
static int imluaProcessToneGamut (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int op = luaL_checkint(L, 3);
  float *param = NULL;

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  /* minimize leak when error, checking array after other checks */
  param = imlua_toarrayfloatopt(L, 4, NULL, 1);

  imProcessToneGamut(src_image, dst_image, op, param);

  if (param)
    free(param);

  return 0;
}

static int imluaImageGamma(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float gamma = (float)luaL_checknumber(L, 2);
  imlua_checknotcfloat(L, 1, image);
  imImageGamma(image, gamma);
  return 0;
}

static int imluaImageBrightnessContrast(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float bright_shift = (float)luaL_checknumber(L, 2);
  float contrast_factor = (float)luaL_checknumber(L, 3);
  imlua_checknotcfloat(L, 1, image);
  imImageBrightnessContrast(image, bright_shift, contrast_factor);
  return 0;
}

static int imluaImageLevel(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float start = (float)luaL_checknumber(L, 2);
  float end = (float)luaL_checknumber(L, 3);
  imlua_checknotcfloat(L, 1, image);
  imImageLevel(image, start, end);
  return 0;
}

static int imluaImageNegative(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  imlua_checknotcfloat(L, 1, image);
  imImageNegative(image);
  return 0;
}

static int imluaImageEqualize(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  imlua_checkhistogramtype(L, 1, image);
  if (image->color_space != IM_RGB && image->color_space != IM_GRAY)
    luaL_argerror(L, 1, "color space must be RGB or Gray");
  imImageEqualize(image);
  return 0;
}

static int imluaImageAutoLevel(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float percent = (float)luaL_optnumber(L, 2, 0);
  imlua_checkhistogramtype(L, 1, image);
  if (image->color_space != IM_RGB && image->color_space != IM_GRAY)
    luaL_argerror(L, 1, "color space must be RGB or Gray");
  imImageAutoLevel(image, percent);
  return 0;
}

static int imluaProcessCalcAutoGamma(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  lua_pushnumber(L, imProcessCalcAutoGamma(image));
  return 1;
}


/*****************************************************************************\
 im.ProcessUnNormalize
\*****************************************************************************/
static int imluaProcessUnNormalize (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkdatatype(L, 1, src_image, IM_FLOAT);
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchcolorspace(L, src_image, dst_image);

  imProcessUnNormalize(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessDirectConv
\*****************************************************************************/
static int imluaProcessDirectConv (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L,
    src_image->data_type == IM_SHORT ||
    src_image->data_type == IM_USHORT ||
    src_image->data_type == IM_INT ||
    src_image->data_type == IM_FLOAT,
    1, "data type can be short, ushort, int or float only");
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchsize(L, src_image, dst_image);

  imProcessDirectConv(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessNegative
\*****************************************************************************/
static int imluaProcessNegative (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  imProcessNegative(src_image, dst_image);
  return 0;
}

static int imluaProcessShiftHSI(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checknotcfloat(L, 1, src_image);
  imlua_match(L, src_image, dst_image);

  imProcessShiftHSI(src_image, dst_image, (float)luaL_checknumber(L, 3), 
                                          (float)luaL_checknumber(L, 4), 
                                          (float)luaL_checknumber(L, 5));
  return 0;
}


/*****************************************************************************\
 Threshold Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessRangeContrastThreshold
\*****************************************************************************/
static int imluaProcessRangeContrastThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int min_range = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRangeContrastThreshold(src_image, dst_image, kernel_size, min_range));
  return 1;
}

/*****************************************************************************\
 im.ProcessLocalMaxThreshold
\*****************************************************************************/
static int imluaProcessLocalMaxThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int min_thres = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessLocalMaxThreshold(src_image, dst_image, kernel_size, min_thres));
  return 1;
}

/*****************************************************************************\
 im.ProcessThreshold
\*****************************************************************************/
static int imluaProcessThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float level = (float)luaL_checknumber(L, 3);
  int value = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  imlua_checknotcfloat(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessThreshold(src_image, dst_image, level, value);
  return 0;
}

/*****************************************************************************\
 im.ProcessThresholdByDiff
\*****************************************************************************/
static int imluaProcessThresholdByDiff (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_checkcolorspace(L, 1, src_image1, IM_GRAY);
  imlua_checknotcfloat(L, 1, src_image1);
  imlua_match(L, src_image1, src_image2);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image1, dst_image);

  imProcessThresholdByDiff(src_image1, src_image2, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessHysteresisThreshold
\*****************************************************************************/
static int imluaProcessHysteresisThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int low_thres = luaL_checkint(L, 3);
  int high_thres = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  imlua_checknotcfloat(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessHysteresisThreshold(src_image, dst_image, low_thres, high_thres);
  return 0;
}

/*****************************************************************************\
 im.ProcessHysteresisThresEstimate
\*****************************************************************************/
static int imluaProcessHysteresisThresEstimate (lua_State *L)
{
  int low_thres, high_thres;

  imImage *src_image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);

  imProcessHysteresisThresEstimate(src_image, &low_thres, &high_thres);
  lua_pushnumber(L, low_thres);
  lua_pushnumber(L, high_thres);

  return 2;
}

/*****************************************************************************\
 im.ProcessUniformErrThreshold
\*****************************************************************************/
static int imluaProcessUniformErrThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushinteger(L, imProcessUniformErrThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessDifusionErrThreshold
\*****************************************************************************/
static int imluaProcessDifusionErrThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int level = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchcheck(L, src_image->depth == dst_image->depth, "images must have the same depth");
  imlua_matchsize(L, src_image, dst_image);

  imProcessDifusionErrThreshold(src_image, dst_image, level);
  return 0;
}

/*****************************************************************************\
 im.ProcessPercentThreshold
\*****************************************************************************/
static int imluaProcessPercentThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checkhistogramtype(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushinteger(L, imProcessPercentThreshold(src_image, dst_image, percent));
  return 1;
}

/*****************************************************************************\
 im.ProcessOtsuThreshold
\*****************************************************************************/
static int imluaProcessOtsuThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkhistogramtype(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushinteger(L, imProcessOtsuThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessMinMaxThreshold
\*****************************************************************************/
static int imluaProcessMinMaxThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  imlua_checknotcfloat(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushnumber(L, imProcessMinMaxThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessLocalMaxThresEstimate
\*****************************************************************************/
static int imluaProcessLocalMaxThresEstimate (lua_State *L)
{
  int thres;
  imImage *image = imlua_checkimage(L, 1);

  imlua_checkhistogramtype(L, 1, image);

  imProcessLocalMaxThresEstimate(image, &thres);

  lua_pushnumber(L, thres);
  return 1;
}

/*****************************************************************************\
 im.ProcessSliceThreshold
\*****************************************************************************/
static int imluaProcessSliceThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  float start_level = (float)luaL_checknumber(L, 3);
  float end_level = (float)luaL_checknumber(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  imlua_checknotcfloat(L, 1, src_image);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessSliceThreshold(src_image, dst_image, start_level, end_level);
  return 0;
}


/*****************************************************************************\
 Special Effects
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessPixelate
\*****************************************************************************/
static int imluaProcessPixelate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int box_size = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessPixelate(src_image, dst_image, box_size);
  return 0;
}

/*****************************************************************************\
 im.ProcessPosterize
\*****************************************************************************/
static int imluaProcessPosterize (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int level = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (level >= 1 && level <= 7), 3, "invalid level, must be >=1 and <=7");

  imProcessPosterize(src_image, dst_image, level);
  return 0;
}

static int imluaProcessNormDiffRatio(lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image1, src_image2);
  imlua_matchcolorspace(L, src_image1, dst_image);
  imlua_checkdatatype(L, 3, dst_image, IM_FLOAT);

  imProcessNormDiffRatio(src_image1, src_image2, dst_image);
  return 0;
}

static int imluaProcessAbnormalHyperionCorrection(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *image_abnormal = NULL;

  imlua_match(L, src_image, dst_image);
  imlua_checknotcfloat(L, 1, src_image);

  if (!lua_isnil(L, 5))
  {
    image_abnormal = imlua_checkimage(L, 5);
    imlua_checkcolorspace(L, 5, image_abnormal, IM_BINARY);
  }

  imProcessAbnormalHyperionCorrection(src_image, dst_image, luaL_checkint(L, 3), luaL_checkint(L, 4), image_abnormal);
  return 0;
}


static int imlua_ProcessOpenMPSetMinCount(lua_State *L)
{
  lua_pushinteger(L, imProcessOpenMPSetMinCount(luaL_checkint(L, 1)));
  return 1;
}

static int imlua_ProcessOpenMPSetNumThreads(lua_State *L)
{
  lua_pushinteger(L, imProcessOpenMPSetNumThreads(luaL_checkint(L, 1)));
  return 1;
}

static const luaL_Reg improcess_lib[] = {
  {"CalcRMSError", imluaCalcRMSError},
  {"CalcSNR", imluaCalcSNR},
  {"CalcCountColors", imluaCalcCountColors},
  {"CalcHistogram", imluaCalcHistogram},
  {"CalcGrayHistogram", imluaCalcGrayHistogram},
  {"CalcImageStatistics", imluaCalcImageStatistics},
  {"CalcHistogramStatistics", imluaCalcHistogramStatistics},
  {"CalcHistoImageStatistics", imluaCalcHistoImageStatistics},
  {"CalcPercentMinMax", imluaCalcPercentMinMax},

  {"AnalyzeFindRegions", imluaAnalyzeFindRegions},
  {"AnalyzeMeasureArea", imluaAnalyzeMeasureArea},
  {"AnalyzeMeasurePerimArea", imluaAnalyzeMeasurePerimArea},
  {"AnalyzeMeasureCentroid", imluaAnalyzeMeasureCentroid},
  {"AnalyzeMeasurePrincipalAxis", imluaAnalyzeMeasurePrincipalAxis},
  {"AnalyzeMeasurePerimeter", imluaAnalyzeMeasurePerimeter},
  {"AnalyzeMeasureHoles", imluaAnalyzeMeasureHoles},

  {"ProcessPerimeterLine", imluaProcessPerimeterLine},
  {"ProcessRemoveByArea", imluaProcessRemoveByArea},
  {"ProcessFillHoles", imluaProcessFillHoles},

  {"ProcessHoughLines", imluaProcessHoughLines},
  {"ProcessHoughLinesDraw", imluaProcessHoughLinesDraw},
  {"ProcessDistanceTransform", imluaProcessDistanceTransform},
  {"ProcessRegionalMaximum", imluaProcessRegionalMaximum},

  {"ProcessReduce", imluaProcessReduce},
  {"ProcessResize", imluaProcessResize},
  {"ProcessReduceBy4", imluaProcessReduceBy4},
  {"ProcessCrop", imluaProcessCrop},
  {"ProcessAddMargins", imluaProcessAddMargins},
  {"ProcessInsert", imluaProcessInsert},

  {"ProcessCalcRotateSize", imluaProcessCalcRotateSize},
  {"ProcessRotate", imluaProcessRotate},
  {"ProcessRotateRef", imluaProcessRotateRef},
  {"ProcessRotate90", imluaProcessRotate90},
  {"ProcessRotate180", imluaProcessRotate180},
  {"ProcessMirror", imluaProcessMirror},
  {"ProcessFlip", imluaProcessFlip},
  {"ProcessRadial", imluaProcessRadial},
  {"ProcessSwirl", imluaProcessSwirl},
  {"ProcessInterlaceSplit", imluaProcessInterlaceSplit},

  {"ProcessGrayMorphConvolve", imluaProcessGrayMorphConvolve},
  {"ProcessGrayMorphErode", imluaProcessGrayMorphErode},
  {"ProcessGrayMorphDilate", imluaProcessGrayMorphDilate},
  {"ProcessGrayMorphOpen", imluaProcessGrayMorphOpen},
  {"ProcessGrayMorphClose", imluaProcessGrayMorphClose},
  {"ProcessGrayMorphTopHat", imluaProcessGrayMorphTopHat},
  {"ProcessGrayMorphWell", imluaProcessGrayMorphWell},
  {"ProcessGrayMorphGradient", imluaProcessGrayMorphGradient},

  {"ProcessBinMorphConvolve", imluaProcessBinMorphConvolve},
  {"ProcessBinMorphErode", imluaProcessBinMorphErode},
  {"ProcessBinMorphDilate", imluaProcessBinMorphDilate},
  {"ProcessBinMorphOpen", imluaProcessBinMorphOpen},
  {"ProcessBinMorphClose", imluaProcessBinMorphClose},
  {"ProcessBinMorphOutline", imluaProcessBinMorphOutline},
  {"ProcessBinMorphThin", imluaProcessBinMorphThin},

  {"ProcessMedianConvolve", imluaProcessMedianConvolve},
  {"ProcessRangeConvolve", imluaProcessRangeConvolve},
  {"ProcessRankClosestConvolve", imluaProcessRankClosestConvolve},
  {"ProcessRankMaxConvolve", imluaProcessRankMaxConvolve},
  {"ProcessRankMinConvolve", imluaProcessRankMinConvolve},

  {"ProcessConvolve", imluaProcessConvolve},
  {"ProcessConvolveDual", imluaProcessConvolveDual},
  {"ProcessConvolveRep", imluaProcessConvolveRep},
  {"ProcessConvolveSep", imluaProcessConvolveSep},
  {"ProcessCompassConvolve", imluaProcessCompassConvolve},
  {"ProcessRotateKernel", imluaProcessRotateKernel},
  {"ProcessDiffOfGaussianConvolve", imluaProcessDiffOfGaussianConvolve},
  {"ProcessLapOfGaussianConvolve", imluaProcessLapOfGaussianConvolve},
  {"ProcessMeanConvolve", imluaProcessMeanConvolve},
  {"ProcessBarlettConvolve", imluaProcessBarlettConvolve},
  {"ProcessGaussianConvolve", imluaProcessGaussianConvolve},
  {"ProcessSobelConvolve", imluaProcessSobelConvolve},
  {"ProcessPrewittConvolve", imluaProcessPrewittConvolve},
  {"ProcessSplineEdgeConvolve", imluaProcessSplineEdgeConvolve},
  {"ProcessZeroCrossing", imluaProcessZeroCrossing},
  {"ProcessCanny", imluaProcessCanny},
  {"ProcessUnsharp", imluaProcessUnsharp},
  {"ProcessSharp", imluaProcessSharp},
  {"ProcessSharpKernel", imluaProcessSharpKernel},
  {"GaussianKernelSize2StdDev", imluaGaussianKernelSize2StdDev},
  {"GaussianStdDev2KernelSize", imluaGaussianStdDev2KernelSize},

  {"ProcessUnaryPointOp", imluaProcessUnaryPointOp},
  {"ProcessUnaryPointColorOp", imluaProcessUnaryPointColorOp},
  {"ProcessMultiPointOp", imluaProcessMultiPointOp},
  {"ProcessMultiPointColorOp", imluaProcessMultiPointColorOp},
  {"ProcessUnArithmeticOp", imluaProcessUnArithmeticOp},
  {"ProcessArithmeticOp", imluaProcessArithmeticOp},
  {"ProcessArithmeticConstOp", imluaProcessArithmeticConstOp},
  {"ProcessBlendConst", imluaProcessBlendConst},
  {"ProcessBlend", imluaProcessBlend},
  {"ProcessCompose", imluaProcessCompose},
  {"ProcessSplitComplex", imluaProcessSplitComplex},
  {"ProcessMergeComplex", imluaProcessMergeComplex},
  {"ProcessMultipleMean", imluaProcessMultipleMean},
  {"ProcessMultipleStdDev", imluaProcessMultipleStdDev},
  {"ProcessAutoCovariance", imluaProcessAutoCovariance},
  {"ProcessMultiplyConj", imluaProcessMultiplyConj},

  {"ProcessQuantizeRGBUniform", imluaProcessQuantizeRGBUniform},
  {"ProcessQuantizeGrayUniform", imluaProcessQuantizeGrayUniform},

  {"ProcessExpandHistogram", imluaProcessExpandHistogram},
  {"ProcessEqualizeHistogram", imluaProcessEqualizeHistogram},

  {"ProcessSplitYChroma", imluaProcessSplitYChroma},
  {"ProcessSplitHSI", imluaProcessSplitHSI},
  {"ProcessMergeHSI", imluaProcessMergeHSI},
  {"ProcessSplitComponents", imluaProcessSplitComponents},
  {"ProcessMergeComponents", imluaProcessMergeComponents},
  {"ProcessNormalizeComponents", imluaProcessNormalizeComponents},
  {"ProcessReplaceColor", imluaProcessReplaceColor},
  {"ProcessSetAlphaColor", imluaProcessSetAlphaColor},

  {"ProcessBitwiseOp", imluaProcessBitwiseOp},
  {"ProcessBitwiseNot", imluaProcessBitwiseNot},
  {"ProcessBitMask", imluaProcessBitMask},
  {"ProcessBitPlane", imluaProcessBitPlane},

  {"ProcessRenderOp", imluaProcessRenderOp},
  {"ProcessRenderCondOp", imluaProcessRenderCondOp},
  {"ProcessRenderAddSpeckleNoise", imluaProcessRenderAddSpeckleNoise},
  {"ProcessRenderAddGaussianNoise", imluaProcessRenderAddGaussianNoise},
  {"ProcessRenderAddUniformNoise", imluaProcessRenderAddUniformNoise},
  {"ProcessRenderRandomNoise", imluaProcessRenderRandomNoise},
  {"ProcessRenderConstant", imluaProcessRenderConstant},
  {"ProcessRenderWheel", imluaProcessRenderWheel},
  {"ProcessRenderCone", imluaProcessRenderCone},
  {"ProcessRenderTent", imluaProcessRenderTent},
  {"ProcessRenderRamp", imluaProcessRenderRamp},
  {"ProcessRenderBox", imluaProcessRenderBox},
  {"ProcessRenderSinc", imluaProcessRenderSinc},
  {"ProcessRenderGaussian", imluaProcessRenderGaussian},
  {"ProcessRenderLapOfGaussian", imluaProcessRenderLapOfGaussian},
  {"ProcessRenderCosine", imluaProcessRenderCosine},
  {"ProcessRenderGrid", imluaProcessRenderGrid},
  {"ProcessRenderChessboard", imluaProcessRenderChessboard},

  {"ProcessToneGamut", imluaProcessToneGamut},
  {"ProcessUnNormalize", imluaProcessUnNormalize},
  {"ProcessDirectConv", imluaProcessDirectConv},
  {"ProcessNegative", imluaProcessNegative},
  {"ProcessCalcAutoGamma", imluaProcessCalcAutoGamma},
  {"ProcessShiftHSI", imluaProcessShiftHSI},

  {"ProcessRangeContrastThreshold", imluaProcessRangeContrastThreshold},
  {"ProcessLocalMaxThreshold", imluaProcessLocalMaxThreshold},
  {"ProcessThreshold", imluaProcessThreshold},
  {"ProcessThresholdByDiff", imluaProcessThresholdByDiff},
  {"ProcessHysteresisThreshold", imluaProcessHysteresisThreshold},
  {"ProcessHysteresisThresEstimate", imluaProcessHysteresisThresEstimate},
  {"ProcessUniformErrThreshold", imluaProcessUniformErrThreshold},
  {"ProcessDifusionErrThreshold", imluaProcessDifusionErrThreshold},
  {"ProcessPercentThreshold", imluaProcessPercentThreshold},
  {"ProcessOtsuThreshold", imluaProcessOtsuThreshold},
  {"ProcessMinMaxThreshold", imluaProcessMinMaxThreshold},
  {"ProcessLocalMaxThresEstimate", imluaProcessLocalMaxThresEstimate},
  {"ProcessSliceThreshold", imluaProcessSliceThreshold},

  {"ProcessPixelate", imluaProcessPixelate},
  {"ProcessPosterize", imluaProcessPosterize},
  {"ProcessNormDiffRatio", imluaProcessNormDiffRatio},
  {"ProcessAbnormalHyperionCorrection", imluaProcessAbnormalHyperionCorrection},
  
  {"ProcessOpenMPSetMinCount", imlua_ProcessOpenMPSetMinCount},
  {"ProcessOpenMPSetNumThreads", imlua_ProcessOpenMPSetNumThreads},

  {NULL, NULL}
};

static const luaL_Reg imimageprocess_lib[] = {
  {"Gamma", imluaImageGamma},
  {"Negative", imluaImageNegative},
  {"BrightnessContrast", imluaImageBrightnessContrast},
  {"Equalize", imluaImageEqualize},
  {"AutoLevel", imluaImageAutoLevel},
  {"Level", imluaImageLevel},

  {NULL, NULL}
};

/*****************************************************************************\
 Constants
\*****************************************************************************/
static const imlua_constant im_process_constants[] = {

  { "UN_EQL", IM_UN_EQL, NULL },
  { "UN_ABS", IM_UN_ABS, NULL },
  { "UN_LESS", IM_UN_LESS, NULL },
  { "UN_INV", IM_UN_INV, NULL },
  { "UN_SQR", IM_UN_SQR, NULL },
  { "UN_SQRT", IM_UN_SQRT, NULL },
  { "UN_LOG", IM_UN_LOG, NULL },
  { "UN_EXP", IM_UN_EXP, NULL },
  { "UN_SIN", IM_UN_SIN, NULL },
  { "UN_COS", IM_UN_COS, NULL },
  { "UN_CONJ", IM_UN_CONJ, NULL },
  { "UN_CPXNORM", IM_UN_CPXNORM, NULL },
  { "UN_POSITIVES", IM_UN_POSITIVES, NULL },
  { "UN_NEGATIVES", IM_UN_NEGATIVES, NULL },

  { "BIN_ADD", IM_BIN_ADD, NULL },
  { "BIN_SUB", IM_BIN_SUB, NULL },
  { "BIN_MUL", IM_BIN_MUL, NULL },
  { "BIN_DIV", IM_BIN_DIV, NULL },
  { "BIN_DIFF", IM_BIN_DIFF, NULL },
  { "BIN_POW", IM_BIN_POW, NULL },
  { "BIN_MIN", IM_BIN_MIN, NULL },
  { "BIN_MAX", IM_BIN_MAX, NULL },

  { "BIT_AND", IM_BIT_AND, NULL },
  { "BIT_OR", IM_BIT_OR, NULL },
  { "BIT_XOR", IM_BIT_XOR, NULL },

  { "GAMUT_NORMALIZE", IM_GAMUT_NORMALIZE, NULL },
  { "GAMUT_POW", IM_GAMUT_POW, NULL },
  { "GAMUT_LOG", IM_GAMUT_LOG, NULL },
  { "GAMUT_EXP", IM_GAMUT_EXP, NULL },
  { "GAMUT_INVERT", IM_GAMUT_INVERT, NULL },
  { "GAMUT_ZEROSTART", IM_GAMUT_ZEROSTART, NULL },
  { "GAMUT_SOLARIZE", IM_GAMUT_SOLARIZE, NULL },
  { "GAMUT_SLICE", IM_GAMUT_SLICE, NULL },
  { "GAMUT_EXPAND", IM_GAMUT_EXPAND, NULL },
  { "GAMUT_CROP", IM_GAMUT_CROP, NULL },
  { "GAMUT_BRIGHTCONT", IM_GAMUT_BRIGHTCONT, NULL },
  { "GAMUT_MINMAX", IM_GAMUT_MINMAX, NULL },

  { NULL, -1, NULL },
};

/* from imlua_kernel.c */
void imlua_open_kernel(lua_State *L);

/* from imlua_convert.c */
void imlua_open_processconvert(lua_State *L);

int imlua_open_process(lua_State *L)
{
  luaL_register(L, "im", improcess_lib);   /* leave "im" table at the top of the stack */
  imlua_regconstants(L, im_process_constants);

  luaL_getmetatable(L, "imImage");
  luaL_register(L, NULL, imimageprocess_lib);
  lua_pop(L, 1);

#ifdef IMLUA_USELOH
#include "im_process.loh"
#else
#ifdef IMLUA_USELH
#include "im_process.lh"
#else
  luaL_dofile(L, "im_process.lua");
#endif
#endif

  imlua_open_kernel(L);
  imlua_open_processconvert(L);
  return 1;
}

int luaopen_imlua_process(lua_State *L)
{
  return imlua_open_process(L);
}

int luaopen_imlua_process_omp(lua_State *L)
{
  return imlua_open_process(L);
}
