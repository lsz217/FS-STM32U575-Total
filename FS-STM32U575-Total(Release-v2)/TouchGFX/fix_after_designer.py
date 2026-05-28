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
TEXT = os.path.join(ROOT, "generated", "texts")
ASSETS = os.path.join(ROOT, "assets", "texts")
fixed_count = 0

def write_file(path, content):
    """Write content to path, making the file writable first (Designer makes files read-only)."""
    try:
        os.chmod(path, stat.S_IWRITE | stat.S_IREAD)
    except OSError:
        pass
    with open(path, "w", encoding="utf-8") as f:
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
    with open(sensor_base, "r", encoding="utf-8") as f:
        content = f.read()

    count = content.count("add(__background)")
    if count > 1:
        idx = 0
        occurrences = []
        while True:
            idx = content.find(BG_BLOCK, idx)
            if idx == -1:
                break
            occurrences.append(idx)
            idx += len(BG_BLOCK)

        if len(occurrences) >= 2:
            last = occurrences[-1]
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
    with open(base_cpp, "r", encoding="utf-8") as f:
        content = f.read()

    old = "touchgfx::Callback<FrontendApplicationBase>(this, &FrontendApplication::"
    new = "touchgfx::Callback<FrontendApplicationBase>(this, &FrontendApplicationBase::"

    if old in content:
        content = content.replace(old, new)
        write_file(base_cpp, content)
        count = content.count(new)
        print(f"[FIXED] FrontendApplicationBase: {count} callback(s) -> &FrontendApplicationBase::")
        fixed_count += 1
    else:
        print(f"[OK]    FrontendApplicationBase: callbacks already correct")
else:
    print(f"[SKIP]  FrontendApplicationBase: file not found")

# =============================================================================
# Fix 3 — HomePageViewBase: SwipeContainer missing size
# Designer omits setWidth(320)/setHeight(240) on swipeContainer1,
# causing 0x0 container -> black screen.
# =============================================================================
home_base = os.path.join(GEN, "src", "homepage_screen", "HomePageViewBase.cpp")

if os.path.exists(home_base):
    with open(home_base, "r", encoding="utf-8") as f:
        content = f.read()

    # Check if setWidth is missing after setXY
    old_swipe = "swipeContainer1.setXY(0, 0);\n    swipeContainer1.setPageIndicatorBitmaps"
    new_swipe = "swipeContainer1.setXY(0, 0);\n    swipeContainer1.setWidth(320);\n    swipeContainer1.setHeight(240);\n    swipeContainer1.setPageIndicatorBitmaps"

    if old_swipe in content:
        content = content.replace(old_swipe, new_swipe)
        write_file(home_base, content)
        print(f"[FIXED] HomePageViewBase: added swipeContainer1.setWidth(320) + setHeight(240)")
        fixed_count += 1
    else:
        print(f"[OK]    HomePageViewBase: SwipeContainer size already present")
else:
    print(f"[SKIP]  HomePageViewBase: file not found")

# =============================================================================
# Fix 4 — HomePageViewBase: WiFiModalLink Z-order
# Designer puts add(WiFiModalLink) BEFORE add(swipeContainer1),
# making the modal appear behind the swipe container (invisible / unclickable).
# Move it to the constructor end.
# =============================================================================
if os.path.exists(home_base):
    with open(home_base, "r", encoding="utf-8") as f:
        content = f.read()

    # Pattern: add(WiFiModalLink) followed by a blank line then swipeContainer1
    old_zorder = "    add(WiFiModalLink);\n\n    swipeContainer1.setXY"
    if old_zorder in content:
        # Remove the early add(WiFiModalLink)
        content = content.replace("    add(WiFiModalLink);\n\n    ", "", 1)
        # Add it before the closing brace of the constructor
        # Find the last add(...) before the closing }
        # Insert add(WiFiModalLink) before the '}' of the constructor
        marker = "    add(WirelessConnection);\n}"
        if marker in content:
            content = content.replace(marker,
                "    add(WirelessConnection);\n\n    add(WiFiModalLink);\n}")
        write_file(home_base, content)
        print(f"[FIXED] HomePageViewBase: WiFiModalLink moved to end (Z-order)")
        fixed_count += 1
    else:
        print(f"[OK]    HomePageViewBase: WiFiModalLink Z-order correct or pattern changed")
else:
    print(f"[SKIP]  HomePageViewBase: file not found")

