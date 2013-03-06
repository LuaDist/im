require"imlua"
require"imlua_process"

local infilename = "lines.png"
local outfilename = "lines_hough.png"

local image = im.FileImageLoad(infilename)

-- turn it to a gray image
local gray    = im.ImageCreate(image:Width(), image:Height(), im.GRAY, im.BYTE) 
im.ProcessConvertColorSpace(image, gray);

-- use Canny convolution filtering to detect edges 
local sigma             = 0.4 
local filter            = im.ProcessCannyNew(gray, sigma)

low_level, high_level   = im.ProcessHysteresisThresEstimate(filter)
local hst               = im.ProcessHysteresisThresholdNew(filter, low_level, high_level)

local width             = image:Width()
local height            = image:Height()

local rhomax            = math.sqrt(width*width +height*height)/2;
local hough_height      = 2*rhomax+1;
local hough             = im.ImageCreate(180, hough_height, im.GRAY, im.INT);
local hough_binary      = im.ImageCreate(180, hough_height, im.BINARY, im.BYTE);

im.ProcessHoughLines(hst, hough)
im.ProcessLocalMaxThreshold(hough, hough_binary, 7, 100)

im.ProcessHoughLinesDraw(image, hough, hough_binary, image); 
--im.ProcessHoughLinesDraw(gray, hough, hough_binary, gray); 

image:Save(outfilename, "PNG")
--gray:Save(outfilename, "PNG")
