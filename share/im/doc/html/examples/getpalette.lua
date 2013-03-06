require"imlua"

ifile = im.FileOpen("flower.gif")
error, width, height, color_mode = ifile:ReadImageInfo(0)
print("Color Space: "..im.ColorModeSpaceName(color_mode))
pal = ifile:GetPalette()
print("Palette Count: "..#pal)
print("Palette[0]: ",im.ColorDecode(pal[0]))
print("Palette[1]: ",im.ColorDecode(pal[1]))
print("Palette[2]: ",im.ColorDecode(pal[2]))
ifile:Close()
