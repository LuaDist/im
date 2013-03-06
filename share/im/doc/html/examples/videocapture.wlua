local im = require "imlua"
require "imlua_capture"
local gl = require "luagl"
local iup = require "iuplua"
require "iupluagl"

local vc = im.VideoCaptureCreate()
if (not vc) then
  error("ERROR: No capture device found!")
end

if (vc:Connect(0) == 0) then
  error("ERROR: Fail to connect to capture device")
end

local capw, caph = vc:GetImageSize()
local initimgsize = string.format("%ix%i", capw, caph)


local frbuf = im.ImageCreate(capw, caph, im.RGB, im.BYTE)
local gldata, glformat = frbuf:GetOpenGLData()

cnv = iup.glcanvas{buffer="DOUBLE", rastersize = initimgsize}

function cnv:resize_cb(width, height)
  iup.GLMakeCurrent(self)
  gl.Viewport(0, 0, width, height)
end

function cnv:action(x, y)
  iup.GLMakeCurrent(self)
  gl.PixelStore(gl.UNPACK_ALIGNMENT, 1)

  gldata, glformat = frbuf:GetOpenGLData() --update the data
  gl.DrawPixelsRaw (capw, caph, glformat, gl.UNSIGNED_BYTE, gldata)

  iup.GLSwapBuffers(self)
end


vc:Live(1)

local dlg = iup.dialog{title = "Oh Snap!", cnv}

local in_loop = true

function dlg:close_cb()
  in_loop = false
end

function dlg:k_any(c)
  if c == iup.K_q or c == iup.K_ESC then
    return iup.CLOSE
  end

  if c == iup.K_F1 then
    if fullscreen then
      fullscreen = false
      dlg.fullscreen = "No"
    else
      fullscreen = true
      dlg.fullscreen = "Yes"
    end
    iup.SetFocus(cnv)
  end
end

dlg:show()
iup.SetFocus(cnv)

while in_loop do
  vc:Frame(frbuf,-1)
  iup.Update(cnv)
  local result = iup.LoopStep()
  if result == iup.CLOSE then
    in_loop = false
  end
end

vc:Live(0)
vc:Disconnect()
vc:Destroy()
