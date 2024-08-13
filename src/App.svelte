<script lang="ts">
  import AVRRunner from './execute';
  import LedMatrix from './lib/LedMatrix.svelte';
  import firmwareUrl from '../build/firmware.ino.with_bootloader.bin?url';

  const ZONES = [
    { start:  0, end: 23 },
    { start: 24, end: 33 },
    { start: 34, end: 43 },
    { start: 44, end: 53 },
  ];
  const TOTAL_MODULES = 54;

  const matrix: Uint8Array[] = new Array(TOTAL_MODULES);
  for (let i = 0; i < matrix.length; i++) {
    matrix[i] = new Uint8Array(8);
  }

  fetch(firmwareUrl).then(async response => {
    const firmware = await response.arrayBuffer();
    const runner = new AVRRunner(new Uint16Array(firmware));
    let display = 0;
    let opcode: number | null = null;

    runner.spi.onByte = value => {
      if (opcode === null) {
        opcode = value;
      } else {
        if (opcode <= 8) {
          matrix[display][opcode-1] = value;
        } else {
          console.log('unhandled opcode: %s %s', opcode, value);
        }
        display = (display + 1) % TOTAL_MODULES;
        opcode = null;
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