# =============================================================================
# Fix 5 — HomePageViewBase: toggleButton4 callback target
# Designer changes toggleButton4 interaction to gotoApplicationPage*,
# but we need gotoApplicationPageScreenNoTransition -> gotoSensorPageScreenNoTransition.
# =============================================================================
if os.path.exists(home_base):
    with open(home_base, "r", encoding="utf-8") as f:
        content = f.read()

    old_cb = "application().gotoApplicationPageScreenNoTransition();"
    new_cb = "application().gotoSensorPageScreenNoTransition();"

    if old_cb in content:
        content = content.replace(old_cb, new_cb)
        write_file(home_base, content)
        print(f"[FIXED] HomePageViewBase: toggleButton4 -> SensorPage")
        fixed_count += 1
    else:
        print(f"[OK]    HomePageViewBase: toggleButton4 callback already SensorPage")
else:
    print(f"[SKIP]  HomePageViewBase: file not found")

# =============================================================================
# Fix 6 — texts.xml: SensorPage wildcard text entries
# Designer sets these to "New Text" / "New Text1" (plain strings,
# no wildcard placeholder). TextAreaWithOneWildcard needs a wildcard char
# in the typed text to display the buffer content.
# =============================================================================
texts_xml = os.path.join(ASSETS, "texts.xml")

if os.path.exists(texts_xml):
    with open(texts_xml, "r", encoding="utf-8") as f:
        content = f.read()

    changes = {
        '__SingleUse_RBVJ': ('New Text1', '&lt;Temp&gt;'),    # Temperature
        '__SingleUse_NDVJ': ('New Text',  '&lt;Hum&gt;'),     # Humidity
        '__SingleUse_JR7L': ('New Text',  '&lt;CO2&gt;'),     # CO2
        '__SingleUse_1AWP': ('New Text',  '&lt;Heart&gt;'),   # Heart rate
    }

    patched = False
    for text_id, (old_val, new_val) in changes.items():
        # Build a pattern unique enough: the ID + the old value
        old_pattern = f'<Text Id="{text_id}"'
        if old_pattern in content:
            # Find the Translation for this text
            old_trans = f'<Translation Language="GB">{old_val}</Translation>'
            new_trans = f'<Translation Language="GB">{new_val}</Translation>'
            if old_trans in content:
                content = content.replace(old_trans, new_trans)
                patched = True

    if patched:
        write_file(texts_xml, content)
        print(f"[FIXED] texts.xml: wildcard placeholders for SensorPage texts")
        fixed_count += 1
    else:
        print(f"[OK]    texts.xml: SensorPage texts already correct")
else:
    print(f"[SKIP]  texts.xml: file not found")

# =============================================================================
# Fix 7 — Texts.cpp: generated string table
# Designer generates "New Text" / "New Text1" as literal Unicode strings.
# Replace in-place with wildcard character 0x02 (same as other wildcard texts).
# Padding with 0x00 preserves the offset table (LanguageGb.cpp untouched).
# =============================================================================
texts_cpp = os.path.join(TEXT, "src", "Texts.cpp")

if os.path.exists(texts_cpp):
    with open(texts_cpp, "r", encoding="utf-8") as f:
        content = f.read()

    # "New Text1" at offset 72 (10 elements)
    old_nt1 = "0x4e, 0x65, 0x77, 0x20, 0x54, 0x65, 0x78, 0x74, 0x31, 0x0, // @72 \"New Text1\""
    new_nt1 = "0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // @72 \"<Temp>\""

    # "New Text" at offset 92 (9 elements)
    old_nt2 = "0x4e, 0x65, 0x77, 0x20, 0x54, 0x65, 0x78, 0x74, 0x0, // @92 \"New Text\""
    new_nt2 = "0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // @92 wildcard"

    patched_cpp = False
    if old_nt1 in content:
        content = content.replace(old_nt1, new_nt1)
        patched_cpp = True
    if old_nt2 in content:
        content = content.replace(old_nt2, new_nt2)
        patched_cpp = True

    if patched_cpp:
        write_file(texts_cpp, content)
        print(f"[FIXED] Texts.cpp: wildcard chars for SensorPage strings")
        fixed_count += 1
    else:
        print(f"[OK]    Texts.cpp: strings already correct")
else:
    print(f"[SKIP]  Texts.cpp: file not found")

# =============================================================================
# Add future fixes below this line
# =============================================================================

if fixed_count:
    print(f"\n{fixed_count} issue(s) fixed. Ready to build.")
else:
    print(f"\nAll clean - nothing to fix.")
