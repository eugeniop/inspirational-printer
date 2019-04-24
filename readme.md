### The *Inspirational Printer* TL;DR

The *Inspirational Printer* prints an inspirational message from my favorite philosophers. Every ~10 seconds, it will call an API and check if a quote is available for printing. If a quote is available, it will print it. If there's no quote, it waits anothere 10 seconds and tries again.

> I could have equipped the board with more intelligence for querying: a schedule, etc. but C++ is much harder than JavaScript, this board doesn't have a real-time clock, and even though I toyed with the idea of using NTP, I ended up moving all the logic to the server. The core goal was to experiment with authentication after all.

The printer's brain is an [Arduino based board](https://www.adafruit.com/product/3010) available from Adafruit that comes with WiFi capabilities, among other goodies. It is a pretty impressive little computer!

> The board also comes with TCP, and an HTTPS stack which is, needless to say, very convenient, as I'd rather not implement all that stuff from scratch.

The board is connected to a [thermal printer](https://github.com/adafruit/Adafruit-Thermal-Printer-Library) (also from Adafruit) via one of the available serial ports (`Serial1`).

> `Serial` is mapped to the USB port you connect the Arduino to your computer used for  developing and debugging.

The printer uses the [OAuth Device Grant](https://oauth.net/2/device-flow/) to obtain `access_token` to call the API.
