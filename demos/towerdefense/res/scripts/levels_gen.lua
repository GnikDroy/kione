kione.level_count = 5

function on_create(self)
    kione.levels = {
    [1] = {
        name = "GREENWOOD",
        scene = "level1",
        gold = 12,
        lives = 5,
        lanes = {  },
        waypoints = { { -704, 160 }, { 160, 160 }, { 160, -160 }, { 544, -160 } },
        waves = {
            { { "grunt", 5, 1.4 } },
            { { "grunt", 6, 1.1 }, { "scout", 3, 1.0 } },
            { { "scout", 8, 0.7 }, { "grunt", 4, 1.2 } },
            { { "grunt", 8, 0.9 }, { "scout", 6, 0.6 } },
        },
    },
    [2] = {
        name = "DUNES",
        scene = "level2",
        gold = 12,
        lives = 5,
        lanes = { 180 },
        waypoints = { { -704, 224 }, { -288, 224 }, { -288, -160 }, { 288, -160 }, { 288, 96 }, { 544, 96 } },
        waves = {
            { { "grunt", 6, 1.2 } },
            { { "scout", 6, 0.8 }, { "grunt", 4, 1.2 } },
            { { "apc", 3, 2.2 }, { "scout", 5, 0.8 } },
            { { "grunt", 8, 0.8 }, { "apc", 3, 1.8 } },
            { { "apc", 4, 1.6 }, { "scout", 8, 0.6 }, { "bomber", 1, 1 } },
        },
    },
    [3] = {
        name = "HIGHLANDS",
        scene = "level3",
        gold = 14,
        lives = 5,
        lanes = { 220, -40 },
        waypoints = { { -416, 448 }, { -416, -224 }, { -96, -224 }, { -96, 224 }, { 224, 224 }, { 224, -224 }, { 544, -224 } },
        waves = {
            { { "grunt", 7, 1.0 }, { "scout", 4, 0.8 } },
            { { "apc", 4, 1.8 }, { "scout", 6, 0.7 } },
            { { "tank", 2, 3.0 }, { "grunt", 8, 0.8 } },
            { { "apc", 4, 1.6 }, { "bomber", 2, 4.0 }, { "scout", 6, 0.6 } },
            { { "tank", 3, 2.4 }, { "apc", 4, 1.4 }, { "bomber", 2, 3.5 } },
            { { "tank", 4, 2.0 }, { "scout", 10, 0.5 }, { "bomber", 3, 3.0 } },
        },
    },
    [4] = {
        name = "BADLANDS",
        scene = "level4",
        gold = 14,
        lives = 5,
        lanes = { 240, 0, -200 },
        waypoints = { { -704, 32 }, { -96, 32 }, { -96, 224 }, { 352, 224 }, { 352, -160 }, { 96, -160 } },
        waves = {
            { { "apc", 4, 1.6 }, { "scout", 6, 0.7 } },
            { { "tank", 3, 2.4 }, { "grunt", 8, 0.8 } },
            { { "elite", 1, 1 }, { "apc", 5, 1.4 }, { "bomber", 2, 4.0 } },
            { { "tank", 4, 2.0 }, { "scout", 10, 0.5 }, { "bomber", 2, 3.5 } },
            { { "elite", 2, 3.5 }, { "apc", 6, 1.2 }, { "bomber", 3, 3.0 } },
            { { "tank", 5, 1.8 }, { "elite", 2, 3.0 }, { "grunt", 10, 0.6 }, { "bomber", 3, 2.8 } },
        },
    },
    [5] = {
        name = "THE ASHEN KEEP",
        scene = "level5",
        gold = 16,
        lives = 5,
        lanes = { 260, 40, -160 },
        waypoints = { { -544, 448 }, { -544, -160 }, { -288, -160 }, { -288, 160 }, { -32, 160 }, { -32, -160 }, { 224, -160 }, { 224, 160 }, { 480, 160 }, { 480, -160 }, { 544, -160 } },
        waves = {
            { { "apc", 5, 1.4 }, { "scout", 8, 0.6 } },
            { { "tank", 3, 2.2 }, { "grunt", 10, 0.7 } },
            { { "elite", 2, 3.5 }, { "apc", 5, 1.3 }, { "bomber", 2, 4.0 } },
            { { "tank", 4, 1.9 }, { "scout", 12, 0.45 }, { "bomber", 3, 3.5 } },
            { { "elite", 3, 2.8 }, { "tank", 3, 2.2 }, { "bomber", 3, 3.0 } },
            { { "tank", 5, 1.7 }, { "elite", 3, 2.6 }, { "apc", 6, 1.1 }, { "bomber", 3, 2.6 } },
            { { "elite", 5, 2.2 }, { "tank", 5, 1.6 }, { "bomber", 4, 2.4 }, { "scout", 12, 0.4 } },
        },
    },
    }
    local index = tonumber(string.match(self.tag, "%d+$"))
    if index then
        kione.level_index = index
    end
end
