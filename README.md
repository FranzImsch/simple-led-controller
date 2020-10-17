# Simplest LED Controller

Very simple LED controller with a NRF52 as MCU. Controlled via Home Assistant and MySensors using ESB protocol. Basically, it's just a PCB with some MOSFETs, a few resistors and caps as well as a screw terminal.

The main goal was to use as few advanced components as possible. So the power for the MCU is supplied by a LM2596 converter (which is a bit overkill but broadly available). The MOSFETs are IRLZ44N TO-220, even though they are through hole components. The only special thing is the MCU, an E73-2G4M04S1B module from E-Byte.