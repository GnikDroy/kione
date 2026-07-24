local ENEMY_COLLISION_LAYER = 1
local MISSILE_RACK_LOADED_X = 21 * 64
local MISSILE_RACK_EMPTY_X = 22 * 64

local tower, stats
local cooldown = 0
local aim_angle = 0
local muzzle_flash = 0

function on_create(entity)
    tower = kione.game and kione.game.towers and kione.game.towers[entity:id()] or nil
    if not tower then
        return
    end

    stats = kione.game.config.tower_kinds[tower.kind]
    aim_angle = math.random() * 6.28
    entity:sprite().color.w = 1
end

local function find_furthest_enemy_in_range()
    local best, best_progress = nil, -1
    for _, found in ipairs(kione.query_circle(tower.x, tower.y, stats.range, ENEMY_COLLISION_LAYER)) do
        local enemy = kione.game.enemies[found:id()]
        local targetable = enemy and enemy.health > 0 and (stats.hits_air or not enemy.flying)
        if targetable and enemy.progress > best_progress then
            best, best_progress = enemy, enemy.progress
        end
    end
    return best
end

local function fire_at(target)
    local muzzle_x = tower.x + math.cos(aim_angle) * stats.barrel_length
    local muzzle_y = tower.y + math.sin(aim_angle) * stats.barrel_length
    kione.play_sound(stats.fire_sound, 0.4, 0.9 + math.random() * 0.2)

    local projectile = kione.game.spawn(stats.projectile, muzzle_x, muzzle_y)
    projectile:sprite().color.w = 1
    projectile:point_light().intensity = stats.projectile_light
    kione.game.projectiles[projectile:id()] = {
        entity = projectile,
        target = target,
        x = muzzle_x,
        y = muzzle_y,
        target_x = target.x,
        target_y = target.y,
        angle = aim_angle,
        damage = stats.damage,
        splash_radius = stats.splash_radius,
        speed = stats.projectile_speed,
        hits_air = stats.hits_air,
        homing = stats.homing,
        lifetime = 3,
    }

    cooldown = stats.fire_period
    muzzle_flash = 2.4
end

function on_update(entity, dt)
    if not tower then
        return
    end

    cooldown = math.max(0, cooldown - dt)
    muzzle_flash = math.max(0, muzzle_flash - dt * 8)
    entity:point_light().intensity = 0.3 + muzzle_flash

    if tower.kind == "missile" then
        local reloading = cooldown > stats.fire_period * 0.4
        entity:sprite().region.x = reloading and MISSILE_RACK_EMPTY_X or MISSILE_RACK_LOADED_X
    end

    local target = find_furthest_enemy_in_range()
    if not target then
        return
    end

    local desired = math.atan(target.y - tower.y, target.x - tower.x)
    local misalignment = desired - aim_angle
    while misalignment > math.pi do misalignment = misalignment - 2 * math.pi end
    while misalignment < -math.pi do misalignment = misalignment + 2 * math.pi end

    local max_turn = stats.turn_speed * dt
    aim_angle = aim_angle + math.max(-max_turn, math.min(max_turn, misalignment))
    entity:transform().angle = aim_angle + stats.rest_rotation

    if cooldown <= 0 and math.abs(misalignment) < 0.25 then
        fire_at(target)
    end
end
