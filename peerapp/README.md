# Introduction
MeshController is the application which creates and manages the mesh network.
This document describes, how Android MeshController can be used to add mesh devices and mesh gateway and manages the network.

## Prerequisites, Installation and Usage of MeshController
1. The user is expected to have cypress mesh devices, for quick start guide of cypress mesh solution refer https://www.cypress.com/products/ble-mesh.

2. The user should have adb installed in the PC, for adb installation please refer to ttps://developer.android.com/studio/command-lihttps://www.cypress.com/products/ble-mesh.ne/adb
3. The user is expected to have an Android phone with OS version Android 7.0 or greater.

4.  Install MeshLightingController.apk on the android phone through the shell by using the command "adb install -r MeshController.apk"

5. Start the application and grant permission when app opens for first time.

6. Once you are on the application home screen, create network and create a room and start adding mesh devices.
    1. The UI of Android MeshController is implemented for a home automation scenario and hence it expects all mesh devices to be added to a room (in mesh terminology rooms are analogous to groups).
    2. The Android Application serves only as a reference app for developers, the source code of app and MeshController puplic api are available as part of BT-SDK installation located <installation path>/common/mesh/peerapp/Android.

## Adding Light

1. Install BT-SDK 1.3 and "create a Dimmable Light Bulb Project" and program
CYBT-213043-MESH kits. On first bootup after programing, the devices will start as unprovisioned devices which can added (provisioned) to the network.

2. Use MeshController to add Lights to the network
    a.  Click on Add device to search for unprovisioned light and select the device to start provisioning.
    b. The application wraps provisioning and configuration as part of adding light
    Note : it might take about 10-20 seconds to add a light
    c. Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.
3. Depending on the type of device added, appropriate UI option (OnOff/HSL) will appear
4. To do device specific operation click on the device and use the controls.

## Adding Temperature Sensor

1. Install BT-SDK 1.3 and "create a Sensor Temperature project" and program
CYBT-213043-MESH kits. On first bootup after programing, the devices will start as unprovisioned devices which can added (provisioned) to the network.

2. Use MeshController to add sensor to the network
    a.  Click on Add device to search for unprovisioned sensor and select the device to start provisioning.
    b.  The application wraps provisioning and configuration as part of adding sensor
     Note : it might take about 10-20 seconds to add a sensor
    c.  Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.

3. Depending the type of device added, appropriate UI option (sensor) will appear
4. Configuration of sensor 
    1. Select the property of sensor to control/configure. (Currently only temperature sensor is supported)
    2. Click on "Configure", now you can configure sensor publication, cadence and settings.
    3. To get the current sensor data click on "Get Sensor Data".
    4. set cadence of the sensor :
        set minimum interval in which sensor data has to be published.
        set the range in which the fast cadence has to be observed.
        set the fast cadence period (how fast the data has to be published with respect to publish period).
        set the unit in which if the values change the data should be published and trigger type (Native or percentage), example : publish data if the data changes by 2 units/10%

## Adding Switch

1. Install BT-SDK 1.3 and "create a Mesh OnOff Switch project" and program
CYBT-213043-MESH kits. On first bootup after programing, the devices will start as unprovisioned devices which can added (provisioned) to the network.

2. Use MeshController to add Switch to the network
    a.  Click on Add device to search for unprovisioned switch and select the device to start provisioning.
    b.  The application wraps provisioning and configuration as part of adding sensor
     Note : it might take about 10-20 seconds to add a sensor
    c.  Once provisioning and configuration is completed provision complete popup will appear and the appropriate device will be seen on the UI.

3. Depending the type of device added, appropriate UI option (Switch) will appear
4. To do device specific operation click on the device and use the controlls.
5. To assign light to a switch :
    1. Click on assign button on the switch. Select appropriate light from the Popup.
       Light selected will be assigned to the switch.
    2. To use the switch press the user button on  CYBT-213043-MESH kitM, you can see that you are able to turn on and turn off the light using the switch.

## Mesh OTA support
 
