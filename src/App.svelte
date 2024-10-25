<script lang="ts">
  import AVRRunner from './execute';
  import matrix from './matrix';
  import LedMatrix from './lib/LedMatrix.svelte';
  import firmwareUrl from '../build/firmware.ino.with_bootloader.bin?url';

  const messages = [
    ['Alert: Follow', 'AF0\nTestFollow\n'],
    ['Alert: Sub', 'AS0\nTestSub\nNew sub message\n'],
    ['Alert: Resub', 'As12\nTestResub\nResub message\n'],
    ['Alert: Gift', 'AG10\nTestGiftSub\n'],
    ['Alert: Cheer', 'AC1000\nTestCheer\nCheer message\n'],
    ['Alert: Raid', 'AR10000\nTestRaid\nRaid Test\n'],
    ['Label: Follow', 'LF0\nTestFollow\n'],
    ['Label: Sub', 'LS0\nTestSub\n'],
    ['Label: Resub', 'Ls24\nTestResub\n'],
    ['Label: Cheer', 'LC1000\nTestCheer\n'],
  ];
  let runner: AVRRunner;

  fetch(firmwareUrl).then(async response => {
    const firmware = await response.arrayBuffer();
    runner = new AVRRunner(new Uint16Array(firmware));

    runner.spi.onByte = value => {
      matrix.set(value);
      runner.spi.completeTransfer(value);
    };

    runner.usart.onLineTransmit = line => {
      console.log(line);
    };

    runner.execute(() => {});

    return () => {
      runner.stop();
    };
  });
</script>

<main>
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(0)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(1)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(2)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(3)} />
  <div>
    <label>
      <input type="checkbox" name="debug" checked={matrix.debug} on:change={() => { matrix.debug = !matrix.debug; }} />
      Debug
    </label>
  {#each messages as message}
    <button type="button" on:click={() => { runner.serialWrite(message[1]); }}>{message[0]}</button>
  {/each}
  </div>
</main>
