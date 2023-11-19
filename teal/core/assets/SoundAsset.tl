local Sound = require('core.audio.Sound')
local record SoundAsset
    name: string
    path: string

    getName: function(SoundAsset) : string
    getPath: function(SoundAsset) : string
    load: function(SoundAsset) : Sound
end

function SoundAsset:getName() : string
    return self.name
end

function SoundAsset:getPath() : string
    return self.path
end

return SoundAsset