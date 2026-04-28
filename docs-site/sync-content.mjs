#!/usr/bin/env node
/**
 * Syncs ALL content into Starlight's src/content/docs/ tree.
 *
 * The entire src/content/docs/ directory is generated -- no hand-written
 * files live there.  Sources come from three places:
 *
 *   1. docs/           -- plain markdown (FILE_MAP, ROOT_FILE_MAP)
 *                         Gets frontmatter prepended and links rewritten.
 *   2. docs/site/      -- Starlight-native pages (SITE_FILES)
 *                         Already have YAML/MDX frontmatter; copied verbatim.
 *   3. repo root       -- CONTRIBUTORS.md, etc. (ROOT_FILE_MAP)
 *
 * Run automatically via `npm run build` / `npm run dev`.
 */

import { readFileSync, writeFileSync, mkdirSync, existsSync, cpSync, rmSync } from 'fs';
import { dirname, join, posix } from 'path';

const REPO = join(import.meta.dirname, '..');
const DOCS = join(REPO, 'docs');
const SITE = join(DOCS, 'site');       // Starlight-native pages
const OUT = join(import.meta.dirname, 'src', 'content', 'docs');
const BASE = '/universal';  // Astro base path

// ── Source maps ────────────────────────────────────────────────────

/** Map from source path (relative to docs/) -> destination path (relative to content/docs/).
 *  These files get frontmatter prepended and links rewritten. */
const FILE_MAP = {
  // ── Number Systems ──────────────────────────────────────────────
  'number-systems/README.md': 'number-systems/index.md',
  'number-systems/integer.md': 'number-systems/integer.md',
  'number-systems/fixpnt.md': 'number-systems/fixpnt.md',
  'number-systems/dfixpnt.md': 'number-systems/dfixpnt.md',
  'number-systems/rational.md': 'number-systems/rational.md',
  'number-systems/cfloat.md': 'number-systems/cfloat.md',
  'number-systems/bfloat16.md': 'number-systems/bfloat16.md',
  'number-systems/areal.md': 'number-systems/areal.md',
  'number-systems/dfloat.md': 'number-systems/dfloat.md',
  'number-systems/hfloat.md': 'number-systems/hfloat.md',
  'number-systems/microfloat.md': 'number-systems/microfloat.md',
  'number-systems/e8m0.md': 'number-systems/e8m0.md',
  'number-systems/mxfloat.md': 'number-systems/mxfloat.md',
  'number-systems/nvblock.md': 'number-systems/nvblock.md',
  'number-systems/zfpblock.md': 'number-systems/zfpblock.md',
  'number-systems/posit.md': 'number-systems/posit.md',
  'number-systems/posit1.md': 'number-systems/posit1.md',
  'number-systems/posito.md': 'number-systems/posito.md',
  'number-systems/quire.md': 'number-systems/quire.md',
  'number-systems/takum.md': 'number-systems/takum.md',
  'number-systems/lns.md': 'number-systems/lns.md',
  'number-systems/lns-implementation.md': 'number-systems/lns-implementation.md',
  'number-systems/lns-addsub-algorithms.md': 'number-systems/lns-addsub-algorithms.md',
  'number-systems/lns-tolerance-traits.md': 'number-systems/lns-tolerance-traits.md',
  'number-systems/dbns.md': 'number-systems/dbns.md',
  'number-systems/valid.md': 'number-systems/valid.md',
  'number-systems/interval.md': 'number-systems/interval.md',
  'number-systems/sorn.md': 'number-systems/sorn.md',
  'number-systems/unum2.md': 'number-systems/unum2.md',
  'number-systems/dd.md': 'number-systems/dd.md',
  'number-systems/qd.md': 'number-systems/qd.md',
  'number-systems/dd_cascade.md': 'number-systems/dd-cascade.md',
  'number-systems/td_cascade.md': 'number-systems/td-cascade.md',
  'number-systems/qd_cascade.md': 'number-systems/qd-cascade.md',
  'number-systems/complex.md': 'number-systems/complex.md',

  // ── ucalc ──────────────────────────────────────────────────────
  'ucalc/README.md': 'ucalc/index.md',
  'ucalc/examples.md': 'ucalc/examples.md',
  'ucalc/step-by-step.md': 'ucalc/step-by-step.md',
  'ucalc/mcp-server.md': 'ucalc/mcp-server.md',

  // ── Tutorials ───────────────────────────────────────────────────
  'command-line-tools.md': 'getting-started/command-line-tools.md',
  'tutorials/type-parameterization.md': 'tutorials/type-parameterization.md',
  'tutorials/posit-refinement.md': 'tutorials/posit-refinement.md',
  'tutorials/arbitrary-precision.md': 'tutorials/arbitrary-precision.md',
  'tutorials/a-real-with-uncertainty.md': 'tutorials/a-real-with-uncertainty.md',
  'tutorials/multi-component.md': 'tutorials/multi-component.md',
  'tutorials/ucalc-repl.md': 'tutorials/ucalc-repl.md',

  // ── Mixed Precision ────────────────────────────────────────────
  'mixed-precision-methodology.md': 'mixed-precision/methodology.md',
  'mixed-precision-sdk.md': 'mixed-precision/sdk.md',
  'mixed-precision-utilities.md': 'mixed-precision/utilities.md',
  'block-formats.md': 'mixed-precision/block-formats.md',

  // ── Design ─────────────────────────────────────────────────────
  'multi-limb-arithmetic.md': 'design/multi-limb.md',
  'floatcascade-design.md': 'design/floatcascade.md',
  'design/error-propagation-design.md': 'design/error-propagation.md',
  'design/error_tracker.md': 'design/error-tracker.md',
  'design/pop-precision-tuning.md': 'design/pop-precision-tuning.md',
  'decimal_conversion.md': 'design/decimal-conversion.md',
  'ereal_limb_limit_derivation.md': 'design/ereal_limb_limit.md',

  // ── Build & Install ────────────────────────────────────────────
  'cross-compilation.md': 'build/cross-compilation.md',
  'code-formatting.md': 'build/code-formatting.md',
  'troubleshooting.md': 'build/troubleshooting.md',
  'linux-packages.md': 'build/linux-packages.md',

  // ── Exact Arithmetic ──────────────────────────────────────────
  'the-kulisch-super-accumulator.md': 'exact-arithmetic/kulisch-super-accumulator.md',

  // ── Contributing ───────────────────────────────────────────────
  'RELEASE_PROCESS.md': 'contributing/release-process.md',
};

/** Files from the repo root (not docs/) */
const ROOT_FILE_MAP = {
  'PURPOSE.md': 'purpose.md',
  'CONTRIBUTORS.md': 'contributing/contributors.md',
  'CODE-OF-CONDUCT.md': 'contributing/code-of-conduct.md',
  'CHANGELOG.md': 'changelog.md',
};

/** Starlight-native pages from docs/site/.
 *  These already have YAML/MDX frontmatter and are copied verbatim.
 *  Source path is relative to docs/site/, destination relative to content/docs/. */
const SITE_FILES = {
  'index.mdx':                      'index.mdx',
  'getting-started/index.md':       'getting-started/index.md',
  'getting-started/docker.md':      'getting-started/docker.md',
  'getting-started/first-program.md': 'getting-started/first-program.md',
  'getting-started/installation.md': 'getting-started/installation.md',
  'tutorials/index.md':             'tutorials/index.md',
  'build/index.md':                 'build/index.md',
  'contributing/index.md':          'contributing/index.md',
  'design/index.md':                'design/index.md',
  'exact-arithmetic/index.md':      'exact-arithmetic/index.md',
  'resources/citation.md':          'resources/citation.md',
  'resources/presentations.md':     'resources/presentations.md',
};

// ── Link rewriting ─────────────────────────────────────────────────

/**
 * Build a lookup from source .md path (relative to docs/) -> Starlight clean URL slug.
 * e.g. 'number-systems/integer.md' -> '/universal/number-systems/integer/'
 *      'number-systems/dd_cascade.md' -> '/universal/number-systems/dd-cascade/'
 */
function buildLinkLookup() {
  const lookup = {};
  for (const [src, dest] of Object.entries(FILE_MAP)) {
    const slug = dest.replace(/\.mdx?$/, '').replace(/\/index$/, '/');
    lookup[src] = `${BASE}/${slug.endsWith('/') ? slug : slug + '/'}`;
  }
  for (const [src, dest] of Object.entries(ROOT_FILE_MAP)) {
    const slug = dest.replace(/\.mdx?$/, '').replace(/\/index$/, '/');
    lookup[`../${src}`] = `${BASE}/${slug.endsWith('/') ? slug : slug + '/'}`;
  }
  return lookup;
}

const LINK_LOOKUP = buildLinkLookup();

/**
 * Rewrite relative .md links to Starlight clean URLs.
 * @param {string} content - markdown content
 * @param {string} srcRelative - source path relative to docs/ (e.g. 'number-systems/README.md')
 */
