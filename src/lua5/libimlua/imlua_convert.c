/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#include "im.h"
#include "im_image.h"
#include "im_convert.h"

#ifdef IM_PROCESS
#include "im_process_pnt.h"
#endif

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_image.h"
#include "imlua_aux.h"


/*****************************************************************************\
 im.ConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode)
\*****************************************************************************/
static int imluaConvertDataType (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int cpx2real = luaL_checkint(L, 3);
  float gamma = (float) luaL_checknumber(L, 4);
  int abssolute = lua_toboolean(L, 5);
  int cast_mode = luaL_checkint(L, 6);

  imlua_matchcolorspace(L, src_image, dst_image);
#ifdef IM_PROCESS
  imlua_pusherror(L, imProcessConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode));
#else
  imlua_pusherror(L, imConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode));
#endif
  return 1;
}

/*****************************************************************************\
 im.ConvertColorSpace(src_image, dst_image)
\*****************************************************************************/
static int imluaConvertColorSpace (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchdatatype(L, src_image, dst_image);
#ifdef IM_PROCESS
  imlua_pusherror(L, imProcessConvertColorSpace(src_image, dst_image));
#else
  imlua_pusherror(L, imConvertColorSpace(src_image, dst_image));
#endif
  return 1;
}

/*****************************************************************************\
 im.ConvertToBitmap(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode)
\*****************************************************************************/
static int imluaConvertToBitmap (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int cpx2real = luaL_checkint(L, 3);
  float gamma = (float) luaL_checknumber(L, 4);
  int abssolute = lua_toboolean(L, 5);
  int cast_mode = luaL_checkint(L, 6);

  imlua_matchsize(L, src_image, dst_image);
  imlua_matchcheck(L, imImageIsBitmap(dst_image), "image must be a bitmap");

#ifdef IM_PROCESS
  imlua_pusherror(L, imProcessConvertToBitmap(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode));
#else
  imlua_pusherror(L, imConvertToBitmap(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode));
#endif
  return 1;
}

#ifdef IM_PROCESS
static const luaL_Reg imconvert_lib[] = {
  {"ProcessConvertDataType", imluaConvertDataType},
  {"ProcessConvertColorSpace", imluaConvertColorSpace},
  {"ProcessConvertToBitmap", imluaConvertToBitmap},
  {NULL, NULL}
};

void imlua_open_processconvert (lua_State *L)
{
  /* "im" table is at the top of the stack */
  luaL_register(L, NULL, imconvert_lib);

#ifdef IMLUA_USELOH
#include "im_processconvert.loh"
#else
#ifdef IMLUA_USELH
#include "im_processconvert.lh"
#else
  luaL_dofile(L, "im_processconvert.lua");
#endif
#endif

}
#else
static const luaL_Reg imconvert_lib[] = {
  {"ConvertDataType", imluaConvertDataType},
  {"ConvertColorSpace", imluaConvertColorSpace},
  {"ConvertToBitmap", imluaConvertToBitmap},
  {NULL, NULL}
};

void imlua_open_convert (lua_State *L)
{
  /* "im" table is at the top of the stack */
  luaL_register(L, NULL, imconvert_lib);

#ifdef IMLUA_USELOH
#include "im_convert.loh"
#else
#ifdef IMLUA_USELH
#include "im_convert.lh"
#else
  luaL_dofile(L, "im_convert.lua");
#endif
#endif

}
#endif
