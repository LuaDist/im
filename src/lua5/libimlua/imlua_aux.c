/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "im.h"
#include "im_image.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"


/*****************************************************************************\
 Creates an int array.
\*****************************************************************************/
int imlua_newarrayint (lua_State *L, const int *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Creates an unsigned long array.
\*****************************************************************************/
int imlua_newarrayulong (lua_State *L, const unsigned long *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushnumber(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Creates a float array.
\*****************************************************************************/
int imlua_newarrayfloat (lua_State *L, const float *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushnumber(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Retrieve an int array.
\*****************************************************************************/
int *imlua_toarrayint(lua_State *L, int index, int *count, int start)
{
  luaL_checktype(L, index, LUA_TTABLE);
  return imlua_toarrayintopt(L, index, count, start);
}

int *imlua_toarrayintopt(lua_State *L, int index, int *count, int start)
{
  int i, n;
  int *value = NULL;

  if (count) *count = 0;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (int*) malloc (sizeof(int) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = luaL_checkint(L, -1);
      lua_pop(L, 1);
    }
  }
  else if (!lua_isnil(L, index))
    luaL_argerror(L, index, "must be a table or nil");

  return value;
}

/*****************************************************************************\
 Retrieve an ulong array.
\*****************************************************************************/
unsigned long *imlua_toarrayulong(lua_State *L, int index, int *count, int start)
{
  luaL_checktype(L, index, LUA_TTABLE);
  return imlua_toarrayulongopt(L, index, count, start);
}

unsigned long *imlua_toarrayulongopt(lua_State *L, int index, int *count, int start)
{
  int i, n;
  unsigned long *value = NULL;

  if (count) *count = 0;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (unsigned long*) malloc (sizeof(unsigned long) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = luaL_checkint(L, -1);
      lua_pop(L, 1);
    }
  }
  else if (!lua_isnil(L, index))
    luaL_argerror(L, index, "must be a table or nil");

  return value;
}

/*****************************************************************************\
 Retrieve a float array.
\*****************************************************************************/
float *imlua_toarrayfloat(lua_State *L, int index, int *count, int start)
{
  luaL_checktype(L, index, LUA_TTABLE);
  return imlua_toarrayfloatopt(L, index, count, start);
}

float *imlua_toarrayfloatopt(lua_State *L, int index, int *count, int start)
{
  int i, n;
  float *value = NULL;

  if (count) *count = 0;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (float*) malloc (sizeof(float) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = (float) luaL_checknumber(L, -1);
      lua_pop(L, 1);
    }
  }
  else if (!lua_isnil(L, index))
    luaL_argerror(L, index, "must be a table or nil");

  return value;
}

imImage* *imlua_toarrayimage(lua_State *L, int index, int *count, int start)
{
  int i, n;
  imImage* *value = NULL;

  *count = 0;

  luaL_checktype(L, index, LUA_TTABLE);

  n = imlua_getn(L, index);
  if (start == 0) n++;
  if (count) *count = n;

  value = (imImage**) malloc (sizeof(imImage*) * n);
  for (i = 0; i < n; i++)
  {
    lua_rawgeti(L, index, i+start);
    value[i] = imlua_checkimage(L, -1);
    lua_pop(L, 1);
  }

  for (i = 1; i < n; i++)
  {
    int check = imImageMatch(value[0], value[i]);
    if (!check) free(value);
    imlua_matchcheck(L, check, "images must have the same size and data type");
  }

  return value;
}

/*****************************************************************************\
 Creates a bit mask based on a string formatted as "11000110".
\*****************************************************************************/
unsigned char imlua_checkmask (lua_State *L, int index)
{
  int i;
  unsigned char mask = 0;
  const char *str = luaL_checkstring(L, index);
  if (strlen(str) != 8)
    luaL_argerror(L, index, "invalid mask, must have 8 elements");

  for (i = 0; i < 8; i++)
  {
    char c = str[i];
    if (c != '0' && c != '1')
      luaL_argerror(L, index, "invalid mask, must have 0s or 1s only");

    mask |= (c - '0') << (7 - i);
  }

  return mask;
}

/*****************************************************************************\
\*****************************************************************************/

void imlua_argerrorcolorspace (lua_State *L, int index, int color_space)
{
  char msg[100] = "color space must be ";
  strcat(msg, imColorModeSpaceName(color_space));
  luaL_argerror(L, index, msg);
}

void imlua_argerrordatatype (lua_State *L, int index, int data_type)
{
  char msg[100] = "data type must be ";
  strcat(msg, imDataTypeName(data_type));
  luaL_argerror(L, index, msg);
}

void imlua_errormatchsize(lua_State *L)
{
  luaL_error(L, "images must have the same size");
}

void imlua_errormatchcolor(lua_State *L)
{
  luaL_error(L, "images must have the same data type and color space");
}

void imlua_errormatchdatatype(lua_State *L)
{
  luaL_error(L, "images must have the same size and data type");
}

void imlua_errormatchcolorspace(lua_State *L)
{
  luaL_error(L, "images must have the same size and color space");
}

void imlua_errormatch(lua_State *L)
{
  luaL_error(L, "images must have the same size, data type and color space");
}

const char* imlua_checkformat(lua_State *L, int index)
{
  const char *format = luaL_checkstring(L, index);

  if (imFormatInfo(format, NULL, NULL, NULL)==IM_ERR_FORMAT)
    luaL_error(L, "invalid, unknown or unregistered format");

  return format;
}
