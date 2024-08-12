/**
 * AVRRunner
 * Part of AVR8js
 *
 * Copyright (C) 2019, Uri Shaked
 */
import {
  avrInstruction,
  avrInterrupt,
  AVRTimer,
  CPU,
  AVRIOPort,
  AVRUSART,
  AVRSPI,
  AVRTWI,
  portBConfig,
  portCConfig,
  portDConfig,
  timer0Config,
  timer1Config,
  timer2Config,
  usart0Config,
  spiConfig,
  twiConfig,
} from 'avr8js';

import { MicroTaskScheduler } from './task-scheduler';

export default class AVRRunner {
  readonly program: Uint16Array;
  readonly cpu: CPU;
  readonly timer0: AVRTimer;
  readonly timer1: AVRTimer;
  readonly timer2: AVRTimer;
  readonly portB: AVRIOPort;
  readonly portC: AVRIOPort;
  readonly portD: AVRIOPort;
  readonly usart: AVRUSART;
  readonly spi: AVRSPI;
  readonly twi: AVRTWI;
  readonly frequency = 16e6; // 16 MHz
  readonly taskScheduler = new MicroTaskScheduler();

  private serialBuffer: Array<number> = [];
  private stopped = false;

  constructor(program: Uint16Array) {
    this.program = program;
    this.cpu = new CPU(this.program);

    this.timer0 = new AVRTimer(this.cpu, timer0Config);
    this.timer1 = new AVRTimer(this.cpu, timer1Config);
    this.timer2 = new AVRTimer(this.cpu, timer2Config);

    this.portB = new AVRIOPort(this.cpu, portBConfig);
    this.portC = new AVRIOPort(this.cpu, portCConfig);
    this.portD = new AVRIOPort(this.cpu, portDConfig);

    this.usart = new AVRUSART(this.cpu, usart0Config, this.frequency);
    this.spi = new AVRSPI(this.cpu, spiConfig, this.frequency);
    this.twi = new AVRTWI(this.cpu, twiConfig, this.frequency);

    // this.serialOnLineTransmit();
    this.cpu.readHooks[usart0Config.UDR] = () => this.serialBuffer.shift() || 0;

    this.taskScheduler.start();
  }

  // Function to send data to the serial port
  serialWrite(value: string) {
    // Writing to UDR transmits the byte
    [...value].forEach(c => {
      // Write a character
      this.serialBuffer.push(c.charCodeAt(0));
    });
  }

  serialOnLineTransmit() {
    // Serial port to browser console
    this.usart.onLineTransmit = (line) => {
      console.log("[Serial] %c%s", "color: red", line);
    };
  }

  rxCompleteInterrupt() {
    const UCSRA = this.cpu.data[usart0Config.UCSRA];

    if ((UCSRA & 0x20) && (this.serialBuffer.length > 0)) {
      avrInterrupt(this.cpu, usart0Config.rxCompleteInterrupt);
    }
  }

  // CPU main loop
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

  analogPort() {
    // Simulate analog port (so that analogRead() eventually return)
    this.cpu.writeHooks[0x7a] = value => {
      if (value & (1 << 6)) {
        // random value
        const analogValue = Math.floor(Math.random() * 1024);

        this.cpu.data[0x7a] = value & ~(1 << 6); // Clear bit - conversion done
        this.cpu.data[0x78] = analogValue & 0xff;
        this.cpu.data[0x79] = (analogValue >> 8) & 0x3;

        return true; // Don't update
      }
    };
  }
}
