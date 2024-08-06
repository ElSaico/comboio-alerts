import { render } from 'preact';
import { useEffect } from 'preact/hooks';

import AVRRunner from './execute';
import buildHex from './compile';
import './app.css';

function App() {
  useEffect(() => {
    fetch('/main.cpp')
      .then(async response => {
        const source = await response.text();
        const result = await buildHex(source);
        const runner = new AVRRunner(result.hex);

        runner.spi.onByte = value => {
          console.log(value);
          runner.spi.completeTransfer(value);
        };
        runner.execute(cpu => {
        });
      });
  }, []);

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

render(<App />, document.getElementById('app')!);
