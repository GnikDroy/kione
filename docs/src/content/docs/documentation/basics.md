---
title: Basics
description: The five ideas behind every Kione game, and how projects work.
---

Everything in Kione is built from five simple ideas. Learn these and the rest of the manual
will make sense.

## The five ideas

**Project** — a folder containing your whole game: its scenes, images, sounds, and scripts,
plus one `.k2project` file that lists them. One game = one project.

**Scene** — one screen of your game: a level, a title screen, a game-over screen. A project
can hold many scenes, and scripts can switch between them
([`kione.load_scene`](../scripting/#kioneload_scene)).

**Entity** — a *thing* in a scene. The player, a coin, a tree, a light, the camera — each is
an entity. On its own an entity is just an empty name with a position.

**Component** — an *ability* you attach to an entity. A **Sprite** component draws a picture.
A **Collider** lets it touch things. A **Script** gives it behavior. You build entities by
stacking components — see the [Components](../components/) page for the full menu.

**Asset** — a file your game uses: an image, a sound, a font, a Lua script, a tileset, an
animation, or a scene. Assets live in your project folder and get a short **name** (like
`hero` or `jump_sound`) that components and scripts refer to.

That's the whole model: a **project** holds **scenes**, scenes hold **entities**, entities are
made of **components**, and components point at **assets** by name.

## Creating a project

Open the `editor` and choose **File → New Project** (Ctrl+N). Pick a folder and a name, and
Kione creates the `.k2project` file plus an empty first scene.

**File → Open Project** (Ctrl+O) reopens an existing one. You can also open a project straight
from the command line:

```sh
./editor path/to/my-game.k2project
```

Save your scene with **Ctrl+S** — and save often.

## Adding assets

1. Copy the file (a `.png`, `.wav`, `.lua`, …) somewhere inside your project folder.
2. In the editor's **Assets** window, click **+** and pick the file.

The asset now has a name, and it appears in the dropdowns all over the editor — a Sprite's
texture picker, a Script component's script picker, and so on. You can rename or remove assets
in the same window.

:::note
Assets must live *inside* the project folder — the whole folder is your game, and it has to
carry everything it needs.
:::

## Running your game

Kione has **no build step**. There are two ways to run a game:

- **In the editor**: press **Ctrl+P** (or the play button) to enter play mode. Scripts run,
  and the view switches to the game's camera. Press again to stop. Play mode runs on a copy of
  your scene, so nothing that happens during play changes your work.
- **With the player**: the `player` program runs a project directly:

```sh
./player path/to/my-game.k2project
```

Inspired by [LÖVE](https://love2d.org/), you can also **drag the `.k2project` file onto the
`player` executable** and it just runs.

## Shipping a game

The `player` is completely self-contained, so shipping is just:

1. Copy the `player` executable into your project folder, next to the `.k2project` file.
2. Zip the folder and share it.

Anyone can double-click `player` to play — no installs, no dependencies, no build.

## Good to know

- Kione is a **small hobby engine** — small enough that you can read and change its source.
- It focuses squarely on **2D** games.
- Everything saves as **plain, readable text** (YAML). Scenes and projects are easy to look
  at, compare, and even edit by hand — readable enough that an LLM can write whole scenes for
  you.

## The project manifest

You rarely need to touch this file — the **Assets** window maintains it for you — but it's
useful to know what's inside.

A project is described by a single `.k2project` file. It records the project `name`, which
scene to open first (`main_scene`), and every asset, grouped by type, mapped from its **name**
to the **file** it lives in:

```yaml
version: 0.0.1
name: main
main_scene: main
assets:
  Scene:
    main: file:///main.k2scene
  Image:
    sky_: file:///textures/sky_.png?filter=nearest
    coin_: file:///textures/coin_.png?filter=nearest
  TileSet:
    sky: file:///tilesets/sky.k2tileset
  Animation:
    coin: file:///animations/coin.k2anim
  Script:
    player: file:///scripts/player.lua
```

`file:///` paths are relative to the project folder. The asset types are `Scene`, `Image`,
`Font`, `Audio`, `Animation`, `Script`, `TileSet`, `Shader`, and `Data`.

## Asset URI options

Image entries accept options after a `?` to control how the picture is displayed:

- **`?filter=nearest`** — keep pixels crisp instead of smoothing them. **Use this for pixel
  art.** The default is smooth (linear) filtering, which blurs small images when they're
  scaled up.
- **`?mipmaps=1`** — turn on mipmaps (pre-shrunk copies used when an image is drawn small).
  Off by default, since 2D art is usually drawn near its real size; turn it on for a texture
  you scale far down.

```yaml
Image:
  tiles: file:///textures/tiles.png?filter=nearest    # crisp pixels
  sky:   file:///textures/sky.png?mipmaps=1           # scaled way down, wants mipmaps
```

Options combine with `&`: `file:///textures/tiles.png?filter=nearest&mipmaps=1`.

:::tip
Blurry pixel art is the most common first-day problem — the fix is adding `?filter=nearest`
to the image's entry in the `.k2project` file.
:::
