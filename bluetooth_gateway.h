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
 * Bluetooth Mesh Gateway application header
 */

#include "gateway_nvram.h"
#include "cy_result_mw.h"

#ifdef ENABLE_DEBUG_TRACES
#define MESH_GATEWAY_DEBUG( X )        printf X
#else
#define MESH_GATEWAY_DEBUG( X )
#endif

#define MESH_GATEWAY_INFO( X )         printf X

#define MESH_GATEWAY_ERROR( X )        printf X

extern mesh_dct_t mesh_dct_info;
extern const char* aws_thing_name;
extern const char* aws_thing_certificate;
extern const char* aws_thing_private_key;
extern const char* aws_root_cert;

/* Public Application-level methods - required for interaction between Mesh and Cloud-modules */
void do_mesh_connect(void);
void do_mesh_disconnect(void);
void do_mesh_send_data(char* payload);

cy_rslt_t http_response(uint8_t* value, int len);
cy_rslt_t setup_http_server(NetworkInterface* network);
cy_rslt_t tearddown_http_server(void);
