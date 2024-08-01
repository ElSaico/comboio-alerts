import { AVRIOPort, AVRTimer, CPU } from "avr8js";
import { useState, useEffect } from 'preact/hooks';
import './app.css';

export function App() {
  const [ firmware, setFirmware ] = useState<Uint16Array>();
  useEffect(() => {
    const cpu = new CPU(firmware);
  }, [firmware]);

  fetch('/firmware.elf')
    .then(async response => setFirmware(new Uint16Array(await response.arrayBuffer())));

  return (
    <>
      <h1>Vite + Preact</h1>
      <div class="card">
        <p>
          Edit <code>src/app.tsx</code> and save to test HMR
        </p>
      </div>
      <p>
        Check out{' '}
        <a
          href="https://preactjs.com/guide/v10/getting-started#create-a-vite-powered-preact-app"
          target="_blank"
        >
          create-preact
        </a>
        , the official Preact + Vite starter
      </p>
      <p class="read-the-docs">
        Click on the Vite and Preact logos to learn more
      </p>
    </>
  )
}
