<script lang="ts">
  import AVRRunner from './execute';
  import firmwareUrl from '../build/firmware.ino.with_bootloader.bin?url';

  const CHAIN_LENGTH = 10;
  const OFF_COLOR = '#444444';
  const color = '#ff9900';
  const ledWidth = 4;
  const ledHeight = 5;

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
  {#each matrix as display}
    <svg xmlns="http://www.w3.org/2000/svg" width={ledWidth * 8} height={ledHeight * 8}>
      {#each display as row, y}
        {#each Array(8) as _, x}
          <ellipse fill={(row >> x) & 1 ? color : OFF_COLOR}
                   rx={ledWidth / 2} ry={ledHeight / 2}
                   cx={ledWidth * (x + 0.5)} cy={ledHeight * (y + 0.5)}/>
        {/each}
      {/each}
    </svg>
  {/each}
</main>
