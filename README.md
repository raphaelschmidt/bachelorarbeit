# bachelorarbeit

Complete Software is packed in the following Docker-Container on docker hub: raphaelschmidt/cic
To use it a LILYGO TTGO T-Camera V1.6.2 and a Linux OS with docker installed is required.

To run the Docker-Conainer type: 
docker run --device=[port_of_your_TTGO] -d -p 5000:5000 raphaelschmidt/cic
If you want to get some debug output omit the -d flag.

You can now access the Webinterface via http://localhost:5000

