<script lang="ts">
  import AVRRunner from './execute';
  import matrix from './matrix';
  import LedMatrix from './lib/LedMatrix.svelte';
  import firmwareUrl from '../build/firmware.ino.with_bootloader.bin?url';

  fetch(firmwareUrl).then(async response => {
    const firmware = await response.arrayBuffer();
    const runner = new AVRRunner(new Uint16Array(firmware));

    runner.spi.onByte = value => {
      matrix.set(value);
      runner.spi.completeTransfer(value);
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
</main>
