local DEMO_MODE = false

local FACES_UP = -math.pi / 2
local AIR_SPAWN_X = -700
local FIRST_WAVE_DELAY = 8
local DELAY_BETWEEN_WAVES = 6
local WAVE_CLEAR_BONUS = 4
local HUD_GOLD_LEFT = -578
local HUD_WAVE_LEFT = 540

local ENEMY_KINDS = {
    scout = {
        health = 3, speed = 150, bounty = 1, explosion_size = 0.8,
        light_intensity = 0.55, headlight_intensity = 0,
        bobs = true, rest_rotation = FACES_UP,
    },
    grunt = {
        health = 6, speed = 90, bounty = 1, explosion_size = 0.8,
        light_intensity = 0.55, headlight_intensity = 0,
        bobs = true, rest_rotation = FACES_UP,
    },
    apc = {
        health = 16, speed = 72, bounty = 2, explosion_size = 1.2,
        light_intensity = 0.55, headlight_intensity = 0,
        bobs = true, rest_rotation = FACES_UP,
    },
    tank = {
        health = 30, speed = 52, bounty = 3, explosion_size = 1.5,
        light_intensity = 0.6, headlight_intensity = 0.2,
        rest_rotation = 0,
    },
    elite = {
        health = 48, speed = 68, bounty = 4, explosion_size = 1.5,
        light_intensity = 0.95, headlight_intensity = 0.2,
        rest_rotation = 0,
    },
    bomber = {
        health = 12, speed = 125, bounty = 3, explosion_size = 1.2,
        light_intensity = 1, headlight_intensity = 0.9,
        flying = true, blinks = true, rest_rotation = 0,
    },
}

local TOWER_KINDS = {
    gun = {
        cost = 6, range = 215, fire_period = 0.34, damage = 1,
        splash_radius = 0, hits_air = true,
        turn_speed = 7.0, barrel_length = 30, rest_rotation = FACES_UP,
        projectile = "bullet", projectile_speed = 760, projectile_light = 0.8,
        fire_sound = "shoot_gun",
    },
    cannon = {
        cost = 10, range = 250, fire_period = 1.45, damage = 6,
        splash_radius = 55, hits_air = false,
        turn_speed = 3.5, barrel_length = 34, rest_rotation = FACES_UP,
        projectile = "shell", projectile_speed = 430, projectile_light = 0.9,
        fire_sound = "shoot_cannon",
    },
    missile = {
        cost = 14, range = 310, fire_period = 2.6, damage = 11,
        splash_radius = 95, hits_air = true, homing = true,
        turn_speed = 2.5, barrel_length = 30, rest_rotation = FACES_UP,
        projectile = "rocket", projectile_speed = 250, projectile_light = 1.4,
        fire_sound = "shoot_missile",
    },
}

local TEMPLATE_TAGS = {
    "scout", "grunt", "apc", "tank", "elite", "bomber",
    "bullet", "shell", "rocket", "gun", "cannon", "missile", "fx", "float",
}

local DEMO_BUILD_PLAN = {
    { at = 2, build = "gun" },
    { at = 4, build = "gun" },
    { at = 14, build = "cannon" },
    { at = 26, build = "gun" },
    { at = 38, build = "missile" },
    { at = 52, build = "cannon" },
    { at = 66, build = "missile" },
    { at = 80, build = "gun" },
}

local save = require("save")

local level = nil
local spawn_queue = {}
local time_until_wave = FIRST_WAVE_DELAY
local wave_index = 0
local air_lane_counter = 0
local banner_text = nil
local banner_age = 0
local demo_start_time = 0
local demo_step = 1

local function record_victory(level_index)
    local data = save.read()
    data.unlocked = math.min(kione.level_count, math.max(data.unlocked or 1, level_index + 1))
    if level_index >= kione.level_count then
        data.victories = (data.victories or 0) + 1
    end
    save.write(data)
end

local function build_path(waypoints)
    local segments, total_length = {}, 0
    for i = 1, #waypoints - 1 do
        local from, to = waypoints[i], waypoints[i + 1]
        local dx, dy = to[1] - from[1], to[2] - from[2]
        local length = math.sqrt(dx * dx + dy * dy)
        segments[i] = {
            x = from[1], y = from[2],
            dx = dx / length, dy = dy / length,
            length = length, start = total_length,
        }
        total_length = total_length + length
    end
    return segments, total_length
end

