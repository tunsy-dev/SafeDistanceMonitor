# SafeDistanceMonitor
A wearable device that can monitor employees distance from each other to help implement social distancing.


The goal of this project is to develop a smart wearable device to support employees in social distancing practices.

The solution that was attained was to create a wearable smart device that will support employees in being aware of the safe distancing rules.

Using Beacon technology the devices provide a small alert to the employee if they are within 2 meters of another employee. For demonstration purposes, a buzzer is used to give the alert. The data received from these devices could additionally be visualized in a custom dashboard. This would allow decision-makers informed insights into safe distancing when developing new scheduling and timings policies.

In order to create the device and make it work as both a transmitter and receiver, I utilized the esp32s dual cores. Using one core for the transmitter and one for the receiver. An infinite for loop was run for both the receiver and the transmitter. The receiver code ends with a deep sleep command in order to ensure both the transmitter and receiver code gets executed.

The distance can be set by changing the RSSI CUTOFF value
