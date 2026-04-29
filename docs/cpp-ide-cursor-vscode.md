# C++ editing: Cursor vs VS Code

This repo’s CMake/Conan setup is editor-agnostic. **IntelliSense / diagnostics** behave differently in **Cursor** than in **stock VS Code** because Cursor does not ship the full Microsoft **cpptools** language server (native binaries such as `cpptools` / `cpptools-srv` are often missing under `anysphere.cpptools`). Cursor documents relying on **clangd** and a **`compile_commands.json`** instead; see [this Cursor forum thread](https://forum.cursor.com/t/the-c-extension-anysphere-cpptools-is-missing-executable-files/128728/3).

## What everyone gets from the repo

- **`CMAKE_EXPORT_COMPILE_COMMANDS`** in CMake → **`build/compile_commands.json`** after configure (real compiler flags, Conan include paths, `-std=c++20`).
- **`.clangd`** → `CompilationDatabase: build` so **clangd** finds that database (same relative layout as **`clangd.arguments`** below).
- **`.vscode/settings.json`** → **`clangd.arguments`** used by the **vscode-clangd** extension:
  - **`--compile-commands-dir=${workspaceFolder}/build`** — compile DB location (explicit when the editor opens nested folders or symlink layouts confuse discovery).
  - **`--background-index`** — index the rest of the project for navigation.
  - **`--query-driver=…`** — see **clangd, libstdc++, and `--query-driver` (Linux)** below for the exact globs. Allowlists **`g++` / `c++`** so clangd may **execute the compiler from `compile_commands.json`** and learn **libstdc++** include paths that match your toolchain ([clangd **System headers** / `--query-driver`](https://clangd.llvm.org/guides/system-headers#query-driver)). Fixes bogus **`cstddef`**, **`std::…`**, and **`std::floating_point`**-style diagnostics when Clang’s default heuristic picks the wrong GCC version ([clangd **Troubleshooting**](https://clangd.llvm.org/troubleshooting)). After changing these flags, run **Clangd: Restart language server**.

On **Windows**, the checked-in **`--query-driver`** globs usually match nothing (drivers are **`cl.exe`**, Clang-on-MSVC, etc.); clangd then relies on default inference from **`compile_commands.json`**. If you use MinGW/MSYS **`g++.exe`** in **`compile_commands.json`**, add a matching glob under **`clangd.arguments`** (your User settings are fine).

VS Code, Neovim, CLion (with clangd), etc. can reuse **`compile_commands.json`** and **`.clangd`**; Neovim/CLI users pass the same flags as **`clangd.arguments`** in their LSP config.

## Cursor-only files

These do **not** affect teammates who only use VS Code or other tools:

| Path | Purpose |
|------|--------|
| **`.cursor/settings.json`** | Suggested Cursor overrides only (**disable Microsoft IntelliSense** so it does not fight **clangd**). **`clangd`** CLI flags live in **`.vscode/settings.json`** so VS Code and Cursor stay aligned. |
| **`.cursor/rules/cpp-clangd-cursor.md`** | Short rule snippet for Cursor agents (same ideas as this doc). |

## What you should install

1. **`clangd`** language server on your OS (e.g. `sudo apt install clangd` on Debian/Ubuntu).
2. The **clangd** VS Code/Cursor extension (`llvm-vs-code-extensions.vscode-clangd`), optional but recommended.

Install the **`clangd` binary** — the extension alone is not enough.

## Conflicting extensions (Linux “Kylin” stacks)

If **clangd** or **CMake Tools** stay stuck **Activating…** and logs mention commands already registered by **Kylin Clangd** / **Kylin CMake**, disable or remove those Kylin extensions so they do not duplicate **clangd** / **CMake** command IDs.

## Optional: `compile_commands.json` at repo root

Some tools only search the workspace root. After CMake configure you can symlink (gitignored locally):

```bash
ln -sf build/compile_commands.json compile_commands.json
```

## clangd, libstdc++, and `--query-driver` (Linux)

More detail for **`clangd.arguments`** in **What everyone gets from the repo** above.

**Symptom:** **`cmake --build`** succeeds but clangd reports **`cstddef` not found**, missing **`std::…`** / **`concepts`**, or “noise” under **`#pragma once`** — often Clang guessed the wrong **GCC** library paths (for example **`/usr/lib/gcc/…/14`** present while headers exist only under **`/usr/include/c++/13`**).

**In-repo flags:** **`.vscode/settings.json`** sets (among others):

- **`--compile-commands-dir=${workspaceFolder}/build`**
- **`--query-driver=/usr/bin/c++,/usr/bin/g++,/usr/bin/g++-*,/usr/bin/x86_64-linux-gnu-g++-*`**

See [clangd **System headers** → Query-driver](https://clangd.llvm.org/guides/system-headers#query-driver) and [clangd **Troubleshooting**](https://clangd.llvm.org/troubleshooting). After toolchain changes or pulling edits to **`clangd.arguments`**, run **Clangd: Restart language server**.

If your **`compile_commands.json`** uses a compiler outside those globs (cross-compile SDK, custom **`install/` tree**), add a matching glob to **`clangd.arguments`** (often via **User** settings only), or install **`libstdc++-*-dev`** / **`g++-*`** so headers match **`gcc`**’s layout on disk.

## Debugging (GDB) vs IntelliSense

- **Squiggles / completion** → **clangd** with **`build/compile_commands.json`**, **`.clangd`**, and **`clangd.arguments`** in **`.vscode/settings.json`** (including Linux **`--query-driver`** when using GCC/libstdc++; see above).
- **Breakpoints / gdb** → `.vscode/launch.json` uses **`cppdbg`** (Microsoft debugger adapter). You still need a debugger extension that provides **`cppdbg`** for **Run and Debug**; that is separate from **clangd** IntelliSense.

See **README.md** for build tasks (**Build Debug (Conan)**) and the usual **Run and Debug** flow once Conan + CMake have been configured.

## Quiet noisy squiggles (optional)

If **`std::…`** / **`concepts`** look broken everywhere even though **`cmake --build`** succeeds, read **clangd, libstdc++, and `--query-driver` (Linux)** above first; restarting alone cannot repair wrong system include paths.

If underlines still appear after **Developer: Reload Window**, they are usually **clangd** diagnostics from **`compile_commands.json`** / parsing, not a generic editor setting. VS Code’s **`editor.renderValidationDecorations`** does **not** disable clangd underlines; clangd publishes diagnostics via LSP ([upstream discussion](https://github.com/clangd/vscode-clangd/issues/836)).

**First, fix or refresh the language server**

1. Configure CMake so **`build/compile_commands.json`** exists (see **README.md**).
2. Command Palette: **Clangd: Restart language server** (picks up a new DB without a full window reload).

**Spell checker:** If **Code Spell Checker** (or similar) flags identifiers, this repo’s **`.vscode/settings.json`** sets **`cSpell.enabled`: `false`** for the **`[cpp]`** language ID (ignored if that extension is not installed).

**Hide all clangd squiggles** (navigation/completion still partly work; you lose inline error feedback): use clangd’s config hook **`Diagnostics` → `Suppress`** ([clangd config](https://clangd.llvm.org/config.html#suppress)). For example, in **project** **`.clangd`** (team-visible) or **user** **`~/.config/clangd/config.yaml`** (Linux, private):

```yaml
Diagnostics:
  Suppress: "*"
```

Use **`"*"`** with quotes so YAML does not treat `*` as special. Uncomment the same block in this repo’s **`.clangd`** if you want it only here (prefer **`compile_commands.json`**, **`.clangd`**, and **`--query-driver`** so diagnostics stay meaningful).

**Still unsure who draws the line?** Open the **Problems** panel and hover an entry, or check the bottom-right **language mode** for the file (**C++** for `.hpp`). Extensions such as clangd usually label diagnostics with the extension name.
