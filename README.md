# queen

Queen is a small x86-64 OS/runtime for studying predictable tail latency.

It is not trying to become a general-purpose Unix or run Linux binaries. The
goal is to run Queen-native programs and measure the latency costs of OS
mechanisms such as scheduling, page faults, syscalls, program boundaries, and
user/kernel isolation.

Start with:
- `misc/project.md` for the short thesis.
- `misc/roadmap.md` for the staged plan.
- `misc/predictions.md` for the prediction log.
- `misc/progress.md` for design decisions and measurement notes.