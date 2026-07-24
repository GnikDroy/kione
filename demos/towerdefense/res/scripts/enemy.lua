local enemy, stats, entity_id
local rest_scale_y = 1
local bob_phase = 0

local function point_along_path(path, distance)
    for i = #path, 1, -1 do
        local segment = path[i]
        if distance >= segment.start then
            local along = math.min(distance - segment.start, segment.length)
            return segment.x + segment.dx * along,
                segment.y + segment.dy * along,
                math.atan(segment.dy, segment.dx)
        end
    end
    return path[1].x, path[1].y, 0
end

local function turn_toward(current, desired, max_turn)
    local delta = desired - current
    while delta > math.pi do delta = delta - 2 * math.pi end
    while delta < -math.pi do delta = delta + 2 * math.pi end
    return current + math.max(-max_turn, math.min(max_turn, delta))
end

function on_create(entity)
    enemy = kione.game and kione.game.enemies and kione.game.enemies[entity:id()] or nil
    if not enemy then
        return
    end

    entity_id = entity:id()
    stats = kione.game.config.enemy_kinds[enemy.kind]
    rest_scale_y = entity:transform().scale.y
    bob_phase = math.random() * 6.28

    entity:sprite().color.w = 1
    for _, child in ipairs(entity:children()) do
        local child_sprite = child:sprite()
        if child_sprite then
            child_sprite.color.w = 1
        end
    end
    entity:point_light().intensity = stats.light_intensity
    local headlight = entity:spot_light()
    if headlight then
        headlight.intensity = stats.headlight_intensity
    end

    if enemy.flying then
        local keep = kione.game.config.keep_position
        local dx, dy = keep.x - enemy.x, keep.y - enemy.y
        enemy.flight_distance = math.sqrt(dx * dx + dy * dy)
        enemy.heading = math.atan(dy, dx)
    else
        local _, _, heading = point_along_path(kione.game.config.path, 0)
        enemy.heading = heading
    end
end

local function remove(entity)
    kione.game.enemies[entity_id] = nil
    entity:destroy()
end

local function die(entity)
    kione.game.gold = kione.game.gold + enemy.bounty
    kione.game.spawn_explosion(enemy.x, enemy.y, stats.explosion_size)
    kione.game.spawn_floating_text(enemy.x, enemy.y + 30, "+" .. enemy.bounty .. "g")
    remove(entity)
end

local function reach_keep(entity)
    local game = kione.game
    game.lives = game.lives - 1
    game.keep_flash = 1
    kione.play_sound("shatter", 0.8)

    local keep = game.config.keep_position
    game.spawn_floating_text(keep.x, keep.y + 60, "-1", 1, 0.35, 0.3)
    kione.log("[TD] leak: " .. enemy.kind .. " reached the keep (crystals=" .. game.lives .. ")")
    remove(entity)
end

local function draw_health_bar()
    local fraction = enemy.health / enemy.max_health
    local bar_y = enemy.y + 36
    kione.draw_rect(enemy.x, bar_y, 38, 6,
        { color = { 0, 0, 0, 0.6 }, z = 4.0, unlit = true })
    kione.draw_rect(enemy.x - 18 + 17 * fraction, bar_y, 34 * fraction, 4,
        { color = { 1 - fraction, fraction * 0.9, 0.15, 0.9 }, z = 4.05, unlit = true })
end

function on_fixed_update(entity, dt)
    if not enemy then
        return
    end
    if enemy.health <= 0 then
        die(entity)
        return
    end

    enemy.progress = enemy.progress + enemy.speed * dt
    if enemy.flying then
        if enemy.progress >= enemy.flight_distance then
            reach_keep(entity)
            return
        end
        enemy.x = enemy.origin_x + math.cos(enemy.heading) * enemy.progress
        enemy.y = enemy.origin_y + math.sin(enemy.heading) * enemy.progress
    else
        if enemy.progress >= kione.game.config.path_length then
            reach_keep(entity)
            return
        end
        local x, y, path_heading = point_along_path(kione.game.config.path, enemy.progress)
        enemy.x, enemy.y = x, y
        enemy.heading = turn_toward(enemy.heading, path_heading, 6 * dt)
    end

    local transform = entity:transform()
    local position = transform.translation
    position.x, position.y, position.z = enemy.x, enemy.y, enemy.z
    transform.angle = enemy.heading + stats.rest_rotation

    if stats.bobs then
        bob_phase = bob_phase + dt * 9
        transform.scale.y = rest_scale_y * (1 + 0.07 * math.sin(bob_phase))
    end
    if stats.blinks then
        entity:point_light().intensity = (math.sin(kione.game.time * 6) > 0.2) and 1.2 or 0.1
    end
end

function on_update(entity, dt)
    if not enemy then
        return
    end

    if enemy.flying then
        kione.draw_circle(enemy.x + 22, enemy.y - 30, 26, { color = { 0, 0, 0, 0.28 }, z = 0.28 })
    end
    if enemy.health < enemy.max_health and enemy.health > 0 then
        draw_health_bar()
    end
end
