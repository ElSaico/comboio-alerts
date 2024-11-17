<script lang="ts">
  import AVRRunner from './execute';
  import matrix from './matrix.svelte';
  import LedMatrix from './LedMatrix.svelte';

  const messages: [string, string[]][] = [
    ['Alert: Follow', ['AF0', 'TestFollow']],
    ['Alert: Sub', ['AS0', 'TestSub', 'Nèw súb mésságê']],
    ['Alert: Resub', ['As12', 'TestResub', 'Rèsúb mésságê']],
    ['Alert: Gift', ['AG10', 'TestGiftSub']],
    ['Alert: Cheer', ['AC1000', 'TestCheer', 'Chêér mèsságë']],
    ['Alert: Raid', ['AR10000', 'TestRaid', 'Rãíd Tèst']],
    ['Label: Follow', ['LF0', 'TestFollow']],
    ['Label: Sub', ['LS0', 'TestSub']],
    ['Label: Resub', ['Ls24', 'TestResub']],
    ['Label: Cheer', ['LC1000', 'TestCheer']],
  ];
  let runner: AVRRunner;
  let serial: string = $state('');
  let serialEl: HTMLPreElement = $state()!;

  fetch('./firmware.bin').then(async response => {
    const firmware = await response.arrayBuffer();
    runner = new AVRRunner(new Uint16Array(firmware));

    runner.spi.onByte = value => {
      matrix.set(value);
      runner.spi.completeTransfer(value);
    };

    runner.usart.onLineTransmit = line => {
      serial += `${line}\n`;
    };

    runner.execute(() => {});

    return () => {
      runner.stop();
    };
  });

  function sendCommands(commands: string[]) {
    for (const command of commands) {
      serial += `>> ${command}\n`;
      runner.serialWrite(`${command}\n`);
    }
  }

  $effect(() => {
    if (serial) {
      serialEl.scroll({ top: serialEl.scrollHeight, behavior: 'smooth' });
    }
  });
</script>

<main>
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(0)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(1)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(2)} />
  <LedMatrix color="#ff9900" ledWidth={4} ledHeight={5} modules={matrix.zone(3)} />
  <div>
  {#each messages as [label, commands]}
    <button type="button" onclick={() => sendCommands(commands)}>{label}</button>
  {/each}
  </div>
  <div class="outputs">
    <pre bind:this={serialEl}>{serial}</pre>  
  </div>
</main>

<style>
  .outputs {
    width: 100%;
    height: calc(100vh - 120px);
    column-gap: 16px;
  }

  .outputs pre {
    padding: 8px;
    height: 100%;
    overflow-y: scroll;
    color: green;
    background-color: black;
  }
</style>