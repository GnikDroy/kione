---
title: Your First Game
description: A step-by-step tutorial — make a square move, collect a coin, and ship it.
---

In this tutorial you'll build a tiny game from nothing: a character you move with the arrow
keys that collects a coin. Along the way you'll touch every important part of Kione once.

No experience needed. Every step tells you exactly what to click and what to type.

## 1. Create a project

1. Open the **editor**.
2. Choose **File → New Project** (or press **Ctrl+N**).
3. Pick an empty folder and give your project a name, e.g. `my-game`.

Kione creates the project and opens an empty scene. A **scene** is one screen of your game — a
level, a menu, a game-over screen. A **project** is the folder that holds all of them.

You're looking at the editor's main layout:

- The big center panel is the **Viewport** — your view into the scene.
- **Entity Selector** (left) lists everything in the scene.
- **Inspector** (right) shows the details of whatever is selected.
- **Assets** lists the files your game uses (images, sounds, scripts).

Don't worry about memorizing panels — each one is explained on the
[Editor Windows](../../documentation/windows/) page.

## 2. Put something on screen

1. Choose **Edit → Create Entity**. A new entry appears in the Entity Selector.
2. With it selected, look at the **Inspector** and click **Add Component → Sprite**.

A white square appears in the Viewport. That's your character for now!

Two words worth learning:

- An **entity** is a *thing* in your scene — a character, a coin, a light, a camera.
- A **component** is one *ability* you give an entity. A **Sprite** component means "draw a
  picture here". Entities start with just a **Transform** (a position), and you stack
  components on top.

Try it: in the Inspector, change the Transform's **Translation** numbers and watch the square
move. Change the Sprite's **Color** to tint it.

:::tip[Moving around the Viewport]
Scroll to zoom. Drag with the middle mouse button (or hold **Space** and drag) to pan.
Press **F** to jump to the selected entity, **Home** to reset the view.
:::

## 3. Name your entity

In the Inspector, find the **Tag** field and type `Player`.

A tag is just a name. Scripts use tags to find entities, so give one to anything a script will
need to look up.

## 4. Use your own picture (optional)

The white square works fine for this tutorial, but using your own art is easy:

1. Copy a `.png` image into your project folder.
2. In the **Assets** window, click the **+** button and pick the file.
3. Select your Player, and in the Sprite component choose your image from the **Texture**
   dropdown.
4. If needed, adjust the Sprite's **Size** (it's in world units, not pixels).

:::note[Pixel art?]
If your art looks blurry, it's being smoothed. See
[Asset options](../../documentation/basics/#asset-uri-options) for the one-line fix
(`?filter=nearest`).
:::

## 5. Add a camera

In the editor you fly around freely, but a *game* needs to know where to look.

1. Create another entity (**Edit → Create Entity**) and tag it `Camera`.
2. **Add Component → Camera**.
3. **Add Component → MainCamera** — this marks it as *the* camera the game looks through.

The default camera shows the area from -640 to +640 across and -360 to +360 up and down,
centered on the middle of the world — a comfortable 1280×720 view. You can see its frame drawn
in the Viewport.

Press **Ctrl+S** to save your scene. Save often!

## 6. Make it move

Time for your first script. Scripts are small text files written in **Lua** — a friendly
little language you can pick up as you go
(there's a [five-minute primer](../../documentation/scripting/#a-five-minute-lua-primer)).

1. In your project folder, create a file called `player.lua` with any text editor and paste:

```lua
local SPEED = 200 -- world units per second

function on_update(self, dt)
    local t = self:transform()
    if Input.is_key_down(Key.left) then
        t.translation.x = t.translation.x - SPEED * dt
    end
    if Input.is_key_down(Key.right) then
        t.translation.x = t.translation.x + SPEED * dt
    end
    if Input.is_key_down(Key.up) then
        t.translation.y = t.translation.y + SPEED * dt
    end
    if Input.is_key_down(Key.down) then
        t.translation.y = t.translation.y - SPEED * dt
    end
end
```

2. In the **Assets** window, click **+** and pick `player.lua`.
3. Select your Player entity, **Add Component → Script**, and choose `player` from the
   dropdown.

Now press **Ctrl+P** (or the play button in the Viewport) to enter **play mode**. The view
switches to your camera, and the arrow keys move your character. Press **Ctrl+P** again to
stop.

What the script means, line by line:

- `function on_update(self, dt)` — Kione calls this function once every frame. `self` is the
  entity the script is attached to; `dt` is how many seconds the last frame took (a small
  number like 0.016).
- `self:transform()` — grabs the entity's Transform component, the same one you saw in the
  Inspector.
- `Input.is_key_down(Key.left)` — `true` while the left arrow is held.
- Multiplying by `dt` makes movement smooth and the same speed on every computer.

:::tip[Something wrong?]
Open the **Log Viewer** window. Script errors and `kione.log(...)` messages show up there —
it's your best friend while scripting.
:::

## 7. Add a coin to collect

1. Create a new entity, tag it `Coin`, and give it a **Sprite** (tint it yellow!). Move it
   somewhere away from the player.
2. Add a **Collider** component to the coin, and another to the Player. A collider is an
   invisible shape used to ask "are these two things touching?" — the default 32×32 box is
   fine here.
3. Add this to the end of `on_update` in `player.lua` (still inside the function):

```lua
    -- collect coins we touch
    for _, coin in ipairs(kione.find_all("Coin")) do
        if self:overlaps(coin) then
            coin:destroy()
            kione.log("Coin collected!")
        end
    end
```

Press **Ctrl+P** and walk into the coin — it disappears, and the message appears in the Log
Viewer.

How it works: `kione.find_all("Coin")` gives you every entity tagged `Coin`,
`self:overlaps(coin)` checks whether the two colliders touch, and `coin:destroy()` removes the
coin from the scene.

:::note
Play mode runs on a *copy* of your scene — anything that happens during play (like the coin
being destroyed) is undone when you press stop. Your editing scene is safe.
:::

## 8. Add a sound (optional)

1. Copy a short `.wav` or `.mp3` file into your project and add it in the **Assets** window
   (say it's named `pickup`).
2. Play it when the coin is collected:

```lua
        coin:destroy()
        kione.play_sound("pickup")
```

## 9. Ship it

Your game is a folder; the player is a program. Shipping is putting them together:

1. Copy the **`player`** executable *into* your project folder, right next to the
   `.k2project` file.
2. Zip the folder and send it to a friend.

They double-click `player` and your game runs — no installs, no builds. You can also drag the
`.k2project` file onto `player`, or run it from a terminal:

```sh
./player my-game/my-game.k2project
```

## Where to go from here

- Give the coin an **Animation** so it spins — author one in the
  [Animation window](../../documentation/windows/#animation).
- Paint a level with the [Tilemap Editor](../../documentation/windows/#tilemap-editor).
- Add a **PointLight** and watch your scene light up
  ([Components → Lights](../../documentation/components/#lights)).
- Read the full [Scripting reference](../../documentation/scripting/) — everything scripts can
  do, with an example for each function.
- Open the [demos](../../documentation/contributing/#demos) — a complete little platformer
  built exactly the way you just built this, and Crystal Keep, a full five-level tower-defense
  campaign showing how far pure content can go.
