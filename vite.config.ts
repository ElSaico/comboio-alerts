import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';

export default defineConfig({
  build: {
    assetsInlineLimit: 32768
  },
  plugins: [
    svelte(),
  ],
});
