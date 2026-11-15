# ehz

ehz comes from "elektronischer Haushaltszähler". It is an efficient C++ based application for controlling a solar generator.

## Solar Generator

It consists of an AC input of solar panels (not controlled, yet) and an AC battery (Marstek Venus E v3). Two power meters (B+G E-Tech WS100-1943) are monitoring the solar panels and the whole generator.

## Control Measurement

A standard german power meter, capable of outputting data in the smart meter language (SML), is used.

## Interface

A lightweight SML parser is included. It interfaces via an IR adapter with the power meter. The IR adapter is connected via UART with an USB-UART converter to the host system.

Other power meters and the battery are connected via RS485 to an USB-RS485 converter.

In all cases the software uses standard Linux drivers (read,write,ioctl). FTDI USB adapters are recommended.

## Features

Acquisition of data and control of the battery. A Marstek Venus E v3 specific balancing algorithm is included.

The main goal, of course, is to have (almost) zero bugs. :) At least the application shall run without user interaction.

## Notes

Although this application is made for a very specific setup, its purpose is to be changeable.

## Disclaimer

It's a private repository. Hopefully someone might find it helpful. Don't hesitate to ask questions. But do not expect me to change any behavior, clone the repository.