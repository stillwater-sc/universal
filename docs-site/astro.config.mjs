import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  site: 'https://stillwater-sc.github.io',
  base: '/universal',
  integrations: [
    starlight({
      title: 'Universal Numbers Library',
      description:
        'A header-only C++ template library for custom arithmetic plug-in types',
      social: [
        {
          icon: 'github',
          label: 'GitHub',
          href: 'https://github.com/stillwater-sc/universal',
        },
      ],
      editLink: {
        baseUrl:
          'https://github.com/stillwater-sc/universal/edit/main/docs-site/',
      },
      customCss: ['./src/styles/custom.css'],
      sidebar: [
        {
          label: 'Getting Started',
          autogenerate: { directory: 'getting-started' },
        },
        {
          label: 'Number Systems',
          items: [
            {
              label: 'Guide',
              link: '/number-systems/',
            },
            {
              label: 'Integer & Fixed-Point',
              collapsed: true,
              items: [
                { label: 'integer', link: '/number-systems/integer/' },
                { label: 'fixpnt', link: '/number-systems/fixpnt/' },
                { label: 'rational', link: '/number-systems/rational/' },
              ],
            },
            {
              label: 'Configurable Floating-Point',
              collapsed: true,
              items: [
                { label: 'cfloat', link: '/number-systems/cfloat/' },
                { label: 'bfloat16', link: '/number-systems/bfloat16/' },
                { label: 'areal', link: '/number-systems/areal/' },
                { label: 'dfloat', link: '/number-systems/dfloat/' },
                { label: 'hfloat', link: '/number-systems/hfloat/' },
              ],
            },
            {
              label: 'Micro-Precision & Block-Scaled',
              collapsed: true,
              items: [
                { label: 'microfloat', link: '/number-systems/microfloat/' },
                { label: 'e8m0', link: '/number-systems/e8m0/' },
                { label: 'mxfloat', link: '/number-systems/mxfloat/' },
                { label: 'nvblock', link: '/number-systems/nvblock/' },
                { label: 'zfpblock', link: '/number-systems/zfpblock/' },
              ],
            },
            {
              label: 'Posit Family',
              collapsed: true,
              items: [
                { label: 'posit', link: '/number-systems/posit/' },
                { label: 'posit1 (legacy)', link: '/number-systems/posit1/' },
                { label: 'posito', link: '/number-systems/posito/' },
                { label: 'quire', link: '/number-systems/quire/' },
                { label: 'takum', link: '/number-systems/takum/' },
              ],
            },
            {
              label: 'Logarithmic',
              collapsed: true,
              items: [
                { label: 'lns', link: '/number-systems/lns/' },
                { label: 'dbns', link: '/number-systems/dbns/' },
              ],
            },
            {
              label: 'Interval & Uncertainty',
              collapsed: true,
              items: [
                { label: 'valid', link: '/number-systems/valid/' },
                { label: 'interval', link: '/number-systems/interval/' },
                { label: 'sorn', link: '/number-systems/sorn/' },
                { label: 'unum2', link: '/number-systems/unum2/' },
              ],
            },
            {
              label: 'Extended Precision',
              collapsed: true,
              items: [
                { label: 'dd', link: '/number-systems/dd/' },
                { label: 'qd', link: '/number-systems/qd/' },
                { label: 'dd_cascade', link: '/number-systems/dd-cascade/' },
                { label: 'td_cascade', link: '/number-systems/td-cascade/' },
                { label: 'qd_cascade', link: '/number-systems/qd-cascade/' },
              ],
            },
            { label: 'Complex', link: '/number-systems/complex/' },
          ],
        },
        {
          label: 'Tutorials',
          autogenerate: { directory: 'tutorials' },
        },
        {
          label: 'Mixed Precision',
          autogenerate: { directory: 'mixed-precision' },
        },
        {
          label: 'Design',
          autogenerate: { directory: 'design' },
        },
        {
          label: 'Build & Install',
          autogenerate: { directory: 'build' },
        },
        {
          label: 'Contributing',
          autogenerate: { directory: 'contributing' },
        },
        {
          label: 'Resources',
          autogenerate: { directory: 'resources' },
        },
      ],
    }),
  ],
});
