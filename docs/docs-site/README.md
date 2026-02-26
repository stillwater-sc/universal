# Content management for the documentation site

The documentation site at `docs-site/` is built with [Starlight](https://starlight.astro.build/) (Astro).
Most of its content is **not** hand-written inside `docs-site/` — it is synced from the `docs/` directory
by `docs-site/sync-content.mjs`.

## How content syncing works

1. `docs-site/sync-content.mjs` runs automatically as a prebuild step (`npm run build` / `npm run dev`)
2. It reads markdown files from `docs/` (and a few repo-root files like `PURPOSE.md`, `CHANGELOG.md`)
3. It extracts the first `# Heading` as the Starlight frontmatter `title:`
4. It rewrites image paths and writes the transformed files into `docs-site/src/content/docs/`
5. The mapping is hardcoded in `FILE_MAP` and `ROOT_FILE_MAP`

The `src/content/docs/` directory is where Starlight expects content. Some pages are hand-written
directly there (those are skipped by sync to avoid overwriting), but most are synced from `docs/`.

**To add a new page to the site**: add an entry to `FILE_MAP` in `sync-content.mjs`, then create or
edit the corresponding markdown file in `docs/`. The sync step will pick it up on the next build.

## What is synced (the curated set)

- All 27 `number-systems/*.md` pages (minus 3 cascade variants and README)
- 6 tutorials (command-line tools, type parameterization, posit refinement, arbitrary precision, multi-component, end-of-error)
- 5 mixed-precision docs (methodology, SDK, findings, utilities, block formats)
- 5 design docs (floatcascade, error propagation, error tracker, multi-limb, decimal conversion)
- 4 build docs (cross-compilation, code formatting, troubleshooting, linux packages)
- 2 contributing docs (release process from `docs/`, contributors + code-of-conduct from repo root)
- 2 root files (`PURPOSE.md`, `CHANGELOG.md`)

## What is NOT synced

| Category | Files | Reason |
|----------|-------|--------|
| Internal working notes | `cascade_math_propagation_plan.md`, `mp_iterative_refinement.md`, `ereal_limb_limit_derivation.md` | Implementation scratch work |
| Code review / refactor notes | `fixpnt_impl_code_review.md`, `header-guard-migration.md`, `modernization-recommendations.md`, `tooling-integration-plan.md` | Internal process docs |
| Research notes | `priest.md`, `position-paper-analysis.md`, `decimal_scientific.md`, `generate_tuples.md` | Reference material, not user-facing |
| `plans/` (16 files) | Implementation plans | Session artifacts |
| `sessions/` (28 files) | Session logs | Development history |
| `session-notes/` (14 files) | Older session notes | Development history |
| `multi-component/` (3 files) | Multi-component arithmetic | Candidate for syncing |

## Selection criteria

The site includes **user-facing documentation that helps someone use the library**: number system
references, tutorials, build guides, and the mixed-precision SDK docs. Everything internal (plans,
session logs, code reviews, research scratch) is excluded.

## Cleanup opportunity

The `docs/` directory is a mix of published documentation and working notes that accumulated
organically. A future cleanup could move internal artifacts to `docs/internal/` or archive them.
