local ENEMY_COLLISION_LAYER = 1
local REST_ROTATIONS = { bullet = 0, shell = 0, rocket = -math.pi / 2 }

local projectile, entity_id
local rest_rotation = 0

function on_create(entity)
    projectile = kione.game and kione.game.projectiles and kione.game.projectiles[entity:id()] or nil
    if not projectile then
        return
    end

    entity_id = entity:id()
    rest_rotation = REST_ROTATIONS[entity.tag] or 0
    entity:transform().angle = projectile.angle + rest_rotation
end

local function target_is_alive()
    local target = projectile.target
    return target ~= nil and target.entity:valid() and target.health > 0
end

local function expire(entity)
    kione.game.projectiles[entity_id] = nil
    entity:destroy()
end

local function apply_splash_damage()
    local direct_hit = target_is_alive() and projectile.target or nil
    local victims = kione.query_circle(projectile.x, projectile.y, projectile.splash_radius, ENEMY_COLLISION_LAYER)
    for _, found in ipairs(victims) do
        local victim = kione.game.enemies[found:id()]
        if victim and victim.health > 0 and (projectile.hits_air or not victim.flying) then
            local damage = victim == direct_hit and projectile.damage or projectile.damage * 0.6
            victim.health = victim.health - damage
        end
    end
end

local function detonate(entity)
    if projectile.splash_radius > 0 then
        apply_splash_damage()
        kione.game.spawn_explosion(projectile.x, projectile.y, projectile.splash_radius > 70 and 1.3 or 1.0)
    else
        if target_is_alive() then
            projectile.target.health = projectile.target.health - projectile.damage
        end
        kione.game.spawn_explosion(projectile.x, projectile.y, 0.45)
    end
    expire(entity)
end

local function steer(dt)
    local desired = math.atan(projectile.target_y - projectile.y, projectile.target_x - projectile.x)
    if not projectile.homing then
        projectile.angle = desired
        return
    end

    local turn = desired - projectile.angle
    while turn > math.pi do turn = turn - 2 * math.pi end
    while turn < -math.pi do turn = turn + 2 * math.pi end
    projectile.angle = projectile.angle + math.max(-4.5 * dt, math.min(4.5 * dt, turn))
    projectile.speed = projectile.speed * (1 + 1.6 * dt)
end

function on_fixed_update(entity, dt)
    if not projectile then
        return
    end

    projectile.lifetime = projectile.lifetime - dt
    if projectile.lifetime <= 0 then
        expire(entity)
        return
    end

    if target_is_alive() then
        projectile.target_x, projectile.target_y = projectile.target.x, projectile.target.y
    end
    steer(dt)

    local dx = projectile.target_x - projectile.x
    local dy = projectile.target_y - projectile.y
    local distance_to_target = math.sqrt(dx * dx + dy * dy)
    local step = projectile.speed * dt
    if step >= distance_to_target then
        projectile.x, projectile.y = projectile.target_x, projectile.target_y
        detonate(entity)
        return
    end
    projectile.x = projectile.x + math.cos(projectile.angle) * step
    projectile.y = projectile.y + math.sin(projectile.angle) * step

    local transform = entity:transform()
    transform.translation.x, transform.translation.y = projectile.x, projectile.y
    transform.angle = projectile.angle + rest_rotation

    if target_is_alive() and entity:overlaps(projectile.target.entity) then
        detonate(entity)
    end
end
