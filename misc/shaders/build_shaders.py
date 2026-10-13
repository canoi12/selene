#!/usr/bin/env python3
"""
Selene shader build tool.

Compiles canonical GLSL sources (.vs / .fs) into per-backend binaries and emits a
single Lua data module (shaders_data.lua) consumed by the engine.

Pipeline (per shader):
    GLSL canonical source
        |-- glslangValidator -V          --> SPIR-V (Vulkan / D3D12)
        |-- spirv-cross --reflect        --> JSON reflection (metadata)
        |-- spirv-cross --hlsl (d3d11)   --> HLSL source
        `-- spirv-cross --oglsl --es (ES)--> GLSL ES variant

Reflection metadata (input locations/types, UBO bindings, samplers) becomes the
source of truth for the engine's vertex layout and resource bindings.

Dependencies (must be on PATH or under $VULKAN_SDK/Bin):
    * glslangValidator   (GLSL -> SPIR-V)
    * spirv-cross        (SPIR-V -> HLSL / GLSL ES, and --reflect JSON)

Only the Python standard library is used (no pip installs).

Usage examples:
    python build_shaders.py --backend vulkan --out ../../plugins/RenderBatch2D
    python build_shaders.py --backend d3d11  --out ./out
    python build_shaders.py --backend opengl --out ./out
    python build_shaders.py --backend opengles --out ./out
    python build_shaders.py --backend vulkan --list
"""

import argparse
import base64
import json
import os
import re
import shutil
import subprocess
import sys


VALID_BACKENDS = ("vulkan", "d3d11", "opengl", "opengles")
BACKEND_ALL = "all"  # generate every backend variant in one run

# File extension for each shader stage -> discovery stage key.
STAGE_SUFFIXES = {
    "vs": "vertex",
    "fs": "fragment",
}

# Discovery stage key -> the stage name the engine's create_shader accepts.
# (engine shader_type_options = { "vertex", "pixel", "compute" })
ENGINE_STAGE = {
    "vertex": "vertex",
    "fragment": "pixel",
}

# GLSL scalar base type -> engine type name.
SCALAR_TYPES = {
    "float": "float",
    "double": "double",
    "int": "int",
    "uint": "uint",
    "bool": "int",  # engine has no bool vertex type; treat as int
    "byte": "byte",
    "short": "short",
}


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def find_tool(explicit, name, arg_name):
    """Locate an external tool by explicit path, PATH, or $VULKAN_SDK/Bin."""
    if explicit:
        if os.path.isfile(explicit) or shutil.which(explicit):
            return explicit
        eprint(f"[error] provided path for {name} not found: {explicit}")
        sys.exit(2)

    on_path = shutil.which(name)
    if on_path:
        return on_path

    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        exe = name + (".exe" if os.name == "nt" else "")
        cand = os.path.join(sdk, "Bin", exe)
        if os.path.isfile(cand):
            return cand

    eprint(
        f"[error] could not find '{name}'. Put it on PATH, set VULKAN_SDK, "
        f"or pass --{arg_name}."
    )
    sys.exit(2)


def run(cmd, capture=True):
    """Run a subprocess and return its stdout. Exit on failure."""
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE if capture else None,
            stderr=subprocess.PIPE,
            check=False,
        )
    except FileNotFoundError:
        eprint(f"[error] failed to execute: {cmd[0]}")
        sys.exit(2)

    if proc.returncode != 0:
        err = proc.stderr.decode("utf-8", "replace") if proc.stderr else ""
        eprint(f"[error] command failed (rc={proc.returncode}): {' '.join(cmd)}")
        if err.strip():
            eprint(err.strip())
        sys.exit(1)

    return proc.stdout.decode("utf-8", "replace") if capture else ""

# --------------------------------------------------------------------------- #
# Type resolution (from spirv-cross GLSL type strings)
# --------------------------------------------------------------------------- #

