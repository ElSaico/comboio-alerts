import { derived, type Invalidator, type Subscriber, type Writable, writable } from 'svelte/store';

class Matrix {
  protected TOTAL_MODULES = 54;
  protected ZONES = [ [0, 24], [24, 34], [34, 44], [44, 54] ];
  protected display = 0;
  protected opcode: number | null = null;
  protected store: Writable<Uint8Array[]>;

  constructor() {
    this.store = writable(Array.from({ length: this.TOTAL_MODULES }, () => new Uint8Array(8)));
  }

  public subscribe(run: Subscriber<Uint8Array[]>, invalidate?: Invalidator<Uint8Array[]>) {
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
      } else {
        console.log('unhandled opcode: %s %s', this.opcode, value);
      }
      this.display = (this.display + 1) % this.TOTAL_MODULES;
      this.opcode = null;
    }
  }

  public zone(n: number) {
    return derived(this.store, $store => $store.slice(...this.ZONES[n]));
  }
}

export default new Matrix();