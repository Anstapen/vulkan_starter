---
description: Format C++ source with clang-format
argument-hint: "[optional path or glob, defaults to whole repo]"
---
Run `clang-format -i` (in-place) using the project's `.clang-format` style on:
- $ARGUMENTS, if given
- otherwise all `.h`/`.cpp` files under `Core/` and `App/`

Then run `git diff --stat` to show which files changed and by how much, and give a one-sentence summary. Don't touch anything outside of applying clang-format — no other edits.