def parse_glsl_type(glsl_type):
    """
    Turn a GLSL type string ("vec3", "mat4", "float", "ivec2", "uvec4") into
    (engine_type_name, component_count) used by the engine's layout parser.
    Returns (None, 0) for unsupported/opaque types.
    """
    if not glsl_type:
        return None, 0

    m = re.match(r"^([iub]?)(vec|mat)(\d)$", glsl_type)
    if m:
        base, kind, n = m.group(1), m.group(2), int(m.group(3))
        scalar = SCALAR_TYPES.get(
            {"i": "int", "u": "uint", "b": "int", "": "float"}[base], "float"
        )
        return scalar, n

    # double-precision vectors/matrices (dvecN / dmatN) are uncommon; ignore size.
    m = re.match(r"^d(vec|mat)(\d)$", glsl_type)
    if m:
        return "double", int(m.group(2))

    if glsl_type in SCALAR_TYPES:
        return SCALAR_TYPES[glsl_type], 1

    return None, 0


# --------------------------------------------------------------------------- #
# Reflection parsing
# --------------------------------------------------------------------------- #

def reflect_spirv(spirv_cross, spv_path):
    """Run `spirv-cross --reflect` and return the parsed JSON dict."""
    out = run([spirv_cross, spv_path, "--reflect"])
    try:
        return json.loads(out)
    except json.JSONDecodeError as ex:
        eprint(f"[error] failed to parse reflection JSON for {spv_path}: {ex}")
        eprint("--- raw output ---")
        eprint(out[:2000])
        sys.exit(1)


def _resolve_type(types_table, type_ref):
    """Resolve a type reference into a GLSL type string.

    Two shapes appear in spirv-cross --reflect JSON:
      * direct GLSL type strings on inputs/outputs (e.g. "vec3", "mat4");
      * ID references into the 'types' table (e.g. "_17") on ubos/ssbos and
        their members.
    Returns the GLSL type string, or None if it cannot be resolved.
    """
    if not type_ref:
        return None

    # Direct GLSL type string (inputs/outputs): it is not an "_N" id.
    if isinstance(type_ref, str) and not re.match(r"^_\d+$", type_ref):
        return type_ref

    key = str(type_ref)
    entry = types_table.get(key) if isinstance(types_table, dict) else None
    if entry is None and isinstance(types_table, list):
        try:
            idx = int(re.sub(r"\D", "", key) or -1)
            entry = types_table[idx] if 0 <= idx < len(types_table) else None
        except ValueError:
            entry = None
    if entry is None:
        return None
    return entry.get("type")


def _resolve_type_entry(types_table, type_ref):
    """Return the raw 'types' table entry for a type id reference (or None)."""
    if not type_ref:
        return None
    key = str(type_ref)
    entry = types_table.get(key) if isinstance(types_table, dict) else None
    if entry is None and isinstance(types_table, list):
        try:
            idx = int(re.sub(r"\D", "", key) or -1)
            entry = types_table[idx] if 0 <= idx < len(types_table) else None
        except ValueError:
            entry = None
    return entry


