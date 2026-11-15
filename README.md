# ehz

ehz comes from "elektronischer Haushaltszähler". It is an efficient C++ based application for controlling a solar generator.

## Solar Generator

It consists of an ac input of solar panels (not controlled, yet) and an AC battery (Marstek Venus E v3). Two power meters (B+G E-Tech WS100-1943) are monitoring the solar panels and the whole generator.

## Control Measurement

A standard german power meter, capable of outputting data in the smart meter language (SML), is used.

## Features

Acquisition of data and control of the battery. A Marstek Venus E v3 specific balancing algorithm is included.

The main goal, of course, is to have (almost) zero bugs. :) At least the application shall run without user interaction.

## Notes

Although this application is made for a very specific setup, its purpose is to be changeable.

## Disclaimer

It's a private repository. Hopefully someone might find it helpful. Don't hesitate to ask questions. But do not expect me to change any behavior, clone the repository.