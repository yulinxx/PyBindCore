# PyBindCore (Phase 2)

Facade-based pybind11 extension for April CAD scripting.

## Module

Built as `Python/apr/_apr_core.pyd` (or `.so` on Linux).

## Python API

```python
import apr

doc = apr.Document.create()
doc.add_line(apr.Vec2(0, 0), apr.Vec2(100, 0))
doc.add_circle(apr.Vec2(50, 0), 5)

snap = doc.export_snapshot()
print(snap.entity_count, snap.bounds)

changes = apr.ApplyChanges()
changes.remove = [entity_id]
doc.apply_changes(changes)
```

## Design notes

- Python never receives raw `AprEntity*` / `SceneManager*`
- Entities are referenced by opaque `EntityRef(id)`
- Scene data crosses the boundary as DTO snapshots (`SceneSnapshot`, `ApplyChanges`)
- `Document.open()` / `Document.save()` are reserved for FileIO integration

## CMake

- `APR_PYBIND_CORE=ON` (default, requires `APR_PYTHON_HOST=ON`)

## Demo

Use the same Python version as the build (see CMake configure output, e.g. 3.12 from vcpkg).
Ensure Engine DLLs are on `PATH` (typically `build/bin_Qt6/Debug` during development).

```powershell
$env:PYTHONPATH = Python
$env:PATH = build/bin_Qt6/Debug;$env:PATH
python Python/apr/scripts/facade_demo.py
```
