local spin_angle = 0

function on_update(entity, dt)
    local game = kione.game
    if not game or not game.ready then
        return
    end

    spin_angle = spin_angle + dt * 0.9
    local flash = game.portal_flash or 0
    local pulse = 1.1 + 0.3 * math.sin(game.time * 2.3)
    if game.state == "victory" then
        pulse = math.max(0, pulse - (game.dawn or 0) * 1.4)
    end

    local transform = entity:transform()
    transform.angle = spin_angle
    local wobble = 1 + 0.08 * math.sin(game.time * 2.3)
    transform.scale.x, transform.scale.y = wobble, wobble

    entity:point_light().intensity = pulse + flash * 2.4
    entity:sprite().color.w = 0.55 + 0.15 * math.sin(game.time * 2.3) + 0.3 * flash

    local x, y = transform.translation.x, transform.translation.y
    local ring_alpha = 0.22 + 0.1 * math.sin(game.time * 2.3) + 0.4 * flash
    kione.draw_circle(x, y, 46 + 6 * math.sin(game.time * 1.7),
        { filled = false, thickness = 3, color = { 0.7, 0.4, 1, ring_alpha }, z = 0.34 })
    kione.draw_circle(x, y, 66 + 8 * math.sin(game.time * 1.7 + 1.2),
        { filled = false, thickness = 2, color = { 0.55, 0.3, 0.9, ring_alpha * 0.7 }, z = 0.34 })
    kione.draw_circle(x, y, 40,
        { color = { 0.4, 0.2, 0.7, 0.10 + 0.15 * flash }, z = 0.33 })
end