local function collect_pads()
    local pads = {}
    for _, pad in ipairs(kione.find_all("pad")) do
        local translation = pad:transform().translation
        table.insert(pads, { x = translation.x, y = translation.y })
    end
    table.sort(pads, function(a, b)
        if a.y ~= b.y then
            return a.y > b.y
        end
        return a.x < b.x
    end)
    return pads
end

local function collect_templates()
    local templates = {}
    for _, tag in ipairs(TEMPLATE_TAGS) do
        templates[tag] = kione.find(tag)
    end
    return templates
end

local function destroy_all(records)
    for _, record in pairs(records) do
        if record.entity:valid() then
            record.entity:destroy()
        end
    end
end

local function restart_level()
    local game = kione.game

    destroy_all(game.enemies)
    destroy_all(game.projectiles)
    game.enemies = {}
    game.projectiles = {}
    game.towers = {}

    for _, pad in ipairs(game.pads) do
        if pad.turret and pad.turret:valid() then
            pad.turret:destroy()
        end
        pad.turret, pad.built = nil, nil
    end

    game.gold = level.gold
    game.lives = level.lives
    game.wave = 0
    game.state = "build"
    game.selected_pad = 1
    game.spawn_count = 0
    game.portal_flash = 0
    game.keep_flash = 0
    game.gold_flash = 0
    game.cursor_flash = 0
    game.dawn = 0

    spawn_queue = {}
    time_until_wave = FIRST_WAVE_DELAY
    wave_index = 0
    demo_start_time = game.time or 0
    demo_step = 1

    kione.log("[TD] level " .. kione.level_index .. " '" .. level.name .. "': "
        .. level.gold .. " gold, " .. level.lives .. " crystals, " .. #level.waves .. " waves")
end

local function spawn_from_template(tag, x, y)
    local spawned = kione.game.templates[tag]:clone()
    local translation = spawned:transform().translation
    translation.x, translation.y = x, y
    return spawned
end

local function build_tower(pad_index, kind)
    local game = kione.game
    local pad = game.pads[pad_index]
    if pad.built then
        return "occupied"
    end

    local tower = TOWER_KINDS[kind]
    if game.gold < tower.cost then
        return "poor"
    end

    game.gold = game.gold - tower.cost
    pad.built = kind

    local turret = spawn_from_template(kind, pad.x, pad.y)
    turret:sprite().color.w = 1
    pad.turret = turret
    game.towers[turret:id()] = { entity = turret, kind = kind, x = pad.x, y = pad.y }

    kione.play_sound("build_chime", 0.5)
    kione.log("[TD] built " .. kind .. " at pad " .. pad_index .. " (gold=" .. game.gold .. ")")
    return nil
end

local function spawn_explosion(x, y, size)
    kione.play_sound("explosion", math.min(1, 0.3 + size * 0.35), 0.85 + math.random() * 0.3)

    local explosion = spawn_from_template("fx", x, y)
    local transform = explosion:transform()
    transform.scale.x, transform.scale.y = size, size
    explosion:sprite().color.w = 1
    explosion:animation():play("boom")

    local light = explosion:point_light()
    light.radius = 150 * size
    light.intensity = 2.4 * size
end

local function spawn_floating_text(x, y, message, red, green, blue)
    local popup = spawn_from_template("float", x, y)
    local text = popup:text()
    text.text = message
    local color = text.color
    color.x, color.y, color.z, color.w = red or 1, green or 0.9, blue or 0.55, 1
end

local function initialize()
    level = kione.levels[kione.level_index]

    local path, path_length = build_path(level.waypoints)
    local keep = level.waypoints[#level.waypoints]

    kione.game = {
        config = {
            path = path,
            path_length = path_length,
            keep_position = { x = keep[1], y = keep[2] },
            enemy_kinds = ENEMY_KINDS,
            tower_kinds = TOWER_KINDS,
        },
        pads = collect_pads(),
        templates = collect_templates(),
        enemies = {},
        towers = {},
        projectiles = {},
        hud = {
            gold = kione.find("hud_gold"),
            wave = kione.find("hud_wave"),
            banner = kione.find("hud_banner"),
            coin = kione.find("hud_coin"),
        },
        time = 0,
        spawn = spawn_from_template,
        build_tower = build_tower,
        spawn_explosion = spawn_explosion,
        spawn_floating_text = spawn_floating_text,
    }

    restart_level()
    kione.game.ready = true
end

local function start_wave()
    local game = kione.game
    wave_index = wave_index + 1
    game.wave = wave_index
    game.state = "wave"

    spawn_queue = {}
    for stream, spec in ipairs(level.waves[wave_index]) do
        local kind, count, interval = spec[1], spec[2], spec[3]
        for n = 0, count - 1 do
            table.insert(spawn_queue, { at = game.time + (stream - 1) * 0.5 + n * interval, kind = kind })
        end
    end
    table.sort(spawn_queue, function(a, b) return a.at < b.at end)

    kione.play_sound("horn", 0.6)
    kione.log("[TD] wave " .. wave_index .. "/" .. #level.waves .. " incoming (" .. #spawn_queue .. " enemies)")
end

local function spawn_enemy(kind)
    local game = kione.game
    local stats = ENEMY_KINDS[kind]

    local x, y
    if stats.flying then
        air_lane_counter = air_lane_counter + 1
        x = AIR_SPAWN_X
        y = level.lanes[(air_lane_counter % #level.lanes) + 1]
    else
        x, y = level.waypoints[1][1], level.waypoints[1][2]
    end

    game.spawn_count = game.spawn_count + 1
    local depth = (stats.flying and 3.2 or 0.5) + (game.spawn_count % 40) * 0.005

    local enemy = spawn_from_template(kind, x, y)
    game.enemies[enemy:id()] = {
        entity = enemy,
        kind = kind,
        health = stats.health,
        max_health = stats.health,
        speed = stats.speed,
        bounty = stats.bounty,
        flying = stats.flying,
        origin_x = x,
        origin_y = y,
        x = x,
        y = y,
        progress = 0,
        z = depth,
    }
    game.portal_flash = 1
end

local function advance_campaign()
    if kione.level_index < kione.level_count then
        kione.load_scene("level" .. (kione.level_index + 1))
    else
        kione.load_scene("title")
    end
end

local function run_demo_builder()
    local game = kione.game
    local step = DEMO_BUILD_PLAN[demo_step]
    if not step or game.time - demo_start_time < step.at then
        return
    end

    for offset = 0, #game.pads - 1 do
        local pad_index = ((demo_step + offset - 1) % #game.pads) + 1
        if not game.pads[pad_index].built then
            if build_tower(pad_index, step.build) == "poor" then
                return
            end
            break
        end
    end
    demo_step = demo_step + 1
end

local function update_demo(dt)
    local game = kione.game
    run_demo_builder()

    if game.state ~= "victory" and game.state ~= "defeat" then
        game.demo_end_timer = 0
        return
    end

    game.demo_end_timer = (game.demo_end_timer or 0) + dt
    if game.demo_end_timer > 4 then
        game.demo_end_timer = 0
        if game.state == "victory" then
            advance_campaign()
        else
            restart_level()
        end
    end
end

local function update_ambient_light(entity, dt)
    local game = kione.game
    local environment = entity:environment()
    if not environment then
        return
    end

    local night_depth = math.min(1, math.max(0, (game.wave - 1) / math.max(1, #level.waves - 1)))
    local base = level.ambient or { 0.5, 0.5, 0.58, 0.46 }
    local red = base[1] + (0.26 - base[1]) * night_depth * 0.8
    local green = base[2] + (0.27 - base[2]) * night_depth * 0.8
    local blue = base[3] + (0.40 - base[3]) * night_depth * 0.8
    local intensity = base[4] + (0.32 - base[4]) * night_depth * 0.8

    if game.state == "victory" then
        game.dawn = math.min(1, (game.dawn or 0) + dt * 0.2)
        red = red + (1.05 - red) * game.dawn
        green = green + (0.86 - green) * game.dawn
        blue = blue + (0.62 - blue) * game.dawn
        intensity = intensity + (0.95 - intensity) * game.dawn
    elseif game.state == "defeat" then
        red, green, blue = 0.45, 0.13, 0.11
        intensity = 0.32
    else
        game.dawn = 0
    end

    local color = environment.ambient_color
    color.x, color.y, color.z = red, green, blue
    environment.ambient_intensity = intensity
end

local function banner_message()
    local game = kione.game
    if game.state == "build" then
        local label = wave_index == 0 and "FIRST WAVE IN " or "NEXT WAVE IN "
        return label .. math.ceil(math.max(0, time_until_wave))
    elseif game.state == "wave" then
        return "WAVE " .. wave_index
    elseif game.state == "victory" then
        if kione.level_index < kione.level_count then
            return "REGION SECURED - CLICK FOR THE NEXT MARCH"
        end
        return "DAWN BREAKS - THE CAMPAIGN IS WON - CLICK FOR TITLE"
    end
    return "THE CRYSTALS ARE GONE - [R] RETRY, CLICK FOR TITLE"
end

local function set_banner(message)
    if banner_text ~= message then
        banner_text = message
        banner_age = 0
        kione.game.hud.banner:text().text = message
    end
end

local function update_hud(dt)
    local game = kione.game
    local flash = game.gold_flash

    local gold_text = game.hud.gold:text()
    gold_text.text = tostring(math.max(0, game.gold))
    local gold_color = gold_text.color
    gold_color.x, gold_color.y, gold_color.z = 1, 0.9 - 0.7 * flash, 0.55 - 0.4 * flash
    game.hud.gold:transform().translation.x = HUD_GOLD_LEFT + game.hud.gold:text_size() / 2

    local coin = game.hud.coin:sprite()
    if coin then
        coin.color.y, coin.color.z = 1 - 0.6 * flash, 1 - 0.8 * flash
    end

    local wave_text = game.hud.wave:text()
    wave_text.text = game.wave .. "/" .. #level.waves
    game.hud.wave:transform().translation.x = HUD_WAVE_LEFT + game.hud.wave:text_size() / 2

    set_banner(banner_message())
    banner_age = banner_age + dt
    local banner_alpha = 1
    if game.state == "wave" then
        banner_alpha = math.max(0, math.min(1, 1 - (banner_age - 2.5) / 0.8))
    end
    game.hud.banner:text().color.w = banner_alpha
end

local function fade_flashes(dt)
    local game = kione.game
    game.portal_flash = math.max(0, game.portal_flash - dt * 3)
    game.keep_flash = math.max(0, game.keep_flash - dt * 1.5)
    game.gold_flash = math.max(0, game.gold_flash - dt * 2.5)
    game.cursor_flash = math.max(0, game.cursor_flash - dt * 2.5)
end

local function update_build_phase(dt)
    local game = kione.game
    time_until_wave = time_until_wave - dt
    if time_until_wave > 0 then
        return
    end

    if wave_index >= #level.waves then
        game.state = "victory"
        record_victory(kione.level_index)
        kione.play_sound("victory_sting", 0.7)
        kione.log("[TD] VICTORY on level " .. kione.level_index)
    else
        start_wave()
    end
end

local function update_wave_phase()
    local game = kione.game

    local waiting = {}
    for _, entry in ipairs(spawn_queue) do
        if game.time >= entry.at then
            spawn_enemy(entry.kind)
        else
            table.insert(waiting, entry)
        end
    end
    spawn_queue = waiting

    if game.lives <= 0 then
        game.state = "defeat"
        spawn_queue = {}
        kione.play_sound("horn", 0.7, 0.5)
        kione.log("[TD] DEFEAT on level " .. kione.level_index .. " at wave " .. wave_index)
    elseif #spawn_queue == 0 and next(game.enemies) == nil then
        game.gold = game.gold + WAVE_CLEAR_BONUS
        game.state = "build"
        time_until_wave = (wave_index >= #level.waves) and 3 or DELAY_BETWEEN_WAVES
        kione.log("[TD] wave " .. wave_index .. " cleared, +" .. WAVE_CLEAR_BONUS
            .. " gold (" .. game.gold .. ")")
    end
end

function on_event(entity, event)
    if not kione.game or not kione.game.ready then
        return
    end

    if event.type == EventType.key and event.key == Key.r and event.state == InputState.press then
        restart_level()
        return true
    end

    if event.type == EventType.mouse_button and event.state == InputState.press then
        if kione.game.state == "victory" then
            advance_campaign()
            return true
        elseif kione.game.state == "defeat" then
            kione.load_scene("title")
            return true
        end
    end
end

function on_update(entity, dt)
    if not level then
        if kione.levels and kione.level_index then
            initialize()
        end
        return
    end
    if not kione.game.ready then
        return
    end

    kione.game.time = kione.game.time + dt
    fade_flashes(dt)

    if DEMO_MODE then
        update_demo(dt)
    end

    if kione.game.state == "build" then
        update_build_phase(dt)
    elseif kione.game.state == "wave" then
        update_wave_phase()
    end

    update_ambient_light(entity, dt)
    update_hud(dt)
end
