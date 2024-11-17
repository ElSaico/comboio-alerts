<script lang="ts">
  import { ConsoleLogger, LogLevel, Simulator } from 'rp2040js';

  import { bootromB1 } from './bootrom';
  import { loadHex } from './intelhex';
  import matrix from './matrix.svelte';
  import LedMatrix from './LedMatrix.svelte';
  import hex from '../public/firmware.hex?raw';

  const BASE_ADDRESS = 0x10000000;
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
  const runner = new Simulator();
  let serial: string = $state('');
  let serialEl: HTMLPreElement = $state()!;

  runner.rp2040.loadBootrom(bootromB1);
  runner.rp2040.logger = new ConsoleLogger(LogLevel.Debug);
  loadHex(hex, runner.rp2040.flash, BASE_ADDRESS);

  runner.rp2040.spi[0].onTransmit = value => {
    matrix.set(value);
    runner.rp2040.spi[0].completeTransmit(value);
  };

  runner.rp2040.uart[0].onByte = value => {
    serial += value;
  };

  runner.rp2040.core.PC = BASE_ADDRESS;
  runner.execute();

  function sendCommands(commands: string[]) {
    for (const command of commands) {
      serial += `>> ${command}\n`;
      //runner.serialWrite(`${command}\n`);
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