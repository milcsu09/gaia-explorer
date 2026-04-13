# Gaia Explorer

Explore real stars from Gaia data releases.

## Showcase
<div align="center">
  <img src="img/1.png" width="800px">

  <img src="img/2.png" width="800px">

  <img src="img/3.png" width="800px">

  <img src="img/4.png" width="800px">
</div>

## Prerequisites

### SFML

```bash
sudo apt update
sudo apt install libsfml-dev
```

### GLEW

```bash
sudo apt update
sudo apt install libglew-dev
```

## Build

```bash
make
```

# Controls

| Bind | Action |
|-|-|
| `ESC` | Quit |
| `F1` | Toggle GUI |
| `0` or `Home` | Teleport to Sun (`0,0,0`) |
| `W` | Move forward |
| `S` | Move backward |
| `A` | Move left |
| `D` | Move right |
| `Space` | Move up |
| `Control` | Move down |
| `Q` | Roll camera left |
| `E` | Roll camera right |
| `+` | Increase camera exposure |
| `-` | Decrease camera exposure |
| `R` | ×10 camera speed |
| `F` | ÷10 camera speed |
| `C` | Set camera speed to `1c` *(speed of light)* |
| `Scroll Up` | Increase camera speed |
| `Scroll Down` | Decrease camera speed |
| `Right Click` + `Scroll Up` | Zoom in |
| `Right Click` + `Scroll Down` | Zoom out |


## Implementation

Star positions are converted to Cartesian coordinates centered on the Sun (`0,0,0`), derived from Gaia right ascension, declination and parallax measurements.

Positions are stored as floating-point numbers, which makes it easy to project stars to the screen using a shader program. This means positional precision decreases with distance from the origin, but this is not a practical concern for one reason: **Gaia's parallax precision degrades faster.** At distances of thousands of parsecs, parallax uncertainty already introduces more positional error than floating-point imprecision ever would.

## Performance

On my system *(AMD Ryzen 5 3600, NVIDIA GeForce GTX 1650)*, the application renders 1'000'000 stars in real time with a peak frame time of \~6 ms (\~166 FPS).

