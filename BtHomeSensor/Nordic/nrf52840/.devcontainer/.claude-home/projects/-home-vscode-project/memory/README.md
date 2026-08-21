# Claude Code memory

Persistent memory for the `zephyr` devcontainer project, written by Claude Code
across sessions (see `MEMORY.md` for the index once it exists).

This lives inside `.devcontainer/.claude-home/`, which is bind-mounted from a
folder in the project's own git repo into the container at `/home/vscode/.claude`.
That mount already survives container *rebuilds*. Everything else under
`.claude-home` (OAuth credentials, session keys, raw conversation transcripts,
shell snapshots) is gitignored and stays local-only — only this `memory/`
folder is tracked, so it travels with the repo to any machine that clones it.

Note: the `.git` for this project lives in a parent folder on the host, above
the devcontainer's `workspaceMount`, so it isn't visible from inside the
container. Commit and push changes here from the host, after the container
is stopped (or from any host terminal with access to the full repo checkout).