1. On the UI if user clicks on any of  added device user will find an option to upgrade OTA
2. store the ota file to the phone and provide the path to the ota file  
3. Create mesh OTA file using the appropriate mesh app in SDK , 
for example when user builds onOff server application using wiced SDK , a binary file is located in the build directory
for OnOffServer the file in the build directory would be named as "mesh_onoff_server-BCM920819EVAL_Q40-rom-ram-Wiced-release.ota.binonOffServer.bin"

## Mesh database JSON export/Import

Cypress Mesh Controller framework stores mesh network information in .json file format specified by SIG MESH WG
1. During Provisioning Android Mesh Controller stores the database in application's internal memory
2. To exercise usecase such as control of mesh devices using multiple phones follow the below steps
    a. After creating a network and provisioning few devices on phone P1
       use the option "export network" in the settings of home screen to export the required meshdb.
    b. Cypress mesh lighting app stores the exported Meshdb in "/sdcard/exports" directory.
    c. A user can now move the exported file to another phone (Say phone P2)
    d. Install MeshLighting app on P2. Use the "import network" option avaible in the settings menu of main screen.
    e. The user can now control the mesh devices using P2 .
    for more information refer the public apis (ImportNetwork/exportNetwork) in MeshController.java  

## Controlling mesh devices through cloud using Mesh Gateway
To expose a BLE only mesh network to IoT, a user will have to add a Mesh Gateway to his/her network.
Cypress Mesh Solution offers Mesh Gateway on CY8CKIT_062S2_43012 (BT+Wifi Combo platform) on Mbed OS.
Users can setup the mesh gateway by downloading the application from Mbed repository, the mesh gateway project can be located on the Mbed repository by searching for the name  "mbed-os-example-bluetooth-mesh-gateway".

## Instructions to add mesh gateway  and control  devices in Home and Away mode 
Adding a mesh gateway to a mesh network enables user to control his/her BLE only mesh devices Internet. Typically a user would choose a IoT Protocol to send and recieve mesh data, on Cypress mesh gateway and Android MeshController we support REST and MQTT as a configurable IoT protocol. Below are the steps to add mesh gateway to the network.

1. User chooses either REST or MQTT as the preffered IoT protocol on the mesh gateway app and choose the same protocol on the MeshController.

2. If user chooses use MQTT via AWS then its important to ensure MeshController and gateway are connected to the same "AWSIoT Thing" and  uses the same AWS credentials and this is enabled on Android MeshController through a file named AWS.conf. 
Below are the steps to load AWS credentials on MeshController.
          In the peerapp folder, user can find AWS.Conf file, please copy this file to /sdcard of your android phone.
          Please refer to comments of AWS.conf to add aws credentials.
          Ensure phone and gateway has internet connectivity and  check network configuration to ensure that MQTT/AWS ports are not blocked by the firewall (such as corporate networks). Recommend using personal hotspots or home WiFi networks.

3. To add a mesh gateway, use setting option in home screen and click on "Add BT Internet Gateway".

4. If the mesh gateway is in unprovisioned state, user can see the gateway advertising with name "mesh proxy", select the device to add the gateway to the network.

5. If the chosen transport is REST, then the user need not use AWS.Conf, however its expected the MeshController and gateway are both connected to the same AP and while provisioning the gateway the user is expected to key-in the IP address of the gateway .
   note : IP address of gateway should be available in gateway's console.

6. After provisioning the gateway, the user can exercise HOME/AWAY usecases.

7. HOME mode:  Its a mode in which the Android MeshController connects to the mesh network using a GATT connection with one of mesh devices in the vicinity

8. AWAY mode : In this mode, its assumed that the Android MeshController is not in the vicinity of the mesh network and it connects to the network through gateway using an IoT Protocol. 

9. To switch to AWAY mode, use the setting option of the home screen and select
  "go to Away", this will disconnect the proxy connection and the phone will connect to mesh gateway through REST/MQTT protocol. Both in Home and Away mode, the user should be able to control his/her mesh devices.