def parse_reflection(refl):
    """
    Convert spirv-cross --reflect JSON into clean metadata:
        inputs, outputs, ubos, ssbos, samplers, images
    """
    types_table = refl.get("types", {}) or {}
    meta = {"inputs": [], "outputs": [], "ubos": [], "ssbos": [],
            "samplers": [], "images": []}

    def resources(kind):
        """Collect a resource list from both top-level and any entryPoints.

        spirv-cross --reflect puts inputs/outputs/ubos at the top level (the
        entryPoints array only carries name/mode). Some versions nest them under
        entryPoints instead, so read both and de-duplicate.
        """
        out = list(refl.get(kind, []) or [])
        eps = refl.get("entryPoints")
        if isinstance(eps, list):
            for ep in eps:
                if isinstance(ep, dict):
                    for res in ep.get(kind, []) or []:
                        if res not in out:
                            out.append(res)
        return out

    def add_interface(dst, res):
        glsl_type = _resolve_type(types_table, res.get("type"))
        eng_type, size = parse_glsl_type(glsl_type)
        dst.append({
            "location": res.get("location"),
            "name": res.get("name"),
            "type": eng_type,
            "glsl_type": glsl_type,
            "size": size,
        })

    for res in resources("inputs"):
        add_interface(meta["inputs"], res)
    for res in resources("outputs"):
        add_interface(meta["outputs"], res)

    def add_block(dst, res):
        # Members are usually not inlined on the block; they live in the 'types'
        # table under the block's type id (e.g. "_17"). Resolve from there, but
        # also honour inlined 'members' if a future version provides them.
        raw_members = res.get("members")
        if not raw_members:
            block_type = _resolve_type_entry(types_table, res.get("type"))
            if block_type:
                raw_members = block_type.get("members", [])

        members = []
        for mem in raw_members or []:
            mem_glsl = _resolve_type(types_table, mem.get("type"))
            eng_type, size = parse_glsl_type(mem_glsl)
            members.append({
                "name": mem.get("name"),
                "type": eng_type,
                "glsl_type": mem_glsl,
                "offset": mem.get("offset"),
                "size": size,
                "array": mem.get("array"),
            })
        dst.append({
            "set": res.get("set", 0),
            "binding": res.get("binding"),
            "name": res.get("name"),
            "block_size": res.get("block_size"),
            "members": members,
        })

    for res in resources("ubos"):
        add_block(meta["ubos"], res)
    for res in resources("ssbos"):
        add_block(meta["ssbos"], res)

    # Sampled textures: spirv-cross reports combined samplers under "textures"
    # (e.g. sampler2D), and split resources under "separate_images"/"separate_samplers".
    # Older/other versions use "samplers" or "images". Collect all, de-duplicated.
    seen = set()
    for kind in ("textures", "samplers", "separate_images",
                 "separate_samplers", "images"):
        for res in resources(kind):
            sig = (res.get("set", 0), res.get("binding"), res.get("name"))
            if sig in seen:
                continue
            seen.add(sig)
            dst = meta["samplers"] if kind in ("textures", "samplers") else meta["images"]
            dst.append({
                "set": res.get("set", 0),
                "binding": res.get("binding"),
                "name": res.get("name"),
                "glsl_type": _resolve_type(types_table, res.get("type"))
                if isinstance(res.get("type"), str) and re.match(r"^_\d+$", res.get("type"))
                else res.get("type"),
            })

    def loc_key(e):
        v = e["location"]
        return (v is None, v if v is not None else 0)

    def bind_key(e):
        return (e["set"] if "set" in e else 0, e.get("binding") or 0)

    meta["inputs"].sort(key=loc_key)
    meta["outputs"].sort(key=loc_key)
    meta["ubos"].sort(key=bind_key)
    meta["ssbos"].sort(key=bind_key)
    meta["samplers"].sort(key=bind_key)
    meta["images"].sort(key=bind_key)

    return meta


# --------------------------------------------------------------------------- #
# Lua serialization
# --------------------------------------------------------------------------- #

def lua_escape(s):
    """Escape a string for a double-quoted Lua literal."""
    out = []
    for ch in s:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\r":
            out.append("\\r")
        elif ch == "\t":
            out.append("\\t")
        elif ord(ch) < 32:
            out.append("\\%d" % ord(ch))
        else:
            out.append(ch)
    return "".join(out)


def to_lua(obj, indent=0):
    """Serialize a Python object (dict/list/str/int/float/bool/None) as Lua."""
    pad = "    " * indent
    pad2 = "    " * (indent + 1)

    if obj is None:
        return "nil"
    if isinstance(obj, bool):
        return "true" if obj else "false"
    if isinstance(obj, (int, float)):
        return repr(obj)
    if isinstance(obj, str):
        return '"' + lua_escape(obj) + '"'

    if isinstance(obj, dict):
        if not obj:
            return "{}"
        lines = ["{"]
        for k, v in obj.items():
            key = (k if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", str(k))
                   else "[" + to_lua(str(k)) + "]")
            lines.append(f"{pad2}{key} = {to_lua(v, indent + 1)},")
        lines.append(pad + "}")
        return "\n".join(lines)

    if isinstance(obj, (list, tuple)):
        if not obj:
            return "{}"
        lines = ["{"]
        for v in obj:
            lines.append(f"{pad2}{to_lua(v, indent + 1)},")
        lines.append(pad + "}")
        return "\n".join(lines)

    raise TypeError(f"cannot serialize type {type(obj)}")


def b85(data):
    """Base85-encode bytes as an ASCII string safe to embed in Lua source."""
    return base64.b85encode(data).decode("ascii")


