-- lua5.1 hdtv_resize.lua *.jpg

require"imlua"
require"imlua_process"

err_msg = {
  "No error.",
  "Error while opening the file.",
  "Error while accessing the file.",
  "Invalid or unrecognized file format.",
  "Invalid or unsupported data.",
  "Invalid or unsupported compression.",
  "Insuficient memory",
  "Interrupted by the counter",
}

filename1 = arg[1]
if (not filename1) then
  print("Must have the file name as parameter.")
  print("  Can have more than one file name as parameters and can use wildcards.")
  print("  Usage:")
  print("    lua hdtv_resize.lua filename1 filename2 ...")
  print("    lua hdtv_resize.lua *.jpg")
  return
end

print(">>> resize of multiple images <<<")
print("")

function ProcessImageFile(file_name)
  print("Loading File: "..file_name)
  image, err = im.FileImageLoad(file_name);

  if (err and err ~= im.ERR_NONE) then
    error(err_msg[err+1])
  end
  
  local w = image:Width()
  local h = image:Height()
  local new_w = 1920
  local new_h = 1080
  if (new_w/new_h < w/h) then
    new_h = new_w * h/w
  else
    new_w = new_h * w/h
  end
  
  local new_image = im.ImageCreateBased(image, new_w, new_h)

  im.ProcessReduce(image, new_image, 1)

  print("   Saving File: "..file_name)
  new_image:Save(file_name, "JPEG")

  new_image:Destroy()
  image:Destroy()
  print("")
end

file_count = 0
for index,value in ipairs(arg) do
  ProcessImageFile(arg[index])
  file_count = file_count + 1
end

if (file_count > 1) then
  print("Processed "..file_count.." Files.")
end
