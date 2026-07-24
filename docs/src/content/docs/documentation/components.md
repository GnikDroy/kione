---
title: Components
description: Every built-in component — purpose, fields, and Lua usage.
---

A component is one **ability** you give an entity: *draw a picture*, *make a sound*, *be
touchable*, *cast light*. You build every game object by stacking a few components on an
entity — there is nothing else to it.

You add and edit components in the **Inspector** (select an entity, click **Add Component**),
and read or change the same fields from scripts through an
[entity handle](../scripting/#entity-handles). Each accessor (`e:sprite()`, `e:collider()`, …)
returns the component, or `nil` if the entity doesn't have it.

Some typical builds, to give you a feel for it:

| You want a… | Give it |
| --- | --- |
| Player | Transform + Sprite + Collider + Script |
| Coin | Transform + Sprite + Animation + Collider |
| Level | Transform + TileMap |
| Camera | Transform + Camera + MainCamera |
| Torch | Transform + Sprite + PointLight |
| Score label | Transform + Text |
| Background music | AudioSource (with `looping` and `play_on_create`) |

Every entity automatically starts with a **Transform** (its position), so in practice you only
add the rest.

## <span class="heading-icon" data-icon="open_with"></span>Transform

The foundation: an entity's position, rotation, and scale in 2D space. **Every visible or
positioned entity needs a Transform** — sprites, text, tilemaps, colliders, and lights are all
placed by it.

**Fields**

- `translation` — `vec3` world position (z orders draw depth).
- `scale` — `vec3` scale.
- `angle` — rotation about the Z axis, in **radians**.

```lua
local t = self:transform()
t.translation = t.translation + vec3.new(0, 1, 0) * speed * dt
t.angle = t.angle + dt          -- spin
```

`e:world_transform()` returns a read-only Transform snapshot with the parent chain applied —
useful when an entity is parented and you need its absolute position.

## <span class="heading-icon" data-icon="image"></span>Sprite

Draws a picture at the entity's position. With no texture picked it draws a plain white
rectangle — useful as a placeholder while you work.

**Fields**

- `texture` — name of an `Image` asset.
- `color` — `vec4` tint (multiplied with the texture).
- `size` — `vec2` on-screen size in world units.
- `region` — `Rect` (`x, y, w, h`) sub-rectangle of the texture, in pixels measured from the
  image's top-left corner (for atlases and spritesheets).
- `unlit` — `bool`; when true the sprite ignores scene lighting.
- `intensity` — `float` emissive multiplier (drives bloom for glowing sprites).
- `blend` — `"alpha"` or `"additive"`.

*Requires a Transform.*

```lua
local s = self:sprite()
s.color = vec4.new(1, 0, 0, 1)
s.blend = "additive"
```

## <span class="heading-icon" data-icon="font_download"></span>Text

Draws text at the entity's position — scores, labels, dialogue. The text stays crisp at any
size.

**Fields**

- `text` — the string to draw.
- `font` — name of a `Font` asset.
- `size` — `float` pixel size.
- `color` — `vec4`.

*Requires a Transform.* Measure laid-out text with `e:text_size()` → `width, height`.

```lua
local label = self:text()
label.text = "Score: " .. score
```

## <span class="heading-icon" data-icon="movie"></span>Animation

Plays a `SpriteAnimation` clip by advancing a Sprite's `region` over time — so it drives an
existing **Sprite**.

**Fields**

- `clip` — name of the current `Animation` asset (read/write via `play`).
- `playing` — `bool`.
- `speed` — `float` playback multiplier.
- `elapsed` — `float` playback position in seconds (read/seek).
- `finished` — `bool`, read-only.

**Methods** — `play(clip)` starts a clip from the beginning; `stop()` pauses.

*Requires a Sprite (and therefore a Transform).*

```lua
local a = self:animation()
if moving then a:play("run") else a:play("idle") end
```

## <span class="heading-icon" data-icon="grid_on"></span>TileMap

A grid of tiles drawn from a `TileSet`, batched into a single draw. Great for level geometry
and backgrounds.

**Fields**

- `tileset` — name of a `TileSet` asset.
- `width`, `height` — grid dimensions, read-only.
- `tile_size` — `vec2` size of a cell in world units.
- `color` — `vec4` tint.
- `unlit` — `bool`.

**Methods**

- `get_tile(x, y)` — the tile index at a cell, or `nil` if empty.
- `set_tile(x, y, index)` — set a cell (`nil`/negative clears it).

*Requires a Transform and a TileSet asset.* Author tilemaps visually in the **Tilemap
Editor**.

```lua
local map = self:tilemap()
map:set_tile(3, 2, 7)   -- place tile 7 at column 3, row 2
```

## <span class="heading-icon" data-icon="adjust"></span>Collider

An invisible shape used to ask "are these things touching?" — from scripts, via
`e:overlaps(other)` and the `kione.query_*` functions. Kione doesn't push objects apart or
simulate physics for you; your script asks and decides what happens (the
[platformer demo's player script](../contributing/#platformer) shows how).

**Fields**

- `shape` — `"box"`, `"circle"`, or `"pill"`.
- `layer` — `uint` bitmask this collider belongs to.
- `mask` — `uint` bitmask of layers it collides with.
- `width`, `height` — box extents (box only).
- `radius` — radius (circle and pill).
- `half_height` — half of the straight section (pill only).

Accessing a field that doesn't apply to the current shape raises a script error.

*Requires a Transform.*

```lua
for _, other in ipairs(kione.query_circle(x, y, 32)) do
    if other:has_component("Collider") then handle(other) end
end
```

## <span class="heading-icon" data-icon="photo_camera"></span>Camera

Defines the view. Tag the active camera entity with **MainCamera** (in the Inspector) so the
renderer uses it.

**Fields**

- `position`, `target`, `up` — `vec3` view vectors; `look_at(x, y)` points it at a spot.
- `projection` — `"orthographic"` (2D default) or `"perspective"`.
- `scale_mode` — how the view maps to a window of a different aspect:
  `"stretch"`, `"fit_width"`, `"fit_height"`, `"expand"`, `"letterbox"`.
- Orthographic bounds: `left`, `right`, `top`, `bottom`.
- Perspective: `fov` (radians), `aspect_ratio`.
- `near_clip`, `far_clip` — clip planes (both projections).

```lua
local cam = kione.find_main_camera():camera()
cam:look_at(player.x, player.y)
cam.scale_mode = "letterbox"
```

## <span class="heading-icon" data-icon="cloud"></span>Environment

Scene-wide lighting and post-processing. Usually placed on the camera or a root entity.

**Fields**

- `ambient_color` — `vec3` ambient light color.
- `ambient_intensity` — `float`.
- `clear_color` — `vec4` background color.
- `bloom` — `bool` enable bloom.
- `bloom_intensity` — `float`.
- `bloom_threshold` — `float` brightness cutoff for bloom.

```lua
local env = self:environment()
env.bloom = true
env.ambient_intensity = 0.8
```

## <span class="heading-icon" data-icon="lightbulb"></span>Lights

Three components add light to your scene. Sprites are shaded by the lights around them (unless
marked `unlit`), so a dark scene with a few lights instantly looks moody. All lights sit at
their entity's position.

**PointLight** — omni-directional.
- `color` — `vec3`, `intensity` — `float`, `radius` — `float`.

**SpotLight** — a cone.
- `color`, `intensity`, `radius`, plus `inner_angle` and `outer_angle` (the cone falloff).

**SpriteLight** — projects a texture as light (e.g. a soft glow or gobo).
- `color` — `vec3`, `intensity` — `float`, `texture` — name of an `Image` asset.

```lua
local light = self:point_light()
light.color = vec3.new(1.0, 0.8, 0.5)
light.radius = 400
```

## <span class="heading-icon" data-icon="volume_up"></span>AudioSource

Plays an `Audio` clip positioned on the entity.

**Fields** — `clip` (asset name), `volume`, `pitch`, `looping` (`bool`), `play_on_create`
(`bool`).

`play_on_create` arms the source: the first frame the engine sees it `true`, the clip starts —
exactly once per component. Set it `false` in `on_create` to suppress an autoplay authored in
the editor; setting it `true` later from a script plays the clip once. It never re-triggers,
and it doesn't stop a sound that's already playing — for on-demand sounds use
[`kione.play_sound`](../scripting/#kioneplay_sound).

```lua
local a = self:audio_source()
a.volume = 0.5
a.looping = true
```

## <span class="heading-icon" data-icon="code"></span>Script

Attaches a `Script` asset to the entity, which is what makes the
[lifecycle hooks](../scripting/#lifecycle-hooks) run for it. Add it in the
Inspector and pick your `.lua` script; there are no scriptable fields on the component itself.

## <span class="heading-icon" data-icon="data_object"></span>Custom data with `:data()`

Every entity carries a free-form Lua table, reachable with `e:data()`. It behaves like a
**custom component you define in script** — store whatever state your game needs, and read it
from *other* entities' scripts too:

```lua
-- on the player
function on_create(self)
    self:data().health = 100
end

-- from an enemy's script
local player = kione.find("Player")
if player then
    player:data().health = player:data().health - 10
end
```

This is how entities share custom state without any engine change — the table is per-entity,
persists for the entity's lifetime, and is copied when the entity is cloned.
