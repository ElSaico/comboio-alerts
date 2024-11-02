import {
  avrInstruction,
  CPU,
  AVRTimer,
  AVRUSART,
  AVRSPI,
  type AVRTimerConfig,
  type USARTConfig,
  type SPIConfig,
} from 'avr8js';

const timer0Config: AVRTimerConfig = {
  bits: 8,
  dividers: { 0: 0, 1: 1, 2: 8, 3: 64, 4: 256, 5: 1024, 6: 0, 7: 0 },

  captureInterrupt: 0,
  compAInterrupt: 0x2a,
  compBInterrupt: 0x2c,
  compCInterrupt: 0,
  ovfInterrupt: 0x2e,

  TCCRA: 0x44,
  TCCRB: 0x45,
  TCCRC: 0,
  TCNT: 0x46,
  OCRA: 0x47,
  OCRB: 0x48,
  OCRC: 0,
  ICR: 0,
  TIMSK: 0x6e,
  // TIMSK bits
  TOIE: 1,
  OCIEA: 2,
  OCIEB: 4,
  OCIEC: 0,
  TIFR: 0x35,
  // TIFR bits
  TOV: 1,
  OCFA: 2,
  OCFB: 4,
  OCFC: 0,

  // OC0{A,B,C}
  compPortA: 0x25, // PORTB
  compPinA: 7,
  compPortB: 0x34, // PORTG
  compPinB: 5,
  compPortC: 0,
  compPinC: 0,
  // T0
  externalClockPort: 0x2b, // PORTD
  externalClockPin: 7,
};

const usart0Config: USARTConfig = {
  rxCompleteInterrupt: 0x32,
  dataRegisterEmptyInterrupt: 0x34,
  txCompleteInterrupt: 0x36,

  UCSRA: 0xc0,
  UCSRB: 0xc1,
  UCSRC: 0xc2,
  UBRRL: 0xc4,
  UBRRH: 0xc5,
  UDR: 0xc6,
}

const spiConfig: SPIConfig = {
  spiInterrupt: 0x30,
  SPCR: 0x4c,
  SPSR: 0x4d,
  SPDR: 0x4e,
};

export default class AVRRunner {
  readonly cpu: CPU;
  readonly timer: AVRTimer;
  readonly usart: AVRUSART;
  readonly spi: AVRSPI;
  readonly frequency = 16e6; // 16 MHz

  private serialBuffer: Array<number> = [];
  private stopped = false;

  constructor(program: Uint16Array) {
    this.cpu = new CPU(program, 0x2000);
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
