/*
 * Copyright 2020 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file
 *
 * Bluetooth Mesh Gateway application implementation
 */

#include <iostream>
#include <assert.h>

#include "mbed.h"
#include "EventQueue.h"
#include "mbed_trace.h"

#include "cloud_client.h"
#include "cloud_client_default_config.h"

#include "embedded_BLE.h"
#include "embedded_BLE_mesh.h"

#include "gateway_config.h"
#include "bluetooth_gateway.h"

#include "JSON.h"
#include "cy_string_utils.h"

using namespace cypress::embedded;
using namespace std;

#define MESH_DATA_JSON_KEY             "status"

#define MESH_NODE_UNPROVISIONED         0   // NODE in UNPROVISIONED STATE
#define MESH_NODE_PROVISIONED           1   // NODE in PROVISIONED STATE
#define MESH_PROVISION_RESULT_SUCCESS   0   ///< Provisioning succeeded
#define MESH_PROVISION_RESULT_TIMEOUT   1   ///< Provisioning failed due to timeout
#define MESH_PROVISION_RESULT_FAILED    2   ///< Provisioning  failed

#define MQTT_SUBSCRIBE_RETRY_COUNT          (3)
#define MESH_JSON_SCRATCHPAD_SIZE           (120)
#define MESH_AWS_YIELD_TIMEOUT_IN_MSEC      (1000)
#define MESH_AWS_KEEP_ALIVE_TIMEOUT_IN_SEC  (60)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t length;
    uint8_t  value[1];
} mesh_value_handle_t;

#ifdef __cplusplus
}
#endif

#if APP_CONFIG_AWS_CLOUD
static CloudClientFactory factory;
static char received_data[MESH_JSON_SCRATCHPAD_SIZE] = { 0 };
#endif

struct bluetooth_gateway
{
    NetworkInterface*       network;
    BLE*                    ble;
    CloudClient*            cloud;
} app_data = { 0 };

class ButtonHandler
{
private:
    int btn_pressed;
    int btn_released;
    bool was_button_pressed_and_released;
public:
    ButtonHandler():btn_pressed(0), btn_released(0), was_button_pressed_and_released(false){
    }

    void button_pressed(void) {
        if (!btn_pressed)
        {
            btn_pressed = 1;
            btn_released = 0;
        }
        was_button_pressed_and_released = false;
    }

    void button_released(void) {
        if (!btn_released)
        {
            if (btn_pressed)
            {
                was_button_pressed_and_released = true;
            }
            btn_released = 1;
            btn_pressed = 0;
        }
    }

    bool is_button_pressed_and_released(void)
    {
        return was_button_pressed_and_released;
    }
};

/* Utilities Functions */

static mesh_value_handle_t* reverse_mesh_byte_stream( mesh_value_handle_t* val )
{
    uint32_t i;
    mesh_value_handle_t* ret_val = (mesh_value_handle_t*) malloc( sizeof(mesh_value_handle_t) + val->length );
    MESH_GATEWAY_INFO(("\n Received mesh proxy packet : "));
    for (i = 0; i < val->length; i++ )
    {
        ret_val->value[ i ] = val->value[ val->length - 1 - i ];
        MESH_GATEWAY_INFO((" %02X " , ret_val->value[ i ]));
    }
    ret_val->length = val->length;
    return ret_val;
}

static mesh_value_handle_t* create_mesh_value( const char* payload )
{
    uint32_t temp  = strlen( payload ) / 2;
    mesh_value_handle_t* value = (mesh_value_handle_t*)malloc(sizeof( mesh_value_handle_t ) + temp );
    if ( value != NULL )
    {
        uint32_t a;
        uint32_t b;
        memset( value, 0, sizeof( mesh_value_handle_t ) + temp );
        value->length = temp;
        for ( a = 0, b = ( value->length - 1 ) * 2 ; a < value->length; a++, b-=2 )
        {
            cy_string_to_unsigned(&payload[b], 2, &temp, 1);
            value->value[a] = (uint8_t)( temp & 0xff );
        }
    }

    return value;
}

