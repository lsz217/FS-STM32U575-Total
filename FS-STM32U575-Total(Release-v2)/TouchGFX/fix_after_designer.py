#!/usr/bin/env python3
"""
Run this script after every TouchGFX Designer "Generate Code".
Fixes known Designer bugs that recur on every code generation.
Only patches files under generated/ — never touches gui/ user code.

Usage:  python fix_after_designer.py
"""
import os, stat, sys

ROOT = os.path.dirname(os.path.abspath(__file__))
GEN  = os.path.join(ROOT, "generated", "gui_generated")
fixed_count = 0

def write_file(path, content):
    """Write content to path, making the file writable first (Designer makes files read-only)."""
    try:
        os.chmod(path, stat.S_IWRITE | stat.S_IREAD)
    except OSError:
        pass
    with open(path, "w") as f:
        f.write(content)

# =============================================================================
# Fix 1 — SensorPageViewBase: duplicate add(__background)
# Designer bug: sometimes emits the __background block twice, which causes
# a runtime hard fault when the same widget is added to the container twice.
# =============================================================================
sensor_base = os.path.join(GEN, "src", "sensorpage_screen", "SensorPageViewBase.cpp")
BG_BLOCK = (
    "    __background.setPosition(0, 0, 320, 240);\n"
    "    __background.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));\n"
    "    add(__background);"
)

if os.path.exists(sensor_base):
    with open(sensor_base, "r") as f:
        content = f.read()

    count = content.count("add(__background)")
    if count > 1:
        # Remove the LAST occurrence of the block (keep the first)
        # Find all occurrences of BG_BLOCK
        idx = 0
        occurrences = []
        while True:
            idx = content.find(BG_BLOCK, idx)
            if idx == -1:
                break
            occurrences.append(idx)
            idx += len(BG_BLOCK)

        if len(occurrences) >= 2:
            # Remove the last one
            last = occurrences[-1]
            # Also remove the preceding newline
            start = last - 1 if last > 0 and content[last-1] == '\n' else last
            end = last + len(BG_BLOCK)
            content = content[:start] + content[end:]
            write_file(sensor_base, content)
            print(f"[FIXED] SensorPageViewBase: removed duplicate add(__background) "
                  f"({count} -> 1)")
            fixed_count += 1
    else:
        print(f"[OK]    SensorPageViewBase: no duplicate")
else:
    print(f"[SKIP]  SensorPageViewBase: file not found")

# =============================================================================
# Fix 2 — FrontendApplicationBase: Callback type conflict
# Designer always writes &FrontendApplication::methodName, which causes a type
# mismatch if the user's derived class declares a method with the same name.
# Changing to &FrontendApplicationBase:: makes it permanently safe.
# =============================================================================
base_cpp = os.path.join(GEN, "src", "common", "FrontendApplicationBase.cpp")

if os.path.exists(base_cpp):
    with open(base_cpp, "r") as f:
        content = f.read()

    old = "touchgfx::Callback<FrontendApplicationBase>(this, &FrontendApplication::"
    new = "touchgfx::Callback<FrontendApplicationBase>(this, &FrontendApplicationBase::"

    if old in content:
        content = content.replace(old, new)
        write_file(base_cpp, content)
        count = content.count(new)
        print(f"[FIXED] FrontendApplicationBase: {count} callback(s) → &FrontendApplicationBase::")
        fixed_count += 1
    else:
        print(f"[OK]    FrontendApplicationBase: callbacks already correct")
else:
    print(f"[SKIP]  FrontendApplicationBase: file not found")

# =============================================================================
# Add future fixes below this line
# =============================================================================

if fixed_count:
    print(f"\n{fixed_count} issue(s) fixed.")
else:
    print(f"\nAll clean — nothing to fix.")
