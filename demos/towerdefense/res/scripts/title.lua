local save = require("save")

local prompt
local prompt_time = 0
local unlocked = 1

local function start_level(index)
    kione.play_sound("build_chime", 0.6)
    kione.load_scene("level" .. index)
end

local function update_stats_line(data)
    local stats = kione.find("Stats")
    if not stats then
        return
    end

    local line = ""
    if unlocked > 1 then
        line = "LEVELS 1-" .. unlocked .. " UNLOCKED - PRESS A NUMBER TO SELECT"
    end
    if (data.victories or 0) > 0 then
        line = line .. (line ~= "" and "   " or "") .. "CAMPAIGN VICTORIES " .. data.victories
    end
    stats:text().text = line
end

function on_create(entity)
    prompt = kione.find("Prompt")

    local data = save.read()
    unlocked = math.max(1, math.min(kione.level_count or 5, data.unlocked or 1))
    update_stats_line(data)

    kione.log("[TD] title screen (unlocked=" .. unlocked .. ")")
end

function on_update(entity, dt)
    prompt_time = prompt_time + dt
    if prompt and prompt:valid() then
        prompt:text().color.w = 0.55 + 0.45 * math.sin(prompt_time * 2.5)
    end
end

function on_event(entity, event)
    if event.type == EventType.key and event.state == InputState.press then
        local number = tonumber(((event.key or ""):gsub("^kp_", "")))
        if number and number >= 1 and number <= unlocked then
            start_level(number)
        else
            start_level(unlocked)
        end
        return true
    end

    if event.type == EventType.mouse_button and event.state == InputState.press then
        start_level(unlocked)
        return true
    end
end
