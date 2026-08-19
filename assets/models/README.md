# Replacement models

The game loads optional models from this directory. Missing files are expected: the existing raylib primitives remain the fallback visual.

Default paths are declared once in `src/render/visual_assets.def`. Put a model at one of those paths, or change its registry entry, rebuild, and the corresponding primitive will be replaced automatically.

Recommended format: `.glb`, with the origin at the bottom center, Y up, dimensions close to one world unit, and textures stored inside the GLB. Per-asset scale and vertical offset can be adjusted in `visual_assets.def` without touching gameplay code.