#if APP_CONFIG_AWS_CLOUD
static char* to_text(const uint8_t* bytes, uint16_t len, uint16_t *ret_len)
{
    uint16_t index;
    uint16_t i;
    if (len == 0)
        return NULL;

    char* buffer = (char*) malloc ((len*2)+1);
    *ret_len = (len * 2);
    if (buffer == NULL)
        return NULL;

    for (i = 0, index = 0; i < len; i++)
    {
        int data = bytes[i];
        if (data < 0)
        {
            data += 256;
        }

        uint8_t n = (uint8_t) (data >> 4);
        if (n < 10) {
            buffer[index++] = (char) ('0' + n);
        } else {
            buffer[index++] = (char) ('A' + n - 10);
        }

        n = (uint8_t) (data & 0x0F);
        if (n < 10) {
            buffer[index++] = (char) ('0' + n);
        } else {
            buffer[index++] = (char) ('A' + n - 10);
        }
    }

    buffer[index] = '\0';
    return buffer;
}
#endif

static void mesh_event_callback(Mesh::BluetoothMeshEvent event, Mesh::MeshEventCallbackData* payload)
{

    MESH_GATEWAY_DEBUG(("[App] %s Mesh Event:%02x Payload:%p\n", __func__, event, payload));
    switch(event)
    {
        /* Provisioning status of Bluetooth device */
        case Mesh::BLUETOOTH_MESH_DEVICE_PROVISIONING_STATUS:
        {
            if (payload)
            {
                MESH_GATEWAY_INFO(("[App] Mesh-Provisioning status %2x\n", payload->provisioning.status));
                if(payload->provisioning.status == MESH_PROVISION_RESULT_SUCCESS)
                {
                    mesh_dct_info.node_authenticated = MESH_NODE_PROVISIONED;
                    mesh_kvstore_write();
                }
            }
        }
        break;

        /* Generic Mesh status of Bluetooth device */
        case Mesh::BLUETOOTH_MESH_DEVICE_STATUS:
        {

        }
        break;

        /* Data received from Mesh network to be sent to cloud */
        case Mesh::BLUETOOTH_MESH_NETWORK_RECEIVED_DATA:
        {
            if (payload)
            {
                uint32_t packet_len = payload->network.length;
                uint8_t* packet = payload->network.packet;
                MESH_GATEWAY_DEBUG(("[App] Proxy Data received %p %lu\n", payload->network.packet, payload->network.length));
                uint8_t* val = (uint8_t*)malloc(sizeof(uint8_t)*(packet_len+1));
                uint8_t* ptr;
                ptr = val;
                ptr++;
                memcpy(ptr, packet,packet_len);

#ifdef APP_CONFIG_HTTP_SERVER
                    val[0] = 0x01;
                    http_response(val, packet_len + 1);
#endif
#ifdef APP_CONFIG_AWS_CLOUD
                uint16_t len = 0;
                char* data = to_text(packet, packet_len, &len);
                AWSMQTTClient* client = (AWSMQTTClient* )app_data.cloud;
                if (app_data.cloud)
                    client->publish(AWS_PUB_TOPIC_MESH_DATA, (uint8_t*)data, len);
#endif
                free(val);
                ptr = NULL;
            }
        }
        break;

        /* Mesh Network status change */
        case Mesh::BLUETOOTH_MESH_NETWORK_STATUS:
        {
            MESH_GATEWAY_INFO(("[App] Mesh Network status received\n"));
        }
        break;

        /* Update NVRAM data */
        case Mesh::BLUETOOTH_MESH_NVRAM_DATA:
        {
            if (payload)
            {
                mesh_write_dct(payload->nvram.id, payload->nvram.data, payload->nvram.length);
            }
        }
        break;

        default:
            break;
    }
}