# --------------------------------------------------------------------------- #
# Compilation
# --------------------------------------------------------------------------- #

def compile_spirv(glslang, src_path, stage, out_spv, defines=None):
    """Compile a GLSL source stage to SPIR-V. Returns the output .spv path."""
    suffix = "vert" if stage == "vertex" else "frag"
    cmd = [glslang, src_path, "-V", "--target-env", "vulkan1.0",
           "-S", suffix, "-o", out_spv]
    if defines:
        for d in defines:
            cmd.insert(-2, d if d.startswith("-D") else f"-D{d}")
    run(cmd)
    return out_spv


def transpile_hlsl(spirv_cross, spv_path, out_hlsl, stage):
    """SPIR-V -> HLSL source (D3D11). Writes and returns the HLSL text."""
    suffix = "vert" if stage == "vertex" else "frag"
    out = run([spirv_cross, spv_path, "--hlsl", "--shader-model", "50",
               "--stage", suffix])
    with open(out_hlsl, "w", encoding="utf-8", newline="\n") as f:
        f.write(out)
    return out


def transpile_gles(spirv_cross, spv_path, out_glsl, stage):
    """SPIR-V -> GLSL ES source (mobile). Writes and returns the GLSL ES text.

    spirv-cross emits OpenGL-style GLSL by default; `--es` forces ESSL.
    """
    suffix = "vert" if stage == "vertex" else "frag"
    out = run([spirv_cross, spv_path, "--es", "--stage", suffix])
    with open(out_glsl, "w", encoding="utf-8", newline="\n") as f:
        f.write(out)
    return out


# --------------------------------------------------------------------------- #
# Shader discovery
# --------------------------------------------------------------------------- #

def discover_shaders(src_dir):
    """
    Find shader base names in src_dir that have at least a .vs file.
    Returns { name: { 'vertex': path, 'fragment': path (optional) } }.
    """
    shaders = {}
    for fname in sorted(os.listdir(src_dir)):
        base, ext = os.path.splitext(fname)
        ext = ext.lstrip(".").lower()
        if ext not in STAGE_SUFFIXES:
            continue
        stage = STAGE_SUFFIXES[ext]
        shaders.setdefault(base, {})[stage] = os.path.join(src_dir, fname)
    return shaders


# --------------------------------------------------------------------------- #
# Main build
# --------------------------------------------------------------------------- #

def encode_variant(spirv_cross, name, stage, spv_path, src, workdir, want):
    """Build the requested backend variant(s) for one stage.

    `want` is the set of backend keys to produce. Returns {backend: {enc, data}}.
    """
    blobs = {}

    # SPIR-V (Vulkan, and D3D12 later) — always available, embedded as base85.
    if "vulkan" in want:
        with open(spv_path, "rb") as f:
            blobs["vulkan"] = {"enc": "base85", "data": b85(f.read())}

    # HLSL (D3D11) — transpiled from SPIR-V, embedded as text.
    if "d3d11" in want:
        hlsl = transpile_hlsl(spirv_cross, spv_path,
                              os.path.join(workdir, f"{name}.{stage}.hlsl"), stage)
        blobs["d3d11"] = {"enc": "text", "data": hlsl}

    # GLSL ES (mobile) — transpiled from SPIR-V, embedded as text.
    if "opengles" in want:
        gles = transpile_gles(spirv_cross, spv_path,
                              os.path.join(workdir, f"{name}.{stage}.gles.glsl"), stage)
        blobs["opengles"] = {"enc": "text", "data": gles}

    # Desktop GLSL — the canonical source text itself, embedded as text.
    if "opengl" in want:
        with open(src, "r", encoding="utf-8") as f:
            blobs["opengl"] = {"enc": "text", "data": f.read()}

    return blobs


