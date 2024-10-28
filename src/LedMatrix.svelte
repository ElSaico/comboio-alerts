<script lang="ts">
  import type { Readable } from "svelte/store";

  const OFF_COLOR = '#444444';

  interface Props {
    ledWidth: number;
    ledHeight: number;
    color: string;
    modules: Readable<Uint8Array[]>;
  }
  let { ledWidth, ledHeight, color, modules }: Props = $props();
</script>

{#each $modules as display}
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
