---
title: Editor Windows
description: Every panel in the Kione editor and what it's for.
---

The editor uses a docking layout — drag any panel by its tab to rearrange or float it. The
**View** menu toggles panels on and off (and **View → Reset Layout** puts everything back),
and the **top bar** has a light/dark theme switcher and an FPS counter. Each heading below
carries its editor icon.

A typical session flows through the panels like this: add your files in **Assets** → create
and arrange entities in the **Entity Selector** and **Viewport** → configure them in the
**Inspector** → press play in the **Viewport** → read errors and messages in the
**Log Viewer**. Everything else is a specialized tool you'll open when you need it.

## <span class="heading-icon" data-icon="visibility"></span>Viewport 2D

The scene view and your main workspace.

- **Pan** with middle-mouse drag, or hold **Space** and drag with the left mouse.
- **Zoom** with the mouse wheel (toward the cursor).
- **Focus** the selected entity with **F**; reset the view with **Home**.
- **Select** an entity by clicking its sprite.
- **Transform gizmo**: press **W** (translate), **E** (rotate), **R** (scale) to manipulate the
  selected entity.
- The **Play / Stop** button (also Ctrl+P) enters and leaves play mode; scripts run and the
  view switches to the game's MainCamera.

## <span class="heading-icon" data-icon="account_tree"></span>Entity Selector

The scene's entity tree. Create entities, select them (which drives the Inspector and gizmo),
build parent/child hierarchies by reparenting, and duplicate or delete entities. Selection here
is shared with the Viewport and Inspector.

## <span class="heading-icon" data-icon="build"></span>Inspector

Edits the components of the currently selected entity. Add a component, remove one, and tweak
every field (positions, colors, asset references, light parameters, camera settings, and so
on). This is the visual counterpart to the [component fields](../components/) you also reach
from scripts.

## <span class="heading-icon" data-icon="inventory_2"></span>Assets

Manages the project's assets and keeps the [`.k2project` manifest](../basics/#the-project-manifest)
in sync. Add, rename, and remove assets here; each gets a name you reference from components and
scripts. Asset pickers throughout the editor list what you've declared in this window.

## <span class="heading-icon" data-icon="brush"></span>Tile Set

Authors a `.k2tileset` from an `Image`: set the tile size, margin, and spacing, and preview the
resulting grid over the atlas. A TileSet is what a TileMap component draws from.

## <span class="heading-icon" data-icon="grid_on"></span>Tilemap Editor

Paints tiles onto the selected entity's TileMap component. Create or resize the grid, pick a
tile from the tileset palette, and paint or erase cells on a dedicated canvas with its own
pan/zoom. Edits apply to the live component, so they show immediately in the Viewport.

## <span class="heading-icon" data-icon="movie"></span>Animation

Authors `SpriteAnimation` clips (`.k2anim`): choose a texture and lay out frames/timing to
produce the clips an [Animation component](../components/#animation) plays.

## <span class="heading-icon" data-icon="settings"></span>Project Settings

Project-wide settings for the currently open project.

## <span class="heading-icon" data-icon="article"></span>Log Viewer

Shows engine and game log output by severity (trace → critical), with filtering and clearing.
`kione.log(...)` from your scripts appears here, which makes it your primary debugging tool.

## <span class="heading-icon" data-icon="description"></span>File Explorer

Browses the project's files on disk relative to the project root, so you can open scenes and
inspect assets without leaving the editor.
