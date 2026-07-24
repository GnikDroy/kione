---
title: Scripting
description: A complete reference for Kione's Lua API — hooks, the kione global, entity handles, input, and vectors.
---

Scripts give entities behavior. A script is a plain text file written in **Lua**, and Kione
runs it for the entity it's attached to. This page starts with the basics, then documents
every function a script can call.

## Your first script

Three steps to attach behavior to an entity:

1. Create a file, e.g. `spin.lua`, in your project folder:

```lua
function on_update(self, dt)
    self:transform().angle = self:transform().angle + dt
end
```

2. Add the file in the **Assets** window (**+** button).
3. Select the entity, **Add Component → Script**, and pick `spin`.

Press play — the entity spins. That's the whole loop: write a function, attach it, play.

Some useful things to know:

- Each entity runs its script in its **own little world** — two entities with the same script
  don't share their local variables. Each gets its own copy.
- If a script has an error, it doesn't crash the game. The error is printed to the
  **Log Viewer** and that function stops being called. Fix the script and press play again.
- `kione.log("message")` prints to the Log Viewer — sprinkle it everywhere while learning.

## A five-minute Lua primer

Never written Lua? Here's everything this manual uses:

```lua
-- Anything after two dashes is a comment.

local speed = 200            -- a variable (local = only this script sees it)
local name = "hero"          -- text goes in quotes
local alive = true           -- true or false

if speed > 100 then          -- if / then / end
    kione.log("fast!")
elseif speed > 0 then
    kione.log("slow")
else
    kione.log("stopped")
end

for i = 1, 5 do              -- count from 1 to 5
    kione.log("hello " .. i) -- .. glues text together
end

local items = { "sword", "shield" }   -- a list (tables start at 1!)
for _, item in ipairs(items) do       -- loop over a list
    kione.log(item)
end

local function double(x)     -- define a function
    return x * 2
end
```

Two things that trip people up: lists start at **1**, not 0, and "not equal" is written `~=`.
That's genuinely most of it — you can learn the rest as you need it.

## Lifecycle hooks

A hook is a function with a special name. You define it; Kione calls it at the right moment.
Define only the ones you need. In every hook, `self` is a handle to the entity the script is
attached to.

### on_create

```lua
function on_create(self) end
```

Called once when the entity appears — when the scene starts, or when the entity is spawned.
Use it to set up starting state.

- **self** — the entity handle.

```lua
function on_create(self)
    self:data().health = 100
    self:sprite().color = vec4.new(1, 1, 1, 1)
end
```

### on_update

```lua
function on_update(self, dt) end
```

Called once per frame — this is where most game logic lives.

- **self** — the entity handle.
- **dt** — seconds since the last frame (a small number like `0.016`). Multiply speeds by
  `dt` so movement is smooth and identical on fast and slow computers.

```lua
function on_update(self, dt)
    self:transform().translation.x = self:transform().translation.x + 60 * dt
end
```

### on_fixed_update

```lua
function on_fixed_update(self, dt) end
```

Called on a fixed clock (many times per second, always the same step), no matter the frame
rate. Use it for movement and physics-like logic that must behave identically everywhere.

- **self** — the entity handle.
- **dt** — the fixed step in seconds (a constant).

```lua
function on_fixed_update(self, dt)
    vy = vy - GRAVITY * dt
end
```

### on_event

```lua
function on_event(self, event) end
```

