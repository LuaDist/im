ifile = im.FileNew("1frame.avi", "AVI")
ifile:SetInfo("CUSTOM")
ifile:SetAttribute("FPS", im.FLOAT, {15}) -- Frames per second 
ifile:SetAttribute("AVIQuality", im.INT, {10000}) -- Default Quality 
new_image = im.ImageCreate(320, 240, im.RGB, im.BYTE)
err = ifile:SaveImage(new_image)
if (err ~= im.ERR_NONE) then
  ifile:Close()
  error("Error Saving: ".. err)
end
ifile:SaveImage(new_image)
ifile:Close()
