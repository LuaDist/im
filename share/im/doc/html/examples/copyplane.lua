require"imlua"
require"imlua_process"
image,error = im.FileImageLoad("flower.jpg")
image:AddAlpha()
width = image:Width() height = image:Height()
r=image[0] 
g=image[1] 
b=image[2] 
t=image[3]
alpha_image = im.ImageCreateBased(image, nil, nil, im.GRAY, nil)
im.ProcessRenderConstant(alpha_image, {192})
alpha_image:CopyPlane(0, image, 3)
alpha_image:Destroy()
