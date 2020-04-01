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
 * Bluetooth Mesh Gateway HTTP Server implementation
 */
#include "mbed.h"
#include "HTTP_server.h"

#include "gateway_config.h"
#include "bluetooth_gateway.h"

#define HTTP_SERVER_DEFAULT_PORT            (80)
#define HTTP_SERVER_DEFAULT_MAX_SOCKETS     (4)

static cy_http_response_stream_t* http_event_stream = NULL;
static cy_network_interface_t http_nw_interface;
static HTTPServer* http_server = NULL;

static const char json_object_start         [] = "{";
static const char json_data_actuator_status [] = "\"data \": \"";
static const char json_data_end4            [] = "\"}\n";

static const char* gateway_server_uris[] = {
    "/mesh/meshdata/value/*",
    "/mesh/subscribe/sse",
    "/mesh/connect",
    "/mesh/disconnect",
};

static int32_t http_request_mesh_connect(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body);
static int32_t http_request_mesh_disconnect(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body);
static int32_t http_request_mesh_data_received(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body);
static int32_t http_subscribe_event_request(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body);

static cy_resource_dynamic_data_t gateway_server_resources[] =
{
    { http_request_mesh_data_received,  NULL},
    { http_subscribe_event_request,     NULL},
    { http_request_mesh_connect,        NULL},
    { http_request_mesh_disconnect,     NULL},
};

static void hex_bytes_to_chars( char* cptr, const uint8_t* bptr, uint32_t blen )
{
    uint32_t i;
    uint32_t j;
    uint8_t temp;

    i = 0;
    j = 0;
    while( i < blen )
    {
        // Convert first nibble of byte to a hex character
        temp = bptr[i] / 16;
        if ( temp < 10 )
        {
            cptr[j] = temp + '0';
        }
        else
        {
            cptr[j] = (temp - 10) + 'A';
        }
        // Convert second nibble of byte to a hex character
        temp = bptr[i] % 16;
        if ( temp < 10 )
        {
            cptr[j+1] = temp + '0';
        }
        else
        {
            cptr[j+1] = (temp - 10) + 'A';
        }
        i++;
        j+=2;
    }
}

static char* get_payload( const char* url_path )
{
    /* /mesh/<command>/values/<payload> */
    char* current_path = (char*) url_path;
    uint32_t a = 0;

    while ( a < 4 )
    {
        if ( *current_path == '\0' )
        {
            /* <service> not found */
            return NULL;
        }
        else if ( *current_path == '/' )
        {
            a++;
        }

        current_path++;
    }

    return current_path;
}

static int32_t http_request_mesh_data_received(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body)
{
    char* payload = get_payload(url_path);
    http_server->http_response_stream_write_header(stream, CY_HTTP_200_TYPE, CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, MIME_TYPE_TEXT_PLAIN);
    http_server->http_response_stream_disconnect( stream );
    do_mesh_send_data(payload);
    return CY_RSLT_SUCCESS;
}

static int32_t http_request_mesh_connect(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body)
{
    MESH_GATEWAY_INFO(("\n [HTTP] %s",__func__));
    http_server->http_response_stream_write_header(stream, CY_HTTP_200_TYPE, CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, MIME_TYPE_TEXT_PLAIN);
    http_server->http_response_stream_disconnect(stream);
    do_mesh_connect();
    return CY_RSLT_SUCCESS;
}

static int32_t http_request_mesh_disconnect(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body)
{
    MESH_GATEWAY_INFO(("\n [HTTP] %s",__func__));
    http_server->http_response_stream_write_header(stream, CY_HTTP_200_TYPE, CHUNKED_CONTENT_LENGTH, CY_HTTP_CACHE_DISABLED, MIME_TYPE_TEXT_PLAIN);
    http_server->http_response_stream_disconnect(stream);
    do_mesh_disconnect();
    return CY_RSLT_SUCCESS;
}

static int32_t http_subscribe_event_request(const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body)
{
    MESH_GATEWAY_INFO(("\n [HTTP] %s \n",__func__));
    http_event_stream = stream;

    http_server->http_response_stream_enable_chunked_transfer(http_event_stream);

    cy_rslt_t res = http_server->http_response_stream_write_header( http_event_stream,
                    CY_HTTP_200_TYPE, CHUNKED_CONTENT_LENGTH,
                    CY_HTTP_CACHE_DISABLED, MIME_TYPE_TEXT_EVENT_STREAM );

    MESH_GATEWAY_INFO(("\n [%s] subscribe_sse_events =%s res:%lu\n",__func__,url_path, res));

    return res;
}

cy_rslt_t http_response(uint8_t* value, int len)
{
    int i;

    if (http_event_stream == NULL)
    {
        MESH_GATEWAY_ERROR(("\n [App] Error sending HTTP Response: Server-side Event stream is not set\n"));
        return CY_RSLT_ERROR;
    }

    if (!value || len == 0)
    {
        MESH_GATEWAY_ERROR(("\n [App] Invalid HTTP Response arguments"));
        return CY_RSLT_ERROR;
    }

    char*  adv_data = (char*) malloc( sizeof(char) * ((len*2) + 1) );
    if (!adv_data)
    {
        MESH_GATEWAY_ERROR(("\n [App] Failed to malloc while sending HTTP Response"));
        return CY_RSLT_ERROR;
    }

    http_server->http_response_stream_write( http_event_stream, json_object_start, sizeof( json_object_start ) - 1 );
    http_server->http_response_stream_write( http_event_stream, json_data_actuator_status, sizeof( json_data_actuator_status ) - 1 );

    for( i = 0; i < ( len ); i++ )
    {
        printf("%x",value[i]);
    }

    hex_bytes_to_chars(adv_data, value, len);
    adv_data[len*2] = 0;
    MESH_GATEWAY_INFO(("\n [App] sending HTTP response[Payload: %s ]\n",adv_data));

    http_server->http_response_stream_write( http_event_stream, adv_data, strlen(adv_data) );
    http_server->http_response_stream_write( http_event_stream, json_data_end4, sizeof( json_data_end4 ) - 1 );
    http_server->http_response_stream_flush( http_event_stream );

    if (value[0] == 0x00 && value[1] == 0x00 )
    {
        MESH_GATEWAY_INFO(("\n [App] http_response : disconnecting the stream \n"));
        http_server->http_response_stream_disconnect(http_event_stream);
        http_event_stream = NULL;
    }

    free(adv_data);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t setup_http_server(NetworkInterface* network)
{
    cy_rslt_t result = CY_RSLT_ERROR;

    if (!network)
    {
        return result;
    }
    http_nw_interface.object = (void *)network;
    http_nw_interface.type = CY_NW_INF_TYPE_WIFI;

    http_server = new HTTPServer(&http_nw_interface, HTTP_SERVER_DEFAULT_PORT, HTTP_SERVER_DEFAULT_MAX_SOCKETS);
    if (!http_server)
    {
        MESH_GATEWAY_ERROR(("[App] Error Creating Instance of HTTP-Server\n"));
        return CY_RSLT_ERROR;
    }

    for (int i=0; i < 4; i++)
    {
        http_server->register_resource((uint8_t*)gateway_server_uris[i], (uint8_t*)"application/json",
                            CY_RAW_DYNAMIC_URL_CONTENT, (void *)&gateway_server_resources[i]);
    }

    result = http_server->start();
    MESH_GATEWAY_INFO(("\n [App] HTTP Server started (result: %lu)\n", result));

    return CY_RSLT_SUCCESS;
}

cy_rslt_t teardown_http_server(void)
{
    return CY_RSLT_ERROR;
}
