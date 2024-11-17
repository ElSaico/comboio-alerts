import fs from 'node:fs';
import path from 'node:path';
import MemoryMap from 'nrf-intel-hex';

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
    board: 'uno',
    files: [
      { name: 'fonts.hpp', content: fonts },
      { name: 'libraries.txt', content: 'MD_Parola' }
    ],
    options: { ramSize: 32768 }
  })
}).then(async response => {
  const result: IHexiResult = await response.json();
  if (!result.hex) {
    console.log(result.stdout);
    console.error(result.stderr);
  } else {
    const memMap = MemoryMap.fromHex(result.hex);
    fs.writeFileSync(path.resolve(import.meta.dirname, '../public/firmware.bin'), memMap.get(0)!);
  }
});
