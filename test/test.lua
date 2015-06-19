package.path = package.path..";../lib/?.lua"
package.cpath = package.cpath..";../?.so"

local tbl = require "tbl"
local audio = require "audio"

assert(audio.init())

local a = audio.load("/Users/wale/loginScene.mp3")
local a2 = audio.load("/Users/wale/beiji.wav")
print (a.gain)
a.gain = 0.5
a.loop = true
print (a.gain)
print (a.loop)
a:play()

a2.loop = true
a2:play()

while (true) do
end
