import fs from 'node:fs';
import path from 'node:path';

export interface IHexiResult {
  stdout: string;
  stderr: string;
  hex: string;
}

const BUILD_URL = 'https://hexi.wokwi.com/build';
const firmware = fs.readFileSync(path.resolve(import.meta.dirname, '../firmware/firmware.ino'), { encoding: 'latin1' });
const fonts = fs.readFileSync(path.resolve(import.meta.dirname, '../firmware/fonts.hpp'), { encoding: 'latin1' });

fetch(BUILD_URL, {
  method: 'POST',
  mode: 'cors',
  cache: 'no-cache',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    sketch: firmware,
    board: 'wokwi-pi-pico',
    files: [
      { name: 'fonts.hpp', content: fonts },
      { name: 'libraries.txt', content: 'MD_Parola' }
    ]
  })
}).then(async response => {
  const result: IHexiResult = await response.json();
  if (!result.hex) {
    console.log(result.stdout);
    console.error(result.stderr);
  } else {
    fs.writeFileSync(path.resolve(import.meta.dirname, '../public/firmware.hex'), result.hex);
  }
});
