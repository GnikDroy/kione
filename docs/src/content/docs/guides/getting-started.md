---
title: Getting Started
description: Install Kione and open the editor for the first time.
---

Kione is a small 2D game engine. You build your game inside its **editor** — placing things,
drawing levels, and attaching small scripts — and you ship it with its **player**, a tiny
program that runs your game on any computer with no extra setup.

You don't need to know how game engines work to use it. If you can move files around and
edit a text file, you can make a game with Kione.

## What you need

- A computer running **Linux, macOS, or Windows**.
- Nothing else. (A compiler is only needed if you build Kione from source, below.)

## Download a release

The easiest way to start:

1. 👉 [Download the latest release](https://github.com/gnikdroy/kione/releases) for your system.
2. Unzip it anywhere you like.
3. Run the **`editor`** program inside.

That's it — the editor opens and you're ready to make something.

:::tip
Also included in the download is **`player`** — the program that runs finished games. You
won't need it until you [ship your game](../../documentation/basics/#shipping-a-game).
:::

## Compile from source

If you'd rather build Kione yourself (or change the engine), you need **CMake** and a
**C++ compiler** — Clang is recommended. Everything else is downloaded automatically during
the build.

### 1. Clone

```sh
git clone https://github.com/gnikdroy/kione.git
cd kione
```

### 2. Build

```sh
cmake -B build
cmake --build build -j
```

The first build takes a few minutes because it downloads and builds Kione's libraries.
It produces three programs in `build/bin/`:

- **`editor`** — the full editor.
- **`player`** — the standalone runtime that ships with your game.
- **`K2_tests`** — the engine's test suite (you can ignore it).

### 3. Run

```sh
./build/bin/editor                                          # open the editor
./build/bin/editor demos/platformer/main.k2project          # open the platformer in the editor
./build/bin/player demos/platformer/main.k2project          # play the platformer directly
./build/bin/player demos/towerdefense/res/game.k2project    # play Crystal Keep, the tower defense
```

## Where to go next

- [Your First Game](../your-first-game/): a step-by-step tutorial. Start here if this is
  your first time.
- [Basics](../../documentation/basics/): what projects, scenes, and assets are.
- [Demos](../../documentation/contributing/#demos): a finished mini-platformer and Crystal
  Keep, a full five-level tower-defense campaign.
