# DOOM 3 Modding Kit

Doom 3 Modding Kit is basically a kit oriented for modders and people that wants to make total conversion mods.

This fork is base on Defunk's MD5 changes for D3Quest such has:

- Gibs
- Level of Detail
- Binary Mesh
- Binary Animations
- PNG has default image format
- Mikktspace
- ImGui

# CHANGES

Compared to the original _dhewm 3_, the changes of _D3Modding Kit_ worth mentioning are:

- fhDoom:
  - Refactor and UI Cleanup of DoomEdit.
  - Added con_size cvar to scale the console.
  - Removal of death code in various places.
  - Dmap timming stats.
- IcedTech / Darkligth:
  - Add MatBuild for mass texture creation.
  - Restore TypeInfoGen (Mostly for standolone games)
- StormEngine2
  - Support for "standard" cubemap side images.
- TEKUUM-D3 / RBDOOM3-BFG:
  - DoomEdit:
    - Window Snapping.
    - Mousewheel Support for cam window.
    - Tool window uses smaller title bar (more screen space for editing)
    - Cam Win Clip Point Manipulation.
    - Rotate Around Selection.
    - New Brush creation in Cam window.
    - Center on Selection.
    - Closing inspector dialog simply hides it.
    - Modified camera control.
    - BSP doesn't hide the Inspector's console.
    - Added Camera Background Color Selection.
    - Selection Centers views on entity.
    - Restore Command List windows.
  - Added Background music support for maps.
  - Added More Console Information.
  - Quake 1 Style ligth styles.
  - Ambient Music for maps.
- The Dark Mod / Hardcorps
  - Re-implemented water system
- Original Changes:
  - Restore Quake 3 Style console base on left overs in the files.
  - Support for source engine skyboxes base on StormEngine2 Changes.
  - fixed noSpecular flag for lights Base on a dhewn3 rejected PR.
  - Enable Common Controls theme engine for the tools.
  - D3XP And Game dlls are in one folder.
  - WIP Template base for TC Games.
  - Update The Sys Console to use different colors and font.
  - Restored Footsteps sounds by using Prey 06 Code.
  - Set gui_mediumFontLimit and gui_smallFontLimit to "0"
  - Added Missing Material Editor models.
  - Added .guide Support from Quake 4.
  - Refactored MFC Tools

See [Changelog](./CHANGELOG.md) for the original dhewn3 changelog.

## Configuration

See [CONFIGURATION.md](./CONFIGURATION.md) for dhewm3-specific configuration, especially for using gamepads.

## Compiling

Additionally to the [Compiling instructions](./COMPILING.md) we use vcpkg for the handling of the dhewn3 dependencies.

## License

The GNU License can be see here [GNU License](./LICENSE.md) the same goes for the original [DOOM 3 License](./LICENSE_DOOM3.md).