function on_update(entity, dt)
    local game = kione.game
    if not game or not game.ready then
        return
    end

    local flash = game.keep_flash
    local pulse = 1.05 + 0.25 * math.sin(game.time * 1.8)
    local red, green, blue = 0.45, 0.9, 1
    if game.state == "victory" then
        red, green, blue = 1, 0.85, 0.5
        pulse = pulse + 0.5
    elseif game.state == "defeat" then
        red, green, blue = 0.7, 0.15, 0.1
        pulse = 0.4
    end
    red = red + (1 - red) * flash
    green = green * (1 - flash * 0.8)
    blue = blue * (1 - flash * 0.85)

    local light = entity:point_light()
    local light_color = light.color
    light_color.x, light_color.y, light_color.z = red, green, blue
    light.intensity = pulse + flash * 2

    local sprite = entity:sprite()
    if sprite then
        local color = sprite.color
        color.y, color.z = 1 - flash * 0.6, 1 - flash * 0.6
    end
end
