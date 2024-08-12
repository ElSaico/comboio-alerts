<script lang="ts">
  import AVRRunner from './execute';
  import LedMatrix from './lib/LedMatrix.svelte';
  import firmwareUrl from '../build/firmware.ino.with_bootloader.bin?url';

  const CHAIN_LENGTH = 10;

  let matrix: Uint8Array[] = [];

  fetch(firmwareUrl).then(async response => {
    const firmware = await response.arrayBuffer();
    const runner = new AVRRunner(new Uint16Array(firmware));
    let display = 0;
    let segment: number | null = null;

    for (let i = 0; i < CHAIN_LENGTH; i++) {
      matrix.push(new Uint8Array(8));
    }

    runner.spi.onByte = value => {
      if (segment === null) {
        segment = value;
      } else {
        if (segment <= 8) {
          matrix[display][segment-1] = value;
        } else {
          console.log('out of bounds: %s %s', segment, value);
        }
        display = (display + 1) % CHAIN_LENGTH;
        segment = null;
      }
      runner.spi.completeTransfer(value);
    };

    runner.execute(() => {});

    return () => {
      runner.stop();
    };
  });
</script>

<main>
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} matrix={matrix} />
</main>
