local record Asset<T>
    getName: function(Asset) : string
    getPath: function(Asset) : string
    load: function(Asset) : T
end

return Asset