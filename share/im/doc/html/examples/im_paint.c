#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <cddbuf.h>
#include <im.h>
#include <im_image.h>

#include <stdio.h>
#include <string.h>

/* TODO:
    cdCanvasPixel is not apropriate, must use cdCanvasLine for continuity.
    if resized, the content is lost
    SaveAs is not implemented
*/

static int cbCanvasRepaint(Ihandle* iup_canvas)
{
  cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(iup_canvas, "cdCanvasDBuf");

  cdCanvasActivate(dbuffer_canvas);
  cdCanvasFlush(dbuffer_canvas);
  
  return IUP_DEFAULT;
}

static int drawing = 0;

static long GetFgColor(Ihandle* ih)
{
  int r, g, b;
  Ihandle* fgcolor_but = IupGetDialogChild(ih, "fgcolor_but");
  sscanf(IupGetAttribute(fgcolor_but, "BGCOLOR"), "%d %d %d", &r, &g, &b);
  return cdEncodeColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
}

static int cbCanvasButton(Ihandle* ih, int but, int pressed, int x, int y)
{
  if (but == IUP_BUTTON1 && pressed)
  {
    cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(ih, "cdCanvasDBuf");
    long fgcolor = GetFgColor(ih);

    cdCanvasActivate(dbuffer_canvas);
    cdCanvasPixel(dbuffer_canvas, x, cdCanvasInvertYAxis(dbuffer_canvas, y), fgcolor);
    cdCanvasFlush(dbuffer_canvas);
    drawing = 1;
  }
  else
    drawing = 0;
 
  return IUP_DEFAULT;
}

static int cbCanvasMotion(Ihandle *ih, int x, int y, char *status)
{
  if (iup_isbutton1(status) && drawing)
  {
    cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(ih, "cdCanvasDBuf");
    long fgcolor = GetFgColor(ih);

    cdCanvasActivate(dbuffer_canvas);
    cdCanvasPixel(dbuffer_canvas, x, cdCanvasInvertYAxis(dbuffer_canvas, y), fgcolor);
    cdCanvasFlush(dbuffer_canvas);
  }
 
  return IUP_DEFAULT;
}

static int cbCanvasMap(Ihandle* ih)
{
  cdCanvas* cd_canvas = cdCreateCanvas(CD_IUP, ih);
  cdCanvas* dbuffer_canvas = cdCreateCanvas(CD_DBUFFER, cd_canvas);
  IupSetAttribute(IupGetDialog(ih), "cdCanvas", (char*)cd_canvas);
  IupSetAttribute(IupGetDialog(ih), "cdCanvasDBuf", (char*)dbuffer_canvas);
  return IUP_DEFAULT;
}

static int cbDialogClose(Ihandle* ih)
{
  cdCanvas* cd_canvas = (cdCanvas*)IupGetAttribute(ih, "cdCanvas");
  cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(ih, "cdCanvasDBuf");

  if (dbuffer_canvas) cdKillCanvas(dbuffer_canvas);
  if (cd_canvas) cdKillCanvas(cd_canvas);

  IupSetAttribute(ih, "cdCanvas", NULL);
  IupSetAttribute(ih, "cdCanvasDBuf", NULL);

  return IUP_CLOSE;
}

static int cbMenuNew(Ihandle* ih)
{
  cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(ih, "cdCanvasDBuf");
  cdCanvasActivate(dbuffer_canvas);
  cdCanvasClear(dbuffer_canvas);
  cdCanvasFlush(dbuffer_canvas);
  return IUP_DEFAULT;
}

static void PrintError(int error)
{
  switch (error)
  {
  case IM_ERR_OPEN:
    IupMessage("IM", "Error Opening File.");
    break;
  case IM_ERR_MEM:
    IupMessage("IM", "Insuficient memory.");
    break;
  case IM_ERR_ACCESS:
    IupMessage("IM", "Error Accessing File.");
    break;
  case IM_ERR_DATA:
    IupMessage("IM", "Image type not Suported.");
    break;
  case IM_ERR_FORMAT:
    IupMessage("IM", "Invalid Format.");
    break;
  case IM_ERR_COMPRESS:
    IupMessage("IM", "Invalid or unsupported compression.");
    break;
  default:
    IupMessage("IM", "Unknown Error.");
  }
}

static int cbMenuOpen(Ihandle* ih)
{
  int error;
  char file_name[200] = "*.*";
  imImage* image;
  cdCanvas* dbuffer_canvas = (cdCanvas*)IupGetAttribute(IupGetDialog(ih), "cdCanvasDBuf");

  if (IupGetFile(file_name) != 0)
    return IUP_DEFAULT;

  image = imFileImageLoadBitmap(file_name, 0, &error);
  if (error) PrintError(error);
  if (!image) return IUP_DEFAULT;

  cdCanvasActivate(dbuffer_canvas);
  cdCanvasClear(dbuffer_canvas);

  imcdCanvasPutImage(dbuffer_canvas, image, 0, 0, image->width, image->height, 0, 0, 0, 0);
  imImageDestroy(image);

  cdCanvasFlush(dbuffer_canvas);

  return IUP_DEFAULT;
}

static int cbMenuSaveAs(Ihandle* ih)
{
  return IUP_DEFAULT;
}

static int cbFgColor(Ihandle* ih)
{
  Ihandle* color_dlg = IupColorDlg();
  IupSetAttributeHandle(color_dlg, "PARENTDIALOG", IupGetDialog(ih));
  IupSetAttribute(color_dlg, "TITLE", "Choose Color");
  IupSetAttribute(color_dlg, "VALUE", IupGetAttribute(ih, "BGCOLOR"));

  IupPopup(color_dlg, IUP_CENTER, IUP_CENTER);

  if (IupGetInt(color_dlg, "STATUS")==1)
  {
    char* value = IupGetAttribute(color_dlg, "VALUE");
    IupStoreAttribute(ih, "BGCOLOR", value);
  }

  IupDestroy(color_dlg);

  return IUP_DEFAULT;
}

static int cbMenuAbout(void)
{
  IupMessage("About", "Simple Paint\nVersion 1.0");
  return IUP_DEFAULT;
}

static Ihandle* CreateDialog(void)
{
  Ihandle *iup_dialog, *iup_canvas, *menu;

  menu = IupMenu(
    IupSubmenu("&File", IupMenu(
      IupSetCallbacks(IupItem("New", NULL), "ACTION", cbMenuNew, NULL),
      IupSetCallbacks(IupItem("Open...\tCtrl+O", NULL), "ACTION", cbMenuOpen, NULL),
      IupSetCallbacks(IupItem("Save As...", NULL), "ACTION", cbMenuSaveAs, NULL), 
      IupSetCallbacks(IupItem("E&xit", NULL), "ACTION", cbDialogClose, NULL),  /* can do that because the menu inherit from the dialog */
      NULL)),
    IupSubmenu("&Help", IupMenu(
      IupSetCallbacks(IupItem("About", NULL), "ACTION", (Icallback)cbMenuAbout, NULL),
      NULL)),
    NULL);

  iup_canvas = IupCanvas(NULL);
  IupSetCallback(iup_canvas, "BUTTON_CB", (Icallback)cbCanvasButton);
  IupSetCallback(iup_canvas, "MOTION_CB", (Icallback)cbCanvasMotion);
  IupSetCallback(iup_canvas, "ACTION", cbCanvasRepaint);
  IupSetCallback(iup_canvas, "MAP_CB", cbCanvasMap);
  
  iup_dialog = IupDialog(
    IupVbox(
      IupSetAttributes(IupHbox(
        IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_FileNew, FLAT=Yes"), "ACTION", (Icallback)cbMenuNew, NULL),
        IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_FileOpen, FLAT=Yes"), "ACTION", (Icallback)cbMenuOpen, NULL),
        IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "IMAGE=IUP_FileSave, FLAT=Yes"), "ACTION", (Icallback)cbMenuSaveAs, NULL),
        IupSetAttributes(IupFill(), "SIZE=50"),
        IupLabel("Color:"),
        IupSetCallbacks(IupSetAttributes(IupButton(NULL, NULL), "BGCOLOR=\"0 0 0\", SIZE=20x, FLAT=Yes, NAME=fgcolor_but"), "ACTION", (Icallback)cbFgColor, NULL),
        NULL), "MARGIN=5x5, GAP=5, ALIGNMENT=ACENTER"),
      iup_canvas,
      NULL)
    );
  IupSetCallback(iup_dialog, "CLOSE_CB", cbDialogClose);
  IupSetCallback(iup_dialog, "K_cO", cbMenuOpen),
  IupSetAttribute(iup_dialog, "SIZE", "HALFxHALF");  /* initial size */
  IupSetAttribute(iup_dialog, "TITLE", "Simple Paint");
  IupSetAttributeHandle(iup_dialog, "MENU", menu);

  return iup_dialog;
}

int main(int argc, char* argv[])
{
  Ihandle* dlg;

  IupOpen(&argc, &argv);
  IupImageLibOpen();

  dlg = CreateDialog();

  IupShow(dlg);
  
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();

  return 0;
}