static void ble_init_callback(void)
{
    MESH_GATEWAY_INFO(("[App] BLE Initialization Callback has been triggered\n"));
}

static cy_rslt_t setup_wifi_network(void)
{
    nsapi_error_t net_status = -1;

    app_data.network = NetworkInterface::get_default_instance();
    const char *addr;
    SocketAddress address;

    MESH_GATEWAY_INFO(("[App] Trying to connect WiFi...\n"));
    for (int tries = 0; tries < 3; tries++)
    {
        net_status = app_data.network->connect();
        if (net_status == NSAPI_ERROR_OK)
        {
            break;
        }
        else
        {
            MESH_GATEWAY_INFO(("Unable to connect to Network. Retrying...\n"));
        }
    }

    if (net_status != NSAPI_ERROR_OK)
    {
        MESH_GATEWAY_INFO(("[App] Error: Connecting to the network failed \n"));
        return CY_RSLT_MW_ERROR;
    }

    net_status = app_data.network->get_ip_address(&address);
    if(net_status != NSAPI_ERROR_OK)
    {
        MESH_GATEWAY_INFO(("[App] get ip address failed \n"));
        return CY_RSLT_MW_ERROR;
    }
    else
    {
        addr = (const char *) address.get_ip_address();
        MESH_GATEWAY_INFO(("[App] Connected to the network successfully. IP address: %s\n", addr));
    }
    return CY_RSLT_SUCCESS;
}

#if APP_CONFIG_AWS_CLOUD

static cy_rslt_t parse_json_shadow_status(cy_JSON_object_t * json_object )
{
    if(strncmp(json_object->object_string, MESH_DATA_JSON_KEY, strlen(MESH_DATA_JSON_KEY)) == 0)
    {
         if(json_object->value_length > 0 && json_object->value_length < sizeof(received_data)-1)
        {
          memcpy(received_data, json_object->value, json_object->value_length);
          received_data[json_object->value_length] = '\0';
        }
     }

    return CY_RSLT_SUCCESS;
}

static void mesh_aws_topic_connection_callback(aws_iot_message_t& md)
{
    uint8_t* payload = (uint8_t *)md.message.payload;
    uint32_t payload_length = md.message.payloadlen;

    MESH_GATEWAY_INFO(("[App] Subscriber callback received Mesh Connection status(from AWS) -- Payload: %.*s\n",(int)payload_length, payload));

    if (payload_length == 1 && payload[0] == 49) //comparing to connected "1"
    {
        MESH_GATEWAY_INFO(("[App] Mesh Connection status: CONNECT\n"));
        do_mesh_connect();
    }
    else if (payload_length == 1 && payload[0] == 48)
    {
        MESH_GATEWAY_INFO(("[App] Mesh Connection status: DISCONNECT\n"));
        do_mesh_disconnect();
    }
}

static void mesh_aws_topic_data_callback(aws_iot_message_t& md)
{
    uint8_t* payload = (uint8_t *)md.message.payload;
    uint32_t payload_length = md.message.payloadlen;
    MESH_GATEWAY_DEBUG(("[App] Subscriber callback received Mesh Data(from AWS) -- Payload: %.*s\n",(int)payload_length, payload));
    cy_rslt_t ret = cy_JSON_parser((const char*) payload, payload_length);
    if (ret == CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_DEBUG(("[App] Subscriber Callback Reported data :  [%s]\n", received_data));
    }

    /* Now Send data received from AWS to Mesh Network */
    do_mesh_send_data(received_data);
}


