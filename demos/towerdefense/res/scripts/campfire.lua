local flicker_time = 0
local rest_intensity = 1

function on_create(entity)
    flicker_time = math.random() * 6.28
    rest_intensity = entity:point_light().intensity
end

function on_update(entity, dt)
    flicker_time = flicker_time + dt

    local flicker = 0.78
        + 0.14 * math.sin(flicker_time * 7.3)
        + 0.08 * math.sin(flicker_time * 12.9 + 1.7)
        + 0.06 * math.sin(flicker_time * 23.7 + 0.4)
    entity:point_light().intensity = rest_intensity * flicker

    local scale = entity:transform().scale
    local size = 1 + 0.06 * math.sin(flicker_time * 9.1)
    scale.x, scale.y = size, size
end
