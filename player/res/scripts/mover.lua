local speed = 400

function on_create(entity)
    kione.log("mover attached to '" .. entity:tag() .. "'")
end

function on_update(entity, dt)
    local transform = entity:transform()
    if transform == nil then
        return
    end

    if Input.is_key_down("a") or Input.is_key_down("left") then
        transform.translation.x = transform.translation.x - speed * dt
    end
    if Input.is_key_down("d") or Input.is_key_down("right") then
        transform.translation.x = transform.translation.x + speed * dt
    end
    if Input.is_key_down("w") or Input.is_key_down("up") then
        transform.translation.y = transform.translation.y + speed * dt
    end
    if Input.is_key_down("s") or Input.is_key_down("down") then
        transform.translation.y = transform.translation.y - speed * dt
    end
end
