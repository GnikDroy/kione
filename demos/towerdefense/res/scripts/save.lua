local SAVE_FILE = "crystal_keep.save"

local save = {}

function save.read()
    local file = io.open(SAVE_FILE, "r")
    if not file then
        return {}
    end

    local content = file:read("*a")
    file:close()

    local ok, data = pcall(load("return " .. content))
    return (ok and type(data) == "table") and data or {}
end

function save.write(data)
    local file = io.open(SAVE_FILE, "w")
    if not file then
        return
    end

    file:write(string.format("{ unlocked = %d, victories = %d }",
        data.unlocked or 1, data.victories or 0))
    file:close()
end

return save
