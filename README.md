# ReNodeToCode UE 5.7 Test Project

Unreal Engine 5.7 test project with two bundled plugins:

- `Plugins/NodeToCode` - Blueprint-to-code editor plugin, ported to UE 5.7.
- `Plugins/KemSUinVR` - VR classroom prototype plugin with room creation/join flow, session code handling, teacher controls, and prototype voice/session scaffolding.

## Open

- Project: `ReNodeToCode.uproject`
- Recommended engine: `Unreal Engine 5.7`

## Quick Start

1. Open `ReNodeToCode.uproject` in UE 5.7.
2. Open the level `Content/VRTemplate/VRTemplateMap`.
3. In the World Outliner (`Окно -> Структура` in Russian UI), search for `KemSUinVR_Bootstrap`.
4. Press Play to see the classroom prototype actors and world-space UI.

## Included Plugins

### NodeToCode

- Cloud translation flow is preserved.
- Local Codex handoff mode is added.
- UE 5.7 compatibility fixes are already applied.

### KemSUinVR

Prototype systems currently included:

- VR entry hub with `Create Auditorium` / `Join Auditorium` flow
- LAN session creation and join by generated room code
- room code display for host
- teacher control panels
- shared lecture screen plus local student copy screen concept
- moderation scaffolding: mute, volume, kick, ban
- presentation primitive spawning in the classroom center

## Notes

- This repository keeps project content and plugin source code.
- Generated folders like `Saved`, `Intermediate`, and `DerivedDataCache` are excluded from git.
- Win64 editor plugin binaries are included to make opening the project easier on Windows.

## Main Paths

- Project root: `C:/Users/stud/Desktop/ReNodeToCode 5.7/UE test project/ReNodeToCode`
- KemSUinVR plugin: `C:/Users/stud/Desktop/ReNodeToCode 5.7/UE test project/ReNodeToCode/Plugins/KemSUinVR`
- NodeToCode plugin: `C:/Users/stud/Desktop/ReNodeToCode 5.7/UE test project/ReNodeToCode/Plugins/NodeToCode`
