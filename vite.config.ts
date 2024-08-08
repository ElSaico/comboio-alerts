import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';

export default defineConfig({
  build: {
    assetsInlineLimit: 32768
  },
  plugins: [
    preact(),
  ],
});
