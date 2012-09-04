require"imlua"
require"imlua_process"

function myLoadImage(filename)
  print("Loading: "..filename) 
  local image, err = im.FileImageLoad(filename)
  if not image then
    error(im.ErrorStr(err))
  end
  return image
end

function myPrintStats(image)
  local stats = im.CalcImageStatistics(image)
  if (image:Depth() == 1) then
    print("min:", stats.min)
    print("mean:", stats.mean )
    print("max:", stats.max)
  else
    print("min:", stats[0].min, stats[1].min, stats[2].min)
    print("mean:", stats[0].mean, stats[1].mean, stats[2].mean )
    print("max:", stats[0].max, stats[1].max, stats[2].max)
  end
end

local file_path = ""

local r  = myLoadImage(file_path.."_B05_L1T.TIF")   -- these are short images
local g  = myLoadImage(file_path.."_B04_L1T.TIF")
local b  = myLoadImage(file_path.."_B03_L1T.TIF")

local t1 = os.clock()

print("Merge Components")
local rgb_short = im.ProcessMergeComponentsNew({r, g, b})
--myPrintStats(rgb_short)

local rgb = im.ImageCreateBased(rgb_short, nil, nil, nil, im.BYTE) 
rgb:AddAlpha()

print("Find Min")
cmin, cmax = im.CalcPercentMinMax(r, 2, true)  -- 2%, and ignore_zero=true
gmin, gmax = im.CalcPercentMinMax(g, 2, true)  -- 2%, and ignore_zero=true
if (gmin < cmin) then cmin = gmin end
if (gmax > cmax) then cmax = gmax end
bmin, bmax = im.CalcPercentMinMax(b, 2, true)  -- 2%, and ignore_zero=true
if (bmin < cmin) then cmin = bmin end
if (bmax > cmax) then cmax = bmax end
--print("cmin=", cmin)
--print("cmax=", cmax)

print("Set Alpha")
rgb:SetAlpha(255)
im.ProcessSetAlphaColor(r, rgb, {0}, 0)
im.ProcessSetAlphaColor(g, rgb, {0}, 0)
im.ProcessSetAlphaColor(b, rgb, {0}, 0)
--Two ways to do the same thing, but using separate planes 
--will also eliminate those invalid colors at the edges.
--im.ProcessSetAlphaColor(rgb_short, rgb, {0, 0, 0}, 0)

-- Expand contrast
print("Tone Gamut - Expand")
--im.ProcessToneGamut(rgb_short, rgb_short, im.GAMUT_EXPAND, {cmin, cmax}); -- must compute cmin and cmax, to be the 2% of the non zero data
rgb_short:Level(cmin, cmax)
--myPrintStats(rgb_short)

-- Improve contrast using a non-linear operation, same as Gamma
print("Tone Gamut - Gamma")
--im.ProcessToneGamut(rgb_short, rgb_short, im.GAMUT_POW, {0.6});
rgb_short:Gamma(0.6)
--myPrintStats(rgb_short)

-- Other possibility
--print("Tone Gamut - Brightness and Contrast")
--im.ProcessToneGamut(rgb_short, rgb_short, im.GAMUT_BRIGHTCONT, {bright_shift, contrast_factor});   --both [-100, 100]

print("Convert Data Type")
im.ProcessConvertDataType(rgb_short, rgb, 0, 0, 0, im.CAST_MINMAX); --short to byte, scan for min-max

local t2 = os.clock()
print("time=", t2-t1)

print("Saving: "..file_path.."_rgb.png") 
rgb:Save(file_path.."_rgb.png", "PNG")

print("Done")
