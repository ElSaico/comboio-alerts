import { defineConfig } from 'vite';
import copy from 'rollup-plugin-copy';
import preact from '@preact/preset-vite';

export default defineConfig({
  plugins: [
    preact(),
    copy({
      targets: [
        { src: '../.pio/build/uno/firmware.elf', dest: 'public' },
      ]
    }),
  ],
});
