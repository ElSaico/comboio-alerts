<script lang="ts">
  const OFF_COLOR = '#444444';

  export let ledWidth: number;
  export let ledHeight: number;
  export let color: string;
  export let matrix: Uint8Array[];
</script>

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
