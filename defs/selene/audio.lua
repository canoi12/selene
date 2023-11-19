--- @meta

--- @class selene.audio
local audio = {}

--- @class selene.audio.Decoder
local Decoder = {}

--- Load decoder from either .ogg or .wav file
--- @param path string
--- @return selene.audio.Decoder
function Decoder.load(path) end

--- Close decoder
function Decoder:close() end

--- Clone decoder
--- @return selene.audio.Decoder
function Decoder:clone() end
 
--- Set current Decoder offset
--- @param index integer
function Decoder:seek(index) end
function Decoder:readS16(out, len) end
function Decoder:readF32(out, len) end
function Decoder:getChunk(out, len) end

function Decoder:getSampleRate() end
function Decoder:getChannels() end
function Decoder:getBitDepth() end
function Decoder:getFrameCount() end