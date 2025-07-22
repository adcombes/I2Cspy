# I2Cspy
an I2c bus spy built with an arduino UNO R3

work is in progress ...

# Summary  
This is an I2C bus spy made with an ARDUINO UNO R3 without any additional electronic hardware.

Note that the spy only supports the I2C bus powered at 5V. To spy on an I2C bus powered at 3V, an electrical level translation board must be added to the SCL and SDA signals.

Simply upload the sketch using the ARDUINO IDE.

The electrical connections are:
| spy Port | Direction | I2C Signal | Note                                                     |
|----------|-----------|------------|----------------------------------------------------------|
| D6       | Input     |            |Trigger input. A low state allows spying on the bus. If connected to GND, recording is always active.|
| D7       | Output    |            |Debug output. Only used for debugging. In regular use, do not connect the output.|
| D8       | Input     | SDA | |
| D9       | Input     | SCL | |

The basic idea is to monitor the logical state of the SCL and SDA signals, analyze the exchange protocol, and send a summary of the exchange to a terminal.

Given the limited memory capacity of the UNO R3 board, the exchange volume will be limited to 200 bytes, which corresponds to a memory volume of 1k bytes. 

The transfer to the terminal will occur as soon as the memory is full.  

The basic I2C protocol allows for an exchange at 100 kHz, which is at the limits of the capabilities of the UNO R3 board; modes at 400 kHz or higher cannot be analyzed with this configuration.  

# Motivations  
I am developing a small application with an ARDUINO UNO connected to a sensor via I2C. I am running the example file provided with the sensor library, and the initial measurements returned by the sensor seem erratic.

I suspect there is an initialization problem with the sensor; an I2C spy would make it easier for me to understand the sensor documentation, which is not very precise, and to analyze the library.  

A quick search on the internet does not return any off-the-shelf solutions. There are only professional devices that are beyond my budget.

I have a UNO R3 that is sitting in a drawer, and it will serve as the basis for this project.

From experience, I know that the standard UNO library will not allow me to complete the project.
## For fun
It's really fun to optimize C code to approach the performance of a processor coded directly in assembly.

On the other hand, the project allows for a detailed exploration of the I2C bus, which I only know from a distance.
## To better understand GitHub
This is my first project as a producer; until now, I have only used it as a consumer of solutions. I will be able to explore the tools offered by the platform.
## To help
In the project wiki, I will provide all the details of the design decisions. I hope this will help intermediate and even advanced readers to follow the creation of the software. Perhaps it will inspire some to embark on similar developments.