static cy_rslt_t setup_aws_cloud(void)
{
    /* Initialize Security params for this client */
    ClientSecurity aws_security(CLIENT_SECURITY_TYPE_TLS);

    int result = aws_security.set_tls_params(aws_thing_name, aws_thing_private_key, aws_thing_certificate);
    if (result != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Error setting TLS Parameters(name, key, cert) \n"));
        return CY_RSLT_MW_ERROR;
    }

    result = aws_security.set_tls_root_certificate(aws_root_cert);
    if (result != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Error setting root Cert \n"));
        return CY_RSLT_MW_ERROR;
    }

    CloudClient* c = factory.getClient((NetworkInterface&)*(app_data.network), CLIENT_MQTT_AWS, &aws_security);
    /* Do custom initialization post-object creation */
    result = c->initialize();
    if (result != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("Failed to initialize cloud client result = %d \n", result ));
        return result;
    }

    app_data.cloud = c;

    /* Get the Remote server endpoint */

    ClientConnectionParams aws_connection(AWS_BROKER_ADDRESS, AWS_MQTT_DEFAULT_SECURE_PORT, MESH_AWS_KEEP_ALIVE_TIMEOUT_IN_SEC);

    result = c->connect(&aws_connection);
    if (result != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("Failed to connect to AWS IoT , result = %d \n", result));
        return result;
    }
    int retries = 0;
    int ret = 0;
    do
    {
        MESH_GATEWAY_INFO(("[App] Subscribing to topic: \"%s\"\n", AWS_SUB_TOPIC_MESH_CONN));
        ret = ((AWSMQTTClient *)c)->subscribe(AWS_SUB_TOPIC_MESH_CONN, mesh_aws_topic_connection_callback);
        retries ++;
    } while ( ( ret != CY_RSLT_SUCCESS ) && ( retries < MQTT_SUBSCRIBE_RETRY_COUNT ) );

    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("Failed\n"));
        return CY_RSLT_MW_ERROR;
    }
    retries = 0;
    do
    {
        MESH_GATEWAY_INFO(("[App] Subscribing to topic: \"%s\"\n", AWS_SUB_TOPIC_MESH_DATA));
        ret = ((AWSMQTTClient *)c)->subscribe(AWS_SUB_TOPIC_MESH_DATA, mesh_aws_topic_data_callback);
        retries ++;
    } while ( ( ret != CY_RSLT_SUCCESS ) && ( retries < MQTT_SUBSCRIBE_RETRY_COUNT ) );

    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("Failed\n"));
        return CY_RSLT_MW_ERROR;
    }
    MESH_GATEWAY_INFO(("[App] AWS Subscriptions Successful.\n"));
    wait_us(100 * 1000);

    return CY_RSLT_SUCCESS;
}
#endif

static void do_main_loop(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    while(result == CY_RSLT_SUCCESS)
    {
#if APP_CONFIG_AWS_CLOUD
        if (app_data.cloud)
        {
            result = ((AWSMQTTClient *)app_data.cloud)->yield(MESH_AWS_YIELD_TIMEOUT_IN_MSEC);
            if ( result == CY_RSLT_AWS_ERROR_DISCONNECTED )
            {
                MESH_GATEWAY_INFO(("Disconnected from AWS broker, one reason could be that the Thing name is not unique \n"));
            }
        }
#endif
        wait_us(200 * 1000);
    }
    return;
}

void do_mesh_connect(void)
{
    BLE& ble = BLE::Instance();
    Mesh& mesh = ble.mesh();
    /* Wait sufficient time to allow GATT disconnection */
    osDelay(1000);
    mesh.connectMesh();
#ifdef APP_CONFIG_HTTP_SERVER
    uint8_t value[] = {0x00, 0x01};
    http_response(value, 2);
#endif

#if APP_CONFIG_AWS_CLOUD
    const char* val = "1";
    if (app_data.cloud)
        ((AWSMQTTClient *)app_data.cloud)->publish(AWS_PUB_TOPIC_MESH_CONN, (uint8_t*)val, 1);
#endif

}

void do_mesh_disconnect(void)
{
    BLE& ble = BLE::Instance();
    Mesh& mesh = ble.mesh();
    mesh.disconnectMesh();

#if APP_CONFIG_HTTP_SERVER
    uint8_t value[] = {0x00, 0x00};
    http_response(value, 2);
#endif

#if APP_CONFIG_AWS_CLOUD
    const char* val = "0";
    if (app_data.cloud)
        ((AWSMQTTClient *)app_data.cloud)->publish(AWS_PUB_TOPIC_MESH_CONN, (uint8_t*)val, 1);
#endif
}

