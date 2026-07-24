local crystal_index
local visibility = 1

function on_create(entity)
    crystal_index = tonumber(string.match(entity.tag, "(%d+)$"))
end

function on_update(entity, dt)
    local game = kione.game
    if not game or not game.ready then
        return
    end

    local target = (game.lives >= crystal_index) and 1 or 0
    visibility = visibility + (target - visibility) * math.min(1, dt * 4)

    local flash = game.keep_flash
    local color = entity:sprite().color
    color.x = 0.55 + 0.45 * flash
    color.y = 0.85 - 0.5 * flash
    color.z = 1 - 0.6 * flash
    color.w = 0.25 + 0.75 * visibility
end
