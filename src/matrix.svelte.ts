import { derived, type Subscriber, type Writable, writable } from 'svelte/store';

class Matrix {
  protected CONTROL_OPCODES: {[key: number]: string} = {
    9: "DECODE MODE", 10: "SET INTENSITY", 11: "SCAN LIMIT", 12: "SHUT DOWN", 15: "DISPLAY TEST"
  };
  protected TOTAL_MODULES = 54;
  // Parola numbers the modules right-to-left, but due to chain propagation data is sent left-to-right,
  // hence why indexing differs from the firmware
  protected ZONES = [ 54, 30, 20, 10, 0 ];
  protected display = 0;
  protected opcode: number | null = null;
  protected store: Writable<Uint8Array[]>;
  public debug = $state('');

  constructor() {
    this.store = writable(Array.from({ length: this.TOTAL_MODULES }, () => new Uint8Array(8)));
  }

  public subscribe(run: Subscriber<Uint8Array[]>, invalidate?: () => void) {
    return this.store.subscribe(run, invalidate);
  }

  public set(value: number) {
    if (this.opcode === null) {
      this.opcode = value;
    } else {
      if (this.opcode <= 8) {
        this.store.update(modules => {
          modules[this.display][this.opcode!-1] = value;
          return modules;
        });
        this.debug += `mod=${this.display.toString().padEnd(2)} row=${this.opcode} val=${value}\n`;
      } else if (this.display === 0) { // control commands are equal across all modules
        this.debug += `control ${this.CONTROL_OPCODES[this.opcode]} set to ${value}\n`;
      }
      this.display = (this.display + 1) % this.TOTAL_MODULES;
      this.opcode = null;
    }
  }

  public zone(n: number) {
    return derived(this.store, $store => $store.slice(this.ZONES[n+1], this.ZONES[n]));
  }
}

export default new Matrix();