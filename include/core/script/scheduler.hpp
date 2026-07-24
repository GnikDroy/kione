#pragma once

namespace k2 {

inline constexpr const char* SCHEDULER_SOURCE = R"lua(
local timers, tweens = {}, {}

local EASE = {
    linear = function(t) return t end,
    ["in"] = function(t) return t * t end,
    out = function(t) return t * (2 - t) end,
    in_out = function(t) if t < 0.5 then return 2 * t * t else return -1 + (4 - 2 * t) * t end end,
    back = function(t) t = t - 1 return 1 + t * t * (2.70158 * t + 1.70158) end,
}

function kione.after(seconds, fn)
    if type(seconds) ~= "number" or type(fn) ~= "function" then
        error("kione.after(seconds, fn) expects a number and a function")
    end
    local timer = { at = seconds, fn = fn }
    timers[#timers + 1] = timer
    return { cancel = function() timer.dead = true end }
end

function kione.tween(target, property, to, duration, easing, owner)
    local ease = EASE[easing or "linear"]
    if not ease then
        error("kione.tween: unknown easing '" .. tostring(easing) .. "'")
    end
    if type(to) ~= "number" or type(duration) ~= "number" then
        error("kione.tween(target, property, to, duration, easing?, owner?) tweens a numeric property")
    end
    local from = target[property]
    if type(from) ~= "number" then
        error("kione.tween: property '" .. tostring(property) .. "' is not a number")
    end
    local tween = { target = target, property = property, from = from, to = to,
        duration = math.max(duration, 1e-6), t = 0, ease = ease, owner = owner }
    tweens[#tweens + 1] = tween
    return { cancel = function() tween.dead = true end }
end

return function(dt)
    local count = #timers -- callbacks may schedule more; those start next frame
    for i = 1, count do
        local timer = timers[i]
        if not timer.dead then
            timer.at = timer.at - dt
            if timer.at <= 0 then
                timer.dead = true
                local ok, err = pcall(timer.fn)
                if not ok then
                    kione.log("[kione.after] " .. tostring(err))
                end
            end
        end
    end

    count = #tweens
    for i = 1, count do
        local tween = tweens[i]
        if not tween.dead then
            if tween.owner and not tween.owner:valid() then
                tween.dead = true
            else
                tween.t = tween.t + dt
                local alpha = math.min(tween.t / tween.duration, 1)
                local value = tween.from + (tween.to - tween.from) * tween.ease(alpha)
                local ok, err = pcall(function() tween.target[tween.property] = value end)
                if not ok then
                    tween.dead = true
                    kione.log("[kione.tween] " .. tostring(err))
                elseif alpha >= 1 then
                    tween.dead = true
                end
            end
        end
    end

    local live_timers, live_tweens = {}, {}
    for i = 1, #timers do
        if not timers[i].dead then live_timers[#live_timers + 1] = timers[i] end
    end
    for i = 1, #tweens do
        if not tweens[i].dead then live_tweens[#live_tweens + 1] = tweens[i] end
    end
    timers, tweens = live_timers, live_tweens
end
)lua";

}
