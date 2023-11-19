--- @meta

--- @class system
local system = {}

---Get current system name
---@return string
function system.getOS() end

--- @class selene
--- @field args table
selene = {}

--- Get current selene version
--- @return string
function selene.getVersion() end

--- Returns true if selene is actually running
--- @return boolean
function selene.isRunning() end

--- Set false to stop selene main loop
--- @param value boolean
function selene.setRunning(value) end