function rewriteLinks(content, srcRelative) {
  const srcDir = posix.dirname(srcRelative);
  return content.replace(/\]\(([^)]+\.md)\)/g, (match, target) => {
    if (target.startsWith('http://') || target.startsWith('https://')) return match;
    const resolved = posix.normalize(posix.join(srcDir, target));
    const url = LINK_LOOKUP[resolved];
    if (url) {
      return `](${url})`;
    }
    return match;
  });
}

function extractTitle(content) {
  const match = content.match(/^#\s+(.+)$/m);
  return match ? match[1].trim() : 'Untitled';
}

function stripFirstHeading(content) {
  return content.replace(/^#\s+.+\n*/m, '');
}

function rewriteImagePaths(content) {
  return content
    .replace(/\]\(img\//g, '](/universal/img/')
    .replace(/\]\(closure_plots\//g, '](/universal/img/closure_plots/')
    .replace(/```bib/g, '```text');
}

function addFrontmatter(content, srcRelative, extraFields = {}) {
  const title = extractTitle(content);
  let body = stripFirstHeading(content);
  body = rewriteImagePaths(body);
  body = rewriteLinks(body, srcRelative);
  const extra = Object.entries(extraFields)
    .map(([k, v]) => `${k}: ${v}`)
    .join('\n');
  const safeTitle = title.replace(/\\/g, '\\\\').replace(/"/g, '\\"');
  const fm = extra ? `---\ntitle: "${safeTitle}"\n${extra}\n---` : `---\ntitle: "${safeTitle}"\n---`;
  return `${fm}\n\n${body}`;
}

function syncFile(srcPath, srcRelative, destRelative, extraFields = {}) {
  if (!existsSync(srcPath)) {
    console.warn(`  SKIP (not found): ${srcPath}`);
    return;
  }
  const destPath = join(OUT, destRelative);
  const content = readFileSync(srcPath, 'utf-8');
  mkdirSync(dirname(destPath), { recursive: true });
  writeFileSync(destPath, addFrontmatter(content, srcRelative, extraFields));
}

/** Copy a file verbatim (already has frontmatter). */
function copySiteFile(srcPath, destRelative) {
  if (!existsSync(srcPath)) {
    console.warn(`  SKIP (not found): ${srcPath}`);
    return;
  }
  const destPath = join(OUT, destRelative);
  mkdirSync(dirname(destPath), { recursive: true });
  cpSync(srcPath, destPath);
}

// ── Main ──────────────────────────────────────────────────────────

// Clear stale Astro data store cache
const astroCache = join(import.meta.dirname, 'node_modules', '.astro');
if (existsSync(astroCache)) {
  rmSync(astroCache, { recursive: true });
}

console.log('Syncing docs/ -> docs-site/src/content/docs/ ...');

// Wipe the entire output directory.
// ALL content is regenerated from docs/, docs/site/, and repo root.
if (existsSync(OUT)) {
  rmSync(OUT, { recursive: true });
}
mkdirSync(OUT, { recursive: true });

// 1. Sync docs/ files (add frontmatter, rewrite links)
for (const [src, dest] of Object.entries(FILE_MAP)) {
  syncFile(join(DOCS, src), src, dest);
}

// 2. Sync repo-root files (srcRelative uses ../ prefix for link resolution)
for (const [src, dest] of Object.entries(ROOT_FILE_MAP)) {
  syncFile(join(REPO, src), `../${src}`, dest);
}

// 3. Copy Starlight-native site pages verbatim (already have frontmatter)
for (const [src, dest] of Object.entries(SITE_FILES)) {
  copySiteFile(join(SITE, src), dest);
}

// 4. Copy images to public/ (served as static assets at /universal/img/)
const PUB = join(import.meta.dirname, 'public');
const imgSrc = join(DOCS, 'img');
const imgDest = join(PUB, 'img');
if (existsSync(imgSrc)) {
  mkdirSync(imgDest, { recursive: true });
  cpSync(imgSrc, imgDest, { recursive: true });
  console.log('  Copied docs/img/ -> public/img/');
}

// Copy closure_plots
const plotsSrc = join(DOCS, 'closure_plots');
const plotsDest = join(PUB, 'img', 'closure_plots');
if (existsSync(plotsSrc)) {
  mkdirSync(plotsDest, { recursive: true });
  cpSync(plotsSrc, plotsDest, { recursive: true });
  console.log('  Copied docs/closure_plots/ -> public/img/closure_plots/');
}

// Copy presentations
const presSrc = join(DOCS, 'presentations');
const presDest = join(import.meta.dirname, 'public', 'presentations');
if (existsSync(presSrc)) {
  mkdirSync(presDest, { recursive: true });
  cpSync(presSrc, presDest, { recursive: true });
  console.log('  Copied docs/presentations/ -> public/presentations/');
}

console.log('Done.');
