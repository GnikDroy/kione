function on_update(entity, dt)
    local light = entity:point_light()
    light.intensity = math.max(0, light.intensity - dt * 7)

    local animation = entity:animation()
    if animation.playing then
        local scale = entity:transform().scale
        scale.x = scale.x * (1 + dt * 1.6)
        scale.y = scale.y * (1 + dt * 1.6)
    end
    if animation.finished then
        entity:destroy()
    end
end
