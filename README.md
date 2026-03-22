# Indoor-climate-monitoring-panel
The device is built for DKE DEPG0370 EPD (E-paper display) demonstration purposes.
It monitors air temperature, relative humidity and air pressure measured using:
-	BMP180 digital pressure sensor
-	HTU21D digital relative humidity and temperature sensor
The control application is written in C for Espressif microcontroller (ESP32, Arduino IDE) and has functions:
-	Displays the last measured parameters on EPD
-	Draws graphics of the last several measurements
-	Controls power consumption of battery supplied system