def build(args):
    src_dir = os.path.abspath(args.src)
    out_dir = os.path.abspath(args.out)
    os.makedirs(out_dir, exist_ok=True)

    glslang = find_tool(args.glsllang, "glslangValidator", "glsllang")
    spirv_cross = find_tool(args.spirv_cross, "spirv-cross", "spirv_cross")

    shaders = discover_shaders(src_dir)
    if not shaders:
        eprint(f"[error] no shaders (.vs/.fs) found in {src_dir}")
        sys.exit(1)

    if args.list:
        print("Discovered shaders:")
        for name, stages in shaders.items():
            have = ", ".join(ENGINE_STAGE[s] for s in sorted(stages.keys()))
            print(f"  {name}: {have}")
        return

    names = ([n.strip() for n in args.shaders.split(",") if n.strip()]
             if args.shaders else sorted(shaders.keys()))

    want = set(VALID_BACKENDS) if args.backend == BACKEND_ALL else {args.backend}
    workdir = os.path.join(out_dir, "_work")
    os.makedirs(workdir, exist_ok=True)

    written = []
    for name in names:
        if name not in shaders:
            eprint(f"[warn] requested shader '{name}' not found; skipping")
            continue

        stages = shaders[name]
        entry = {}
        warnings = []
        skip = False

        for stage in ("vertex", "fragment"):
            src = stages.get(stage)
            if not src:
                if stage == "vertex":
                    eprint(f"[error] shader '{name}' has no vertex (.vs) stage; skipping")
                    skip = True
                    break
                continue

            spv_path = os.path.join(workdir, f"{name}.{stage}.spv")
            compile_spirv(glslang, src, stage, spv_path, args.defines)

            refl = reflect_spirv(spirv_cross, spv_path)
            meta = parse_reflection(refl)
            blobs = encode_variant(spirv_cross, name, stage, spv_path, src,
                                   workdir, want)

            if stage == "vertex" and not meta["inputs"]:
                warnings.append("no vertex inputs detected")

            entry[ENGINE_STAGE[stage]] = {"bin": blobs, "meta": meta}

        if skip:
            continue
        if warnings:
            entry["_warnings"] = warnings

        # One self-contained *_data.lua per shader.
        header = (
            "-- AUTO-GENERATED by misc/shaders/build_shaders.py -- DO NOT EDIT.\n"
            f"-- shader: {name}\n"
            f"-- backends: {', '.join(sorted(want))}\n"
            "-- Canonical source lives in misc/shaders/*.vs|*.fs\n"
        )
        body = "return " + to_lua(entry, 0) + "\n"
        out_lua = os.path.join(out_dir, f"{name}_data.lua")
        with open(out_lua, "w", encoding="utf-8", newline="\n") as f:
            f.write(header + body)
        written.append(out_lua)

        vmeta = entry.get("vertex", {}).get("meta", {})
        print(f"[ok] {name}: {len(vmeta.get('inputs', []))} inputs, "
              f"{len(vmeta.get('ubos', []))} ubos, "
              f"{len(vmeta.get('samplers', [])) + len(vmeta.get('images', []))} samplers (vs)")

    print()
    for w in written:
        print(f"[done] wrote {w}")
    if written:
        print(f"[done] intermediates in {workdir}")


def main():
    p = argparse.ArgumentParser(
        description="Compile canonical GLSL shaders to per-backend variants and "
                    "emit one self-contained *_data.lua per shader.")
    p.add_argument("--backend", choices=VALID_BACKENDS + (BACKEND_ALL,),
                   default=BACKEND_ALL,
                   help="which backend variant(s) to emit (default: all)")
    p.add_argument("--src", default=os.path.dirname(os.path.abspath(__file__)),
                   help="directory containing .vs/.fs canonical sources")
    p.add_argument("--out", default="../../plugins/RenderBatch2D/shaders",
                   help="output directory for the *_data.lua files "
                        "(default: plugins/RenderBatch2D/shaders)")
    p.add_argument("--shaders", default=None,
                   help="comma-separated shader names to build (default: all)")
    p.add_argument("--defines", default=None,
                   help="comma-separated preprocessor defines (e.g. DEBUG,USE_SHADOWS=1)")
    p.add_argument("--list", action="store_true",
                   help="list discovered shaders and exit")
    p.add_argument("--glsllang", default=None, help="path to glslangValidator")
    p.add_argument("--spirv_cross", default=None, help="path to spirv-cross")
    args = p.parse_args()

    args.defines = args.defines.split(",") if args.defines else None
    build(args)


if __name__ == "__main__":
    main()

