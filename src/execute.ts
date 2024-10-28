import {
  avrInstruction,
  CPU,
  AVRTimer,
  AVRUSART,
  AVRSPI,
  timer0Config,
  usart0Config,
  spiConfig,
} from 'avr8js';

export default class AVRRunner {
  readonly cpu: CPU;
  readonly timer: AVRTimer;
  readonly usart: AVRUSART;
  readonly spi: AVRSPI;
  readonly frequency = 16e6; // 16 MHz

  private serialBuffer: Array<number> = [];
  private stopped = false;

  constructor(program: Uint16Array) {
    this.cpu = new CPU(program);
    this.timer = new AVRTimer(this.cpu, timer0Config);
    this.usart = new AVRUSART(this.cpu, usart0Config, this.frequency);
    this.spi = new AVRSPI(this.cpu, spiConfig, this.frequency);

    this.usart.onRxComplete = () => {
      if (this.serialBuffer.length > 0) {
        this.usart.writeByte(this.serialBuffer.shift()!);
      }
    };
  }

  serialWrite(value: string) {
    for (let c of value) {
      this.serialBuffer.push(c.charCodeAt(0));
    };
    if (!this.usart.rxBusy) {
      this.usart.onRxComplete!();
    }
  }

  execute(callback: (cpu: CPU) => void) {
    if (this.stopped) {
      return;
    }

    const { cpu } = this;
    const deadline = cpu.cycles + this.frequency / 60;

    while (cpu.cycles <= deadline) {
      avrInstruction(cpu);
      cpu.tick();
    }

    callback(this.cpu);
    requestAnimationFrame(() => this.execute(callback));
  }

  stop() {
    this.stopped = true;
  }
}
