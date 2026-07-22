local MOVE_SPEED = 220.0
local JUMP_SPEED = 430.0
local GRAVITY = 1400.0
local MAX_FALL = 1200.0
local COIN_TAG = "Coin"

local vx, vy = 0.0, 0.0
local grounded = false
local jump_held = false
local coins = 0

local function half_extents(collider)
    local shape = collider.shape
    if shape == "box" then
        return collider.width * 0.5, collider.height * 0.5
    elseif shape == "circle" then
        return collider.radius, collider.radius
    end
    return collider.radius, collider.radius + collider.half_height -- pill
end

local function solids_at(self_id, cx, cy, hw, hh)
    local solids = {}
    local hits = kione.query_aabb(cx, cy, hw * 2, hh * 2)
    for i = 1, #hits do
        local other = hits[i]
        if other:id() ~= self_id and other:valid() and other.tag ~= COIN_TAG then
            local collider, transform = other:collider(), other:transform()
            if collider and transform then
                local ohw, ohh = half_extents(collider)
                solids[#solids + 1] =
                    { x = transform.translation.x, y = transform.translation.y, hw = ohw, hh = ohh }
            end
        end
    end
    return solids
end

function on_update(entity, dt)
    local transform = entity:transform()
    local collider = entity:collider()
    if not transform or not collider then
        return
    end

    local id = entity:id()
    local hw, hh = half_extents(collider)
    local px, py = transform.translation.x, transform.translation.y

    local move = 0.0
    if Input.is_key_down(Key.a) or Input.is_key_down(Key.left) then
        move = move - 1.0
    end
    if Input.is_key_down(Key.d) or Input.is_key_down(Key.right) then
        move = move + 1.0
    end
    vx = move * MOVE_SPEED

    local jump = Input.is_key_down(Key.space) or Input.is_key_down(Key.w) or Input.is_key_down(Key.up)
    if jump and not jump_held and grounded then
        vy = JUMP_SPEED
        grounded = false
    end
    jump_held = jump

    vy = math.max(vy - GRAVITY * dt, -MAX_FALL)

    px = px + vx * dt
    py = py + vy * dt

    grounded = false
    for _ = 1, 8 do
        local hit, hox, hoy
        for _, s in ipairs(solids_at(id, px, py, hw, hh)) do
            local ox = (hw + s.hw) - math.abs(px - s.x)
            local oy = (hh + s.hh) - math.abs(py - s.y)
            if ox > 0 and oy > 0 and (not hit or math.min(ox, oy) < math.min(hox, hoy)) then
                hit, hox, hoy = s, ox, oy
            end
        end
        if not hit then
            break
        end
        if hox < hoy then
            px = px + (px < hit.x and -hox or hox)
            vx = 0.0
        elseif py < hit.y then
            py = py - hoy
            if vy > 0.0 then
                vy = 0.0
            end
        else
            py = py + hoy
            grounded = true
            if vy < 0.0 then
                vy = 0.0
            end
        end
    end

    transform.translation.x = px
    transform.translation.y = py

    local hits = kione.query_aabb(px, py, hw * 2, hh * 2)
    for i = 1, #hits do
        local other = hits[i]
        if other:valid() and other.tag == COIN_TAG then
            coins = coins + 1
            kione.log("Collected coin (" .. coins .. ")")
            other:destroy()
        end
    end
end