void do_mesh_send_data(char* payload)
{
    mesh_value_handle_t* value = create_mesh_value(payload);
    mesh_value_handle_t* reversed_value = reverse_mesh_byte_stream(value);
    /* Get EmbeddedBLE singleton object */
    BLE& ble = BLE::Instance();
    Mesh& mesh = ble.mesh();
    /* Throttling the mesh data that is sent to the Controller */
    wait_us(100 * 1000);
    mesh.sendData(reversed_value->value, reversed_value->length);
    free( value );
    free( reversed_value );
}

int main(void)
{
    cy_rslt_t ret = CY_RSLT_MW_ERROR;

    ButtonHandler btn_handler;
    InterruptIn flash_button(RESET_FLASH_BUTTON_PIN_NAME, RESET_FLASH_BUTTON_PIN_PULL);
    int timeout = 0; // Wait for 5 seconds for User to trigger the button for resetting flash

    flash_button.fall(Callback<void()>(&btn_handler, &ButtonHandler::button_pressed));
    flash_button.rise(Callback<void()>(&btn_handler, &ButtonHandler::button_released));

    MESH_GATEWAY_INFO(("[App] Press USER_BTN1 on the board to reset the Flash(Timeout in 5 Seconds...) >>\n"));

    while(!btn_handler.is_button_pressed_and_released() && timeout < 5)
    {
        wait_us(1000 * 1000);
        timeout += 1;
    }

    if (btn_handler.is_button_pressed_and_released())
    {
        mesh_reset_nvram_data();
    }

    ret = setup_wifi_network();
    if(ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Error setting up Wi-Fi network\n"));
        return -1;
    }

#if APP_CONFIG_HTTP_SERVER
    ret = setup_http_server(app_data.network);
    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Error setting up HTTP-server\n"));
        return -1;
    }
#endif

#if APP_CONFIG_AWS_CLOUD
    ret = setup_aws_cloud();
    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Failed to set-up AWS Cloud Connection \n"));
        return -1;
    }

    ret = cy_JSON_parser_register_callback(parse_json_shadow_status);
    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Failed to register JSON Parser callback\n"));
        return -1;
    }
#endif

    ret = mesh_init_nvram_data();
    if (ret != CY_RSLT_SUCCESS)
    {
        MESH_GATEWAY_INFO(("[App] Error loading/initializing NVRAM data\n"));
        return -1;
    }

    /* Get EmbeddedBLE singleton object */
    BLE& ble = BLE::Instance();
    app_data.ble = &ble;

    ble.init(ble_init_callback);

    /* busy-wait till BLE controller is up. Can make it asynchronous as well using the init_callback */
    while (!ble.hasInitialized())
    {
        wait_us(100 * 1000);
    };

    Mesh& mesh = ble.mesh();

    MESH_GATEWAY_INFO(("mesh_dct_info.node_authenticated = %d ...\n", mesh_dct_info.node_authenticated));
    if (mesh_dct_info.node_authenticated == MESH_NODE_PROVISIONED)
    {
        MESH_GATEWAY_DEBUG("Write NVRam chunks to BLE Controller...\n");
        for(int i = 0; i < MESH_NV_DATA_MAX_ENTRIES ; i++)
        {
            mesh.pushNVData( mesh_dct_info.mesh_nv_data[ i ].data, mesh_dct_info.mesh_nv_data[ i ].len, mesh_dct_info.mesh_nv_data[ i ].index );
            /* TO DO : push nvm chunks with a delay */
            wait_us(100 * 1000);
        }
     }

    mesh.initialize();
    mesh.registerMeshEventcallback(mesh_event_callback);

    do_main_loop();
    return 1;
}
