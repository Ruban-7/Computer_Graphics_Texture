# Olympic Town

Ruban Chakaravarthi Vairava Sundaram
CSCI4229/5229 Fall 2025

## OVERVIEW
This OpenGL program implements the lighting.

## Features
1. Olympic ring
2. Soccer Court
3. Stadium Gallery
4. Stadium light tower
5. olympic cauldron
6. Swimming pool
7. Two Mickey Mouse
8. Ticket booth
9. Mountains 
10. sky scrapers - Players dorm

## Texture
The  Texture are added for following objects:

stadium light tower, cauldron, sky scrapers, tree leaf, ticketbooth wall, stadium wall, swimming pool handrail, tree bark, swimming pool water, swimming pool wall, and swimming pool deck

Without any additional textures, I think the mountain, Olympic rings, and Mickey Mouse are good.

## CONTROLS

### Object Selection
- **l** (lowercase L): Cycle forward through objects/scenes (0-9)
- **L** (uppercase L): Cycle backward through objects/scenes
  - 0: Full scene with all objects
  - 1: Swimming pool
  - 2: Athletics stadium
  - 3: Olympic cauldron
  - 4: Skyscraper
  - 5: Mountain
  - 6: Mickey Mouse
  - 7: Palm tree
  - 8: Ticket booth
  - 9: Olympic rings

### Texture Controls

- **6** Increase the size of tree bark and tree leaf
- **7** Decrease the size of tree bark and tree leaf
- **n** Increase the sky scraper texutre's repetition 
- **N** Decrease the sky scraper texutre's repetition 
**s/S** Turn off and then resume the stadium wall animation
**c/C** Switch between red, green, blue, and white for ticketbooth roof


### Orbital Light Controls
- **o/O**: Toggle orbital light on/off
- **p/P**: Toggle automatic orbital light orbiting
- **y/Y**: Lower/rise orbital light elevation (2.0 unit increments, -10.0 to 25.0 range)
- **t/T**: Rotate orbital light position (10 degree increments)
- **i/I**: Decrease/increase orbital ambient light (5% increments, 0-100% range)
- **j/J**: Decrease/increase orbital diffuse light (5% increments, 0-100% range)
- **k/K**: Decrease/increase orbital specular light (5% increments, 0-100% range)
- **b/B**: Decrease/increase material shininess 
- **g**: Toggle orbital light distance (10 units for close/50 units for far):Can be used only in full Scene not in individual Object
- **G**: Toggle smooth/flat shading 
- **w/W**: Reset orbital light to default position

### View Mode Controls
- **m/M**: Cycle through view modes (Orthogonal → Perspective → First Person)
- **r/R**: Reset camera/view to default position
- **h/H**: Display help information in console

### Camera Controls (Orthogonal/Perspective Mode)
- **Arrow Keys**: Rotate the view
- **Mouse Drag**: Rotate view by dragging with left mouse button
- **+/- Keys**: Zoom in/out
- **Mouse Wheel**: Scroll to zoom in/out

### First Person Camera Controls
- **Arrow Keys**: Move forward/backward, strafe left/right
- **q/e**: Turn left/right
- **z/x**: Look up/down
- **Mouse Drag**: Look around (hold left mouse button and move mouse)
- **+/- Keys**: Increase/decrease movement speed
- **1/2 Keys**: Decrease/increase mouse sensitivity

### General Controls
- **ESC**: Exit program

### Compilation
```bash
make
```
### Execution
```bash
./hw6.exe
```

## Completion Time
**Approximate time to complete**: 15 hours


## Comments:
* Field should be textured
* Stadium seats should be textured
* Texture dimensions should be: Square, a Power of 2, less than or equal to 512! Please use "GNU Image Manipulation Program" (or your favored software) to correct this issue.

## Updates:
I did all the changes in my code.