Called for each input or window event — a key press, a mouse click, a window resize. Good for
one-time reactions ("the moment the key went down") rather than continuous state ("is the key
held?", which is [`Input.is_key_down`](#inputis_key_down)).

- **self** — the entity handle.
- **event** — a table with a `type` field (see [EventType](#input--constants)) plus
  type-specific fields, e.g. `event.x`, `event.y` for `cursor_position`, or `event.key` and
  `event.state` for `key` events.
- **Returns** — `true` to consume the event (stop other scripts from also receiving it),
  otherwise nothing.

```lua
function on_event(self, event)
    if event.type == "mouse_button" and event.button == "left" and event.state == "press" then
        fire()
        return true
    end
end
```

### on_destroy

```lua
function on_destroy(self) end
```

Called once just before the entity is removed. Use it for goodbyes — drop loot, play a sound,
update a counter.

```lua
function on_destroy(self)
    kione.play_sound("explosion")
end
```

## `kione` — world API

The `kione` table is available in every script. It's how a script talks to the rest of the
scene.

### kione.log

```lua
kione.log(message)
```

Writes a line to the editor's **Log Viewer**. Your main debugging tool.

- **message** — a string.

```lua
kione.log("player spawned at " .. x .. ", " .. y)
```

### kione.find

```lua
kione.find(tag)
```

Finds the first entity with the given tag (the name typed into the **Tag** field in the
Inspector).

- **tag** — a string tag.
- **Returns** — an entity handle, or `nil` if none matches. Always check for `nil`!

```lua
local player = kione.find("Player")
if player then
    chase(player)
end
```

### kione.find_all

```lua
kione.find_all(tag)
```

Finds every entity with the given tag.

- **tag** — a string tag.
- **Returns** — a list of entity handles (possibly empty).

```lua
for _, coin in ipairs(kione.find_all("Coin")) do
    coin:destroy()
end
```

### kione.entities

```lua
kione.entities(...component_names)
```

Finds every entity that has **all** of the named components — useful when tags aren't enough.

- **...component_names** — one or more component-name strings (`"Sprite"`, `"Collider"`, …;
  see [component names](#component-names)).
- **Returns** — a list of entity handles.

```lua
-- dim every light in the scene
for _, e in ipairs(kione.entities("PointLight")) do
    e:point_light().intensity = 0.2
end
```

### kione.find_main_camera

```lua
kione.find_main_camera()
```

Returns the camera entity the game is looking through (the one with the **MainCamera**
component).

- **Returns** — the entity handle, or `nil` if the scene has none.

```lua
local cam = kione.find_main_camera()
if cam then
    cam:camera():look_at(px, py)
end
```

### kione.spawn

```lua
kione.spawn(tag, x, y)
```

Makes a copy of a tagged entity — script and children included — at a position. The common
pattern is to keep a hidden "template" entity in the scene (a bullet, an enemy) and spawn
copies of it while playing.

- **tag** — the template's tag string. Errors if no entity has it.
- **x, y** — where to place the copy, in world units.
- **Returns** — the new entity handle.

```lua
local bullet = kione.spawn("Bullet", muzzle.x, muzzle.y)
bullet:data().direction = 1
```

### kione.load_scene

```lua
kione.load_scene(name)
```

Switches to another scene (the switch happens at the end of the frame).

- **name** — the `Scene` asset name.

```lua
if won then
    kione.load_scene("victory")
end
```

### kione.play_sound

```lua
kione.play_sound(name, volume, pitch)
```

Plays a sound once.

- **name** — an `Audio` asset name.
- **volume** — *optional* loudness (default `1.0`).
- **pitch** — *optional* speed/pitch multiplier (default `1.0`; `2.0` is chipmunk, `0.5` is
  slow-motion).

```lua
kione.play_sound("coin", 0.8, 1.2)
```

### kione.after

```lua
kione.after(seconds, fn)
```

Runs a function once, after a delay — no hand-rolled countdowns.

- **seconds** — how long to wait.
- **fn** — the function to call.
- **Returns** — a handle with a `cancel()` method.

```lua
kione.play_sound("horn")
kione.after(2.0, function()
    kione.spawn("Enemy", -700, 100)
end)
```

Timers live until the scene changes. To stop one early:

```lua
local pending = kione.after(5, open_gate)
-- ...changed your mind:
pending.cancel()
```

### kione.tween

```lua
kione.tween(target, property, to, duration, easing, owner)
```

Slides a numeric property from its current value to a new one over time — fades, slides,
grows, all without writing per-frame math.

- **target** — the thing that owns the property (a component, a color, a position…).
- **property** — the property's name, as a string (`"x"`, `"w"`, `"intensity"`).
- **to** — the value to end at (a number; the start is the current value).
- **duration** — seconds to take.
- **easing** — *optional*: `"linear"` (default), `"in"` (slow start), `"out"` (slow end),
  `"in_out"`, or `"back"` (overshoots, then settles).
- **owner** — *optional* entity handle; if that entity is destroyed, the tween stops
  safely instead of writing to something that's gone. **Pass this whenever the target
  belongs to an entity that might die.**
- **Returns** — a handle with a `cancel()` method.

```lua
-- fade a popup out and drift it upward, then remove it
function on_create(self)
    local translation = self:transform().translation
    kione.tween(translation, "y", translation.y + 32, 0.75, "out", self)
    kione.tween(self:text().color, "w", 0, 0.75, "in", self)
    kione.after(0.8, function()
        if self:valid() then
            self:destroy()
        end
    end)
end
```

### kione.screen_to_world

```lua
kione.screen_to_world(x, y)
```

Converts a screen position (pixels, like a mouse position from `on_event`) into a world
position (the units entities live in).

- **x, y** — screen coordinates.
- **Returns** — two numbers: world `x, y`.

```lua
function on_event(self, event)
    if event.type == "cursor_position" then
        local wx, wy = kione.screen_to_world(event.x, event.y)
        self:transform().translation.x = wx
        self:transform().translation.y = wy
    end
end
```

### kione.world_to_screen

```lua
kione.world_to_screen(x, y)
```

The opposite of `screen_to_world` — where on screen a world position lands.

- **x, y** — world coordinates.
- **Returns** — two numbers: screen `x, y`.

```lua
local sx, sy = kione.world_to_screen(t.translation.x, t.translation.y)
```

### kione.screen_size

```lua
kione.screen_size()
```

The size of the area being drawn into, in pixels.

- **Returns** — two numbers: `width, height`.

```lua
local w, h = kione.screen_size()
```

### kione.query_circle

```lua
kione.query_circle(x, y, radius, mask)
```

Finds entities whose [Collider](../components/#collider) overlaps a circle. Great for "what's
near me?" checks — explosions, pickups, enemy senses.

- **x, y** — circle center in world units.
- **radius** — circle radius.
- **mask** — *optional* layer filter; leave it out to hit everything.
- **Returns** — a list of entity handles.

```lua
-- damage everything within 48 units of the blast
for _, e in ipairs(kione.query_circle(px, py, 48)) do
    if e:data().health then
        e:data().health = e:data().health - 25
    end
end
```

### kione.query_aabb

```lua
kione.query_aabb(x, y, w, h, mask)
```

Like `query_circle`, but with a rectangle centered at `(x, y)`.

- **x, y** — box center in world units.
- **w, h** — full box width and height.
- **mask** — *optional* layer filter.
- **Returns** — a list of entity handles.

```lua
-- everything the player's body is touching
local hits = kione.query_aabb(px, py, 32, 64)
```

### kione.query_point

```lua
kione.query_point(x, y, mask)
```

Finds entities whose collider contains a single point — perfect for "what did I click on?".

- **x, y** — the point in world units.
- **mask** — *optional* layer filter.
- **Returns** — a list of entity handles.

```lua
local wx, wy = kione.screen_to_world(event.x, event.y)
for _, e in ipairs(kione.query_point(wx, wy)) do
    select(e)
end
```

### kione.draw_line / draw_rect / draw_circle / draw_point / draw_polygon

```lua
kione.draw_line(x1, y1, x2, y2, options)
kione.draw_rect(x, y, w, h, options)
kione.draw_circle(x, y, radius, options)
kione.draw_point(x, y, options)
kione.draw_polygon(points, options)
```

Draws simple shapes for this frame only (call them every frame to keep a shape on screen).
Handy for debugging — seeing an attack radius, a path, a hitbox.

- Positions and sizes are in world units. `draw_polygon`'s **points** is a flat list
  `{x1, y1, x2, y2, …}`.
- **options** — *optional* table:
  - `color` — a `vec4` or a `{r, g, b, a}` table (each 0–1).
  - `z` — draw depth.
  - `width` — line/outline thickness.
  - `filled` — `true`/`false` (rects and circles).
  - `closed` — `true`/`false` (polygon).
  - `segments` — how round circles look.
  - `unlit` — `true` to ignore scene lighting.

```lua
-- show the pickup radius while tuning it
kione.draw_circle(px, py, 48, { color = { 1, 0, 0, 1 }, filled = false })
```

## Entity handles

A handle is how a script points at one entity — the `self` in every hook is one, and `find`,
`spawn`, and the queries give you more. Handles are cheap to pass around and safely become
invalid when their entity is destroyed (check with [`e:valid()`](#evalid)).

### Component accessors

```lua
e:transform(); e:sprite(); e:text(); e:animation(); e:tilemap()
e:audio_source(); e:collider(); e:environment(); e:camera()
e:point_light(); e:spot_light(); e:sprite_light()
```

Each returns the matching component — the same object you edit in the Inspector, with the
fields documented on the [Components](../components/) page — or `nil` if the entity doesn't
have that component.

- **Returns** — the component, or `nil`.

```lua
local s = e:sprite()
if s then
    s.color = vec4.new(1, 0, 0, 1)   -- turn it red
end
```

### e:world_transform

```lua
e:world_transform()
```

An entity parented under another moves with its parent. `transform()` gives the position
*relative to the parent*; `world_transform()` gives the final position in the world.

- **Returns** — a Transform **snapshot**. Editing it does nothing — use `e:transform()` to
  actually move the entity.

```lua
local wt = wheel:world_transform()
kione.log("the wheel is really at " .. wt.translation.x .. ", " .. wt.translation.y)
```

### e:valid

```lua
e:valid()
```

- **Returns** — `true` if the handle still refers to a live entity. Check this before using
  a handle you stored earlier — its entity may have been destroyed since.

```lua
if target and target:valid() then
    chase(target)
else
    target = nil
end
```

### e:id

```lua
e:id()
```

- **Returns** — the entity's number, unique while it lives. Useful to tell entities apart —
  for example, to skip yourself in a query result.

```lua
for _, hit in ipairs(kione.query_circle(px, py, 32)) do
    if hit:id() ~= self:id() then
        touch(hit)
    end
end
```

### e.tag

```lua
e.tag                 -- read
e.tag = "Enemy"       -- write
```

The entity's tag — note it's read and written like a variable, with no `()`.

```lua
if e.tag == "Coin" then
    collect(e)
end
```

### e:data

```lua
e:data()
```

Every entity carries a free-form table for your own values — health, score, direction,
anything. Other entities' scripts can read and write it too, which is how entities share
information. See [Components → Custom data](../components/#custom-data-with-data).

- **Returns** — the entity's data table.

```lua
self:data().health = 100
-- ...later, from any script:
local player = kione.find("Player")
player:data().health = player:data().health - 10
```

### e:text_size

```lua
e:text_size()
```

Measures how big the entity's [Text](../components/#text) will be on screen — useful for
centering.

- **Returns** — two numbers `width, height` (both `0` if there's no text or font).

```lua
local w, h = self:text_size()
self:transform().translation.x = -w / 2   -- center horizontally
```

### e:parent

```lua
e:parent()
```

- **Returns** — the parent entity handle, or `nil` if the entity has no parent.

### e:children

```lua
e:children()
```

- **Returns** — a list of the entity's direct children, in order.

```lua
for _, child in ipairs(e:children()) do
    child:sprite().color = vec4.new(1, 1, 1, 0.5)
end
```

### e:set_parent

```lua
e:set_parent(other)
```

Attaches the entity under another, so it moves with it — a sword following a hand, a hat on a
head.

- **other** — the new parent entity handle.

```lua
hat:set_parent(player)
hat:transform().translation = vec3.new(0, 20, 0)   -- 20 units above the player
```

### e:detach

```lua
e:detach()
```

Removes the entity from its parent so it stands alone again.

```lua
hat:detach()   -- the hat flies off
```

### e:add_component

```lua
e:add_component(name)
```

Gives the entity a new component while the game runs (or returns the one it already has).

- **name** — a component-name string (see [component names](#component-names)).
- **Returns** — the component, ready to set up, or `nil` for marker components like
  `MainCamera`.

```lua
local s = e:add_component("Sprite")
s.texture = "hero"
```

### e:remove_component

```lua
e:remove_component(name)
```

Removes the named component if present.

- **name** — a component-name string.

```lua
e:remove_component("PointLight")   -- lights out
```

### e:has_component

```lua
e:has_component(name)
```

- **name** — a component-name string.
- **Returns** — `true` if the entity has that component.

```lua
if e:has_component("Collider") then
    check_collision(e)
end
```

### e:overlaps

```lua
e:overlaps(other)
```

Are these two entities touching? Both need a [Collider](../components/#collider).

- **other** — another entity handle.
- **Returns** — `true` if the two colliders overlap.

```lua
if self:overlaps(door) then
    kione.load_scene("level2")
end
```

### e:clone

```lua
e:clone()
```

Makes a copy of the entity (components, data, and children included).

- **Returns** — the new entity handle.

```lua
local twin = self:clone()
twin:transform().translation.x = twin:transform().translation.x + 40
```

### e:destroy

```lua
e:destroy()
```

Removes the entity at the end of the frame (its `on_destroy` hook runs first). Safe to call
even if it was already destroyed.

```lua
if self:data().health <= 0 then
    self:destroy()
end
```

### Component names

`add_component`, `remove_component`, `has_component`, and `kione.entities` all take these
names: `Transform`, `Sprite`, `Text`, `Animation`, `TileMap`, `AudioSource`, `Collider`,
`Environment`, `Camera`, `MainCamera`, `PointLight`, `SpotLight`, `SpriteLight`, `Script`,
`Data`, `Tag`.

## Input & constants

### Input.is_key_down

```lua
Input.is_key_down(key)
```

- **key** — a value from the `Key` table (e.g. `Key.space`).
- **Returns** — `true` while the key is held.

```lua
if Input.is_key_down(Key.a) or Input.is_key_down(Key.left) then
    move_left()
end
```

Use this for *held* keys (walking); use [`on_event`](#on_event) for the *moment* a key goes
down (jumping, shooting) — otherwise a single press repeats every frame.

Key and button names live in strict tables — a typo like `Key.spcae` raises an error
immediately instead of silently doing nothing:

- **`Key`** — keyboard keys (`Key.a` … `Key.z`, `Key.left`, `Key.space`, `Key.enter`,
  `Key.escape`, `Key.f1` …).
- **`MouseButton`** — `left`, `right`, `middle`.
- **`InputState`** — `press`, `release`, `repeat`.
- **`EventType`** — the `event.type` strings in `on_event` (`key`, `char`, `mouse_button`,
  `cursor_position`, `scroll`, `window_resize`, …).

## Vector math

Positions, colors, and sizes come as vectors: `vec2` (two numbers), `vec3` (three), `vec4`
(four). Their fields are named `x`, `y`, `z`, `w` — for colors, read those as red, green,
blue, alpha (opacity), each from 0 to 1. Create them with `.new(...)`:

```lua
local a = vec2.new(3, 4)
local pos = vec3.new(100, 50, 0)
local red = vec4.new(1, 0, 0, 1)
```

You can do math with them directly:

- Operators: `a + b`, `a - b`, `a * s` (scale), `a / s`, `-a`, `a == b`.
- `v:length()` — how long the vector is.
- `v:normalize()` — same direction, length 1 (safe on zero vectors).
- `v:dot(other)`, `v:distance(other)` — dot product, distance between points.
- `v3:cross(other)` — cross product (`vec3` only).

```lua
-- move toward a target at a steady speed
local dir = (target - t.translation):normalize()
t.translation = t.translation + dir * speed * dt
```

## Sharing code with require

When two scripts need the same helper, put it in a **module**: a Lua file that returns a
table. Add the file as a `Script` asset (say it's named `utils`), and any script can load it:

```lua
-- utils.lua (added as a Script asset named "utils")
local utils = {}

function utils.clamp(value, low, high)
    return math.max(low, math.min(high, value))
end

return utils
```

```lua
-- any other script
local utils = require("utils")

function on_update(self, dt)
    local t = self:transform()
    t.translation.x = utils.clamp(t.translation.x, -600, 600)
end
```

`require` looks the name up among your project's `Script` assets — not on disk — so a
typo'd name fails with a clear error. A module runs once per scene and every script gets
the same table, which also makes it a good home for state that several scripts share.

## Common recipes

Small patterns you'll reuse constantly.

**A cooldown timer** — do something at most every 2 seconds:

```lua
local cooldown = 0

function on_update(self, dt)
    cooldown = cooldown - dt
    if Input.is_key_down(Key.space) and cooldown <= 0 then
        kione.spawn("Bullet", self:transform().translation.x, self:transform().translation.y)
        cooldown = 2.0
    end
end
```

**A camera that follows the player** — put this script on the camera entity:

```lua
function on_update(self, dt)
    local player = kione.find("Player")
    if player then
        local p = player:world_transform().translation
        self:camera():look_at(p.x, p.y)
    end
end
```

**Pulse / blink** — make something breathe using a running clock:

```lua
local time = 0

function on_update(self, dt)
    time = time + dt
    -- colors are vec4s: x, y, z, w = red, green, blue, alpha (opacity)
    self:sprite().color.w = 0.75 + 0.25 * math.sin(time * 4)
end
```

**A simple spawner** — one enemy every 3 seconds, up to 10:

```lua
local timer = 0
local spawned = 0

function on_update(self, dt)
    timer = timer + dt
    if timer > 3 and spawned < 10 then
        timer = 0
        spawned = spawned + 1
        local t = self:transform().translation
        kione.spawn("Enemy", t.x, t.y)
    end
end
```

For a full, real example, read the demo's player controller —
`demos/platformer/scripts/player.lua` — it's ~100 lines of movement, jumping, and coin
collection using nothing beyond this page.
