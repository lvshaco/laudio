local c = require "audio.c"

local audio = {}
audio.init = assert(c.init)

local __pool = {}

function audio.load(name)
    local a = assert(c.load(name))
    __pool[name] = a
    return a
end

function audio.unload(name)
    local a = __pool[name]
    a:unload()
end

function audio.play(name, volume)
    local a = __pool[name]
    a:play(volume or 1.0)
end

function audio.stop(name)
    local a = __pool[name]
    a:stop()
end

function audio.pause(name)
    local a = __pool[name]
    a:pause()
end

function audio.resume(name)
    local a = __pool[name]
    a:resume()
end

function audio.rewind(name)
    local a = __pool[name]
    a:rewind()
end

return audio
