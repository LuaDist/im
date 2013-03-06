require"imlua"
require"imlua_process"

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

print("Loading:", "lena.jpg") 
local img  =  im.FileImageLoad("lena.jpg");

myPrintStats(img)
