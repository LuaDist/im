require"imlua"
require"imlua_process"

local image = im.FileImageLoad("test.png")

counter, filter = im.ProcessSobelConvolveNew(image)

if (image:HasAlpha()) then
  --image:CopyPlane(4, filter, 4)
  filter:SetAlpha(255)
end

filter:Save("test_sobel.png", "PNG")
