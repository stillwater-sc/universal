#!/usr/bin/env node
/**
 * Syncs markdown files from the repo's docs/ directory into Starlight's
 * src/content/docs/ tree, prepending YAML frontmatter extracted from the
 * first H1 heading of each file.
 *
 * Run automatically via `npm run build` / `npm run dev`.
 */

import { readFileSync, writeFileSync, mkdirSync, existsSync, cpSync, rmSync } from 'fs';
import { dirname, join, posix } from 'path';

const REPO = join(import.meta.dirname, '..');
const DOCS = join(REPO, 'docs');
const OUT = join(import.meta.dirname, 'src', 'content', 'docs');
const BASE = '/universal';  // Astro base path

/** Map from source path (relative to docs/) → destination path (relative to content/docs/). */
const FILE_MAP = {
  // ── Number Systems ──────────────────────────────────────────────
  'number-systems/README.md': 'number-systems/index.md',
  'number-systems/integer.md': 'number-systems/integer.md',
  'number-systems/fixpnt.md': 'number-systems/fixpnt.md',
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

  // ── Tutorials ───────────────────────────────────────────────────
  'command-line-tools.md': 'getting-started/command-line-tools.md',
  'number-system-type-parameterization.md': 'tutorials/type-parameterization.md',
  'posit-refinement-viz.md': 'tutorials/posit-refinement.md',
  'arbitrary-precision-design.md': 'tutorials/arbitrary-precision.md',
  'multi-component-arithmetic.md': 'tutorials/multi-component.md',

  // ── Mixed Precision ────────────────────────────────────────────
  'mixed-precision-methodology.md': 'mixed-precision/methodology.md',
  'mixed-precision-sdk.md': 'mixed-precision/sdk.md',
  'mixed-precision-paper-findings.md': 'mixed-precision/findings.md',
  'mixed-precision-utilities.md': 'mixed-precision/utilities.md',
  'block-formats.md': 'mixed-precision/block-formats.md',

  // ── Design ─────────────────────────────────────────────────────
  'floatcascade-design.md': 'design/floatcascade.md',
  'design/error-propagation-design.md': 'design/error-propagation.md',
  'design/error_tracker.md': 'design/error-tracker.md',
  'multi-limb-arithmetic.md': 'design/multi-limb.md',
  'decimal_conversion.md': 'design/decimal-conversion.md',

  // ── Build & Install ────────────────────────────────────────────
  'cross-compilation.md': 'build/cross-compilation.md',
  'code-formatting.md': 'build/code-formatting.md',
  'troubleshooting.md': 'build/troubleshooting.md',
  'linux-packages.md': 'build/linux-packages.md',

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

/**
 * Build a lookup from source .md path (relative to docs/) → Starlight clean URL slug.
 * e.g. 'number-systems/integer.md' → '/universal/number-systems/integer/'
 *      'number-systems/dd_cascade.md' → '/universal/number-systems/dd-cascade/'
 */
function buildLinkLookup() {
  const lookup = {};
  for (const [src, dest] of Object.entries(FILE_MAP)) {
    // dest is like 'number-systems/integer.md' → slug 'number-systems/integer'
    const slug = dest.replace(/\.md$/, '').replace(/\/index$/, '/');
    lookup[src] = `${BASE}/${slug.endsWith('/') ? slug : slug + '/'}`;
  }
  for (const [src, dest] of Object.entries(ROOT_FILE_MAP)) {
    const slug = dest.replace(/\.md$/, '').replace(/\/index$/, '/');
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
  // Match markdown links like [text](target.md) or [text](../target.md)
  // but not external URLs (http://, https://)
  return content.replace(/\]\(([^)]+\.md)\)/g, (match, target) => {
    // Skip absolute URLs
    if (target.startsWith('http://') || target.startsWith('https://')) return match;
    // Resolve relative path against source directory
    const resolved = posix.normalize(posix.join(srcDir, target));
    const url = LINK_LOOKUP[resolved];
    if (url) {
      return `](${url})`;
    }
    // Not in our file map — leave unchanged
    return match;
  });
}

function extractTitle(content) {
  const match = content.match(/^#\s+(.+)$/m);
  return match ? match[1].trim() : 'Untitled';
}

function stripFirstHeading(content) {
  // Remove the first H1 heading line (Starlight renders the frontmatter title)
  return content.replace(/^#\s+.+\n*/m, '');
}

function rewriteImagePaths(content) {
  // Rewrite relative image paths to absolute /universal/ paths (served from public/)
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
  const fm = extra ? `---\ntitle: "${title.replace(/"/g, '\\"')}"\n${extra}\n---` : `---\ntitle: "${title.replace(/"/g, '\\"')}"\n---`;
  return `${fm}\n\n${body}`;
}

function syncFile(srcPath, srcRelative, destRelative, extraFields = {}) {
  if (!existsSync(srcPath)) {
    console.warn(`  SKIP (not found): ${srcPath}`);
    return;
  }
  const destPath = join(OUT, destRelative);
  // Skip if a hand-written page already exists (avoid duplicates)
  if (existsSync(destPath)) {
    return;
  }
  const content = readFileSync(srcPath, 'utf-8');
  mkdirSync(dirname(destPath), { recursive: true });
  writeFileSync(destPath, addFrontmatter(content, srcRelative, extraFields));
}

// ── Main ──────────────────────────────────────────────────────────

// Clear stale Astro data store cache to ensure new/changed files are picked up
const astroCache = join(import.meta.dirname, 'node_modules', '.astro');
if (existsSync(astroCache)) {
  rmSync(astroCache, { recursive: true });
}

console.log('Syncing docs/ → docs-site/src/content/docs/ ...');

// Sync docs/ files
for (const [src, dest] of Object.entries(FILE_MAP)) {
  syncFile(join(DOCS, src), src, dest);
}

// Sync repo-root files (srcRelative uses ../ prefix for link resolution)
for (const [src, dest] of Object.entries(ROOT_FILE_MAP)) {
  syncFile(join(REPO, src), `../${src}`, dest);
}

// Copy images to public/ (served as static assets at /universal/img/)
const PUB = join(import.meta.dirname, 'public');
const imgSrc = join(DOCS, 'img');
const imgDest = join(PUB, 'img');
if (existsSync(imgSrc)) {
  mkdirSync(imgDest, { recursive: true });
  cpSync(imgSrc, imgDest, { recursive: true });
  console.log('  Copied docs/img/ → public/img/');
}

// Copy closure_plots
const plotsSrc = join(DOCS, 'closure_plots');
const plotsDest = join(PUB, 'img', 'closure_plots');
if (existsSync(plotsSrc)) {
  mkdirSync(plotsDest, { recursive: true });
  cpSync(plotsSrc, plotsDest, { recursive: true });
  console.log('  Copied docs/closure_plots/ → public/img/closure_plots/');
}

// Copy presentations
const presSrc = join(DOCS, 'presentations');
const presDest = join(import.meta.dirname, 'public', 'presentations');
if (existsSync(presSrc)) {
  mkdirSync(presDest, { recursive: true });
  cpSync(presSrc, presDest, { recursive: true });
  console.log('  Copied docs/presentations/ → public/presentations/');
}

console.log('Done.');
