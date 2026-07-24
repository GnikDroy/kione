local RISE_DISTANCE = 32
local RISE_DURATION = 0.75

function on_create(entity)
    local text = entity:text()
    if not text or text.color.w <= 0 then
        return
    end

    local translation = entity:transform().translation
    kione.tween(translation, "y", translation.y + RISE_DISTANCE, RISE_DURATION, "out", entity)
    kione.tween(text.color, "w", 0, RISE_DURATION, "in", entity)
    kione.after(RISE_DURATION + 0.05, function()
        if entity:valid() then
            entity:destroy()
        end
    end)
end
