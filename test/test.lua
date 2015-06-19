package.path = package.path..";../lib/?.lua"
package.cpath = package.cpath..";../?.so"

local tbl = require "tbl"
local audio = require "audio"

assert(audio.init())

local a = audio.load("/Users/wale/loginScene.mp3")
local a = audio.load("/Users/wale/beiji.wav")
print (a.gain)
a.gain = 0.1
print (a.gain)

a:play()

while (true) do
end
