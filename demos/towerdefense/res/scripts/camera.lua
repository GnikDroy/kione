function on_update(entity, dt)
    local game = kione.game
    if not game or not game.ready then
        return
    end
    local camera = entity:camera()
    if not camera then
        return
    end

    local shake = game.keep_flash * game.keep_flash * 7
    camera:look_at((math.random() - 0.5) * 2 * shake, (math.random() - 0.5) * 2 * shake)
end
