/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#ifndef __IMLUA_IMAGE_H
#define __IMLUA_IMAGE_H

#if	defined(__cplusplus)
extern "C" {
#endif


typedef struct _imluaImageChannel {
  imImage *image;
  int channel;
} imluaImageChannel;

typedef struct _imluaImageRow {
  imImage *image;
  int channel;
  int row;
} imluaImageRow;

void imlua_open_image(lua_State *L);

int imlua_pushimageerror(lua_State *L, imImage* image, int error);
void imlua_pushimage(lua_State *L, imImage* image);
imImage* imlua_checkimage(lua_State *L, int param);


#ifdef __cplusplus
}
#endif

#endif
