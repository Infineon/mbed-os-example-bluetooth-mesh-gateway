# Introduction
This repository contains the code examples and demos for PSOC6 MCU family of devices bundled with connectivity.
The Application enables Bluetooth Internet Gateway (BIG) as a Mesh Node and thereby making it a Bluetooth Mesh Gateway. On Bootup after flashing, the application connects to configured IoT Protocol such as REST or MQTT based brokers (AWSIoT)  and shall start advertising as an unprovisioned mesh node. A user can now use a provisoner(MeshController) and add the gateway to his/her network and thereby enabling the user to control his/her mesh devices from anywhere from the world.

## Bluetooth Mesh and Cypress Mesh Solution
Bluetooth mesh is a low-power, wireless network that enables many-to-many (m2m) device communication for large-scale device networks. Bluetooth mesh also increases the range achievable by Bluetooth devices by hopping messages from one device to another until it reaches its destination. Applications range from building automation in an industrial environment, to consumer applications, such as home automation

 * The details of the Mesh and Mesh Model spec can be found at https://www.bluetooth.com/specifications/mesh-specifications/

 * The overview, getting started guide of Cypress Mesh solution can be found at https://www.cypress.com/products/ble-mesh

# Instructions to build bluetooth mesh gateway code example

1. Clone the repository

2. Change to the application folder

3. Prepare the cloned working directory for mbed

        mbed config root .

4. Pull the necessary libraries and its dependencies.
This will pull mbed-os, cypress http-server, cypress AWS IoT , cypress bluetooth-gateway libraries and its internal 3rd party dependencies

        mbed deploy

5. Configure the SSID and passphrase of the desired network in the accopmpanying mbed_app.json

6. Configure the type of IoT Protocol which the application would like to use in gateway_config.h.
    - If user chooses to use MQTT broker like AWSIoT, then he/she should configure thing name, thing certificates, thing private key and root certificate 
      in gateway_aws_credentials.cpp.
    - The Thing name created in AWSIoT should be unique and this is used as MQTT client ID.
    - Refer to 'Getting Started with AWS IoT' on the AWS documentation
    - https://docs.aws.amazon.com/iot/latest/developerguide/iot-gs.html
    - If user chooses HTTP, then please ensure to connect MeshController and gateway to the same AP, and once the gateway application connects to AP please note down the IP address of the gateway

7. To build and flash the bluetooth mesh gateway app (.hex binary)
        mbed compile -t GCC_ARM -m CY8CKIT_062S2_43012 -f

8. To setup the mesh network comprising of mesh devices and mesh gateway, install the  MeshController apk located in peerapp folder and follow the instructions described in [MeshController README.md](./peerapp/README.md).

# Supported platforms

This application and it's features are supported on following Cypress platforms:
* CY8CKIT-062S2-43012

# Dependencies
* [ARM mbed-os stack version 5.15.0](https://os.mbed.com/mbed-os/releases)   
  **NOTE:** The Bluetooth mesh gateway library uses the embedded Bluetooth stack present in the CYW43012 Bluetooth chip, and bypasses the Cordio stack.
        Therefore Mbed Cordio Stack APIs should not be used with this library
* [Cypress Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities)
* [Cypress AWS IoT library](https://github.com/cypresssemiconductorco/aws-iot)
* [Cypress HTTP server library](https://github.com/cypresssemiconductorco/http-server)
* [Cypress Bluetooth Gateway](https://github.com/cypresssemiconductorco/bluetooth-gateway)
