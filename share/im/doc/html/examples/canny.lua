require"imlua"
require"imlua_process"

local infilename = "lena.jpg"
local outfilename = "sobel_lena.png"

local image = im.FileImageLoad(infilename)
 
-- turn it to a gray image
local gray = im.ImageCreate(image:Width(), image:Height(), im.GRAY, im.BYTE) 
im.ProcessConvertColorSpace(image, gray);
 
-- use Canny convolution filtering to detect edges 
local sigma = 1.4 
local filter = im.ProcessCannyNew(gray, sigma)
 
local low_thres  = 0    -- not sure what a good value is here
local high_thres = 100  -- not sure either
low_level, high_level  = im.ProcessHysteresisThresEstimate(filter)
 
local hst = im.ProcessHysteresisThresholdNew(filter, low_thres, high_thres)
hst:Save(outfilename, "PNG")
