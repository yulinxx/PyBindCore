# PyBindCore (Phase 2)

Facade-based pybind11 extension for SanYi CAD scripting.

## Module

Built as `Python/sanyi/_sanyi_core.pyd` (or `.so` on Linux).

## Python API

```python
import sanyi

doc = sanyi.Document.create()
doc.add_line(sanyi.Vec2(0, 0), sanyi.Vec2(100, 0))
doc.add_circle(sanyi.Vec2(50, 0), 5)

snap = doc.export_snapshot()
print(snap.entity_count, snap.bounds)

changes = sanyi.ApplyChanges()
changes.remove = [entity_id]
doc.apply_changes(changes)
```

## Design notes

- Python never receives raw `SyEntity*` / `SceneManager*`
- Entities are referenced by opaque `EntityRef(id)`
- Scene data crosses the boundary as DTO snapshots (`SceneSnapshot`, `ApplyChanges`)
- `Document.open()` / `Document.save()` are reserved for FileIO integration

## CMake

- `SANYI_PYBIND_CORE=ON` (default, requires `SANYI_PYTHON_HOST=ON`)

## Demo

Use the same Python version as the build (see CMake configure output, e.g. 3.12 from vcpkg).
Ensure Engine DLLs are on `PATH` (typically `build/bin_Qt6/Debug` during development).

```powershell
$env:PYTHONPATH = Python
$env:PATH = build/bin_Qt6/Debug;$env:PATH
python Python/sanyi/scripts/facade_demo.py
```
