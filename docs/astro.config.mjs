// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

const USERNAME = 'gnikdroy';
const REPO = 'kione';

export default defineConfig({
  site: `https://${USERNAME}.github.io`,
  base: `/${REPO}`,
  integrations: [
    starlight({
      title: 'kione',
      description: 'A C++23 2D game engine with an ImGui editor, EnTT ECS, and Lua scripting.',
      logo: { src: './src/assets/logo.svg', alt: 'kione' },
      favicon: '/favicon.svg',
      customCss: ['./src/styles/custom.css'],
      head: [
        {
          tag: 'link',
          attrs: {
            rel: 'stylesheet',
            href: 'https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined',
          },
        },
      ],
      social: [{ icon: 'github', label: 'GitHub', href: `https://github.com/${USERNAME}/${REPO}` }],
      sidebar: [
        {
          label: 'Guides',
          items: [
            { label: 'Getting Started', slug: 'guides/getting-started' },
            { label: 'Your First Game', slug: 'guides/your-first-game' },
          ],
        },
        {
          label: 'Documentation',
          items: [
            { label: 'Basics', slug: 'documentation/basics' },
            { label: 'Components', slug: 'documentation/components' },
            { label: 'Editor Windows', slug: 'documentation/windows' },
            { label: 'Scripting', slug: 'documentation/scripting' },
            { label: 'Demos & Contributing', slug: 'documentation/contributing' },
          ],
        },
      ],
    }),
  ],
});
