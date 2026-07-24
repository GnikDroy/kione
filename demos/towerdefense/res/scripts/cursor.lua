local BUILD_KEYS = {
    ["1"] = "gun", ["2"] = "cannon", ["3"] = "missile",
    kp_1 = "gun", kp_2 = "cannon", kp_3 = "missile",
}
local PAD_HOVER_RADIUS = 70
local HOVER_DEADZONE = 4

local light_x, light_y
local hover_x, hover_y

function on_create(entity)
    local translation = entity:transform().translation
    light_x, light_y = translation.x, translation.y
end

local function select_hovered_pad(event)
    if hover_x and math.abs(event.x - hover_x) + math.abs(event.y - hover_y) < HOVER_DEADZONE then
        return
    end
    hover_x, hover_y = event.x, event.y

    local world_x, world_y = kione.screen_to_world(event.x, event.y)
    for index, pad in ipairs(kione.game.pads) do
        local dx, dy = pad.x - world_x, pad.y - world_y
        if dx * dx + dy * dy <= PAD_HOVER_RADIUS * PAD_HOVER_RADIUS then
            kione.game.selected_pad = index
            return
        end
    end
end

local function try_build(kind)
    local game = kione.game
    local failure = game.build_tower(game.selected_pad, kind)
    if failure == "poor" then
        game.gold_flash = 1
        game.cursor_flash = 1
        kione.play_sound("deny", 0.5)
    elseif failure == "occupied" then
        game.cursor_flash = 1
    end
end

function on_event(entity, event)
    if not kione.game or not kione.game.ready then
        return
    end

    if event.type == EventType.cursor_position then
        select_hovered_pad(event)
        return
    end
    if event.type ~= EventType.key or event.state ~= InputState.press then
        return
    end

    local game = kione.game
    if event.key == Key.right or event.key == Key.d then
        game.selected_pad = game.selected_pad % #game.pads + 1
        return true
    end
    if event.key == Key.left or event.key == Key.a then
        game.selected_pad = (game.selected_pad - 2) % #game.pads + 1
        return true
    end

    local kind = BUILD_KEYS[event.key]
    if kind then
        try_build(kind)
        return true
    end
end

local function draw_range_preview(pad)
    local range = kione.game.config.tower_kinds[pad.built].range
    kione.draw_circle(pad.x, pad.y, range, { color = { 0.55, 0.85, 1.0, 0.08 }, z = 3.3 })
    kione.draw_circle(pad.x, pad.y, range,
        { filled = false, thickness = 3, color = { 0.55, 0.85, 1.0, 0.5 }, z = 3.4 })
end

function on_update(entity, dt)
    local game = kione.game
    if not game or not game.ready then
        return
    end

    local pad = game.pads[game.selected_pad]
    if pad.built then
        draw_range_preview(pad)
    end

    light_x = light_x + (pad.x - light_x) * math.min(1, dt * 14)
    light_y = light_y + (pad.y - light_y) * math.min(1, dt * 14)
    local position = entity:transform().translation
    position.x, position.y, position.z = light_x, light_y, 3.5

    local flash = game.cursor_flash
    local pulse = 52 + 4 * math.sin(game.time * 5)
    local red, green, blue, alpha
    if pad.built then
        red, green, blue, alpha = 0.6 + 0.4 * flash, 0.85 - 0.5 * flash, 1 - 0.7 * flash, 0.6
    else
        red, green, blue, alpha = 1, 1 - 0.6 * flash, 1 - 0.75 * flash, 0.9
    end
    kione.draw_circle(pad.x, pad.y, pulse,
        { filled = false, thickness = 4, color = { red, green, blue, alpha }, z = 3.5, unlit = true })
    kione.draw_circle(pad.x, pad.y, pulse * 0.55,
        { color = { red, green, blue, 0.12 }, z = 3.45, unlit = true })

    local light = entity:point_light()
    light.intensity = 0.5 + 0.15 * math.sin(game.time * 5) + flash
    local light_color = light.color
    light_color.x, light_color.y, light_color.z = 1, 1 - 0.5 * flash, 1 - 0.6 * flash
end
