/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#ifndef __IMLUA_AUX_H
#define __IMLUA_AUX_H

#if	defined(__cplusplus)
extern "C" {
#endif


/********************************/
/* exported from "imlua_aux.c". */
/********************************/

/* get table size */

#if LUA_VERSION_NUM > 501
#define imlua_getn(L,i)          ((int)lua_rawlen(L, i))
#else
#define imlua_getn(L,i)          ((int)lua_objlen(L, i))
#endif

/* array */

int imlua_newarrayint(lua_State *L, const int *value, int count, int start);
int imlua_newarrayulong(lua_State *L, const unsigned long *value, int count, int start);
int imlua_newarrayfloat(lua_State *L, const float *value, int count, int start);

int *imlua_toarrayint(lua_State *L, int index, int *count, int start);
int *imlua_toarrayintopt(lua_State *L, int index, int *count, int start);
unsigned long *imlua_toarrayulong (lua_State *L, int index, int *count, int start);
unsigned long *imlua_toarrayulongopt(lua_State *L, int index, int *count, int start);
float *imlua_toarrayfloat(lua_State *L, int index, int *count, int start);
float *imlua_toarrayfloatopt(lua_State *L, int index, int *count, int start);
imImage* *imlua_toarrayimage(lua_State *L, int index, int *count, int start);

/* other parameter checking */

unsigned char imlua_checkmask(lua_State *L, int index);
const char* imlua_checkformat(lua_State *L, int index);

void imlua_argerrordatatype (lua_State *L, int index, int data_type);
void imlua_argerrorcolorspace (lua_State *L, int index, int color_space);

#define imlua_checktype(_L, _a, _i, _c, _d) imlua_checkcolorspace(_L, _a, _i, _c); imlua_checkdatatype(_L, _a, _i, _d)
#define imlua_checkcolorspace(_L, _a, _i, _c) if ((_i)->color_space != _c) \
                                                imlua_argerrorcolorspace(_L, _a, _c)
#define imlua_checkdatatype(_L, _a, _i, _d) if ((_i)->data_type != _d) \
                                              imlua_argerrordatatype(_L, _a, _d)

void imlua_errormatchsize(lua_State *L);
void imlua_errormatchcolor(lua_State *L);
void imlua_errormatchdatatype(lua_State *L);
void imlua_errormatchcolorspace(lua_State *L);
void imlua_errormatch(lua_State *L);

#define imlua_matchsize(_L, _i1, _i2) if (!imImageMatchSize(_i1, _i2)) \
                                        imlua_errormatchsize(_L)
#define imlua_matchcolor(_L, _i1, _i2) if (!imImageMatchColor(_i1, _i2)) \
                                         imlua_errormatchcolor(_L)
#define imlua_matchdatatype(_L, _i1, _i2) if (!imImageMatchDataType(_i1, _i2)) \
                                            imlua_errormatchdatatype(_L)
#define imlua_matchcolorspace(_L, _i1, _i2) if (!imImageMatchColorSpace(_i1, _i2)) \
                                              imlua_errormatchcolorspace(_L)
#define imlua_match(_L, _i1, _i2) if (!imImageMatch(_i1, _i2)) \
                                    imlua_errormatch(_L)

/* used only when comparing two images */
#define imlua_matchcheck(L, cond, extramsg) if (!(cond)) \
                                              luaL_error(L, extramsg)

#define imlua_pusherror(L, _e) ((_e == IM_ERR_NONE)? lua_pushnil(L): lua_pushnumber(L, _e))


/********************************/
/* exported from "imlua.c".     */
/********************************/

/* constant registration. */

typedef struct _imlua_constant {
  const char *name;
  lua_Number value;
  const char *str_value;
} imlua_constant;

void imlua_regconstants(lua_State *L, const imlua_constant *imconst);


/********************************/
/* private module open          */
/********************************/

void imlua_open_convert(lua_State *L);  /* imlua_convert.c */
void imlua_open_util(lua_State *L);     /* imlua_util.c    */
void imlua_open_file(lua_State *L);     /* imlua_file.c    */


#ifdef __cplusplus
}
#endif

#endif
