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

#pragma once

/* Put your AWS Broker Endpoint here */
#define AWS_BROKER_ADDRESS                 "a2pn4v3xtwglbn-ats.iot.us-east-1.amazonaws.com"

/* AWS Topic on which the Gateway proxies/sends a response to
 * Mesh connection Request to a remote Mesh-controller(or Node)
 */
#define AWS_PUB_TOPIC_MESH_CONN             "proxy_conn_response"

/* AWS Topic on which the Gateway proxies/sends the Mesh data to a
 * remote Mesh-Controller(or Node)
 */
#define AWS_PUB_TOPIC_MESH_DATA             "proxy_data"

/* AWS Topic on which the Gateway proxies/receives the Connection or
 * Disconnection requests from a remote Mesh Controller/Node.
 */
#define AWS_SUB_TOPIC_MESH_CONN             "proxy_conn_request"

/* AWS Topic on which the Gateway proxies/receives the Mesh Network Data
 * from a remote Mesh Controller to be passed down to the Mesh network.
 */
#define AWS_SUB_TOPIC_MESH_DATA             "mesh_data"

/* User can set the AWS credentials using below macros.
 * By default, Don't use these default credentials. These exist
 * to quickly try the application, debugging, running tests etc.
 * See @file gateway_aws_credentials.cpp.
 */
#define AWS_USE_DEFAULT_CREDENTIALS (0)

/* Your Gateway Device Name */
#define GATEWAY_AWS_THING_NAME ""

/* Your Gatewway Device Public-key Certificate */
#define GATEWAY_AWS_THING_CERTIFICATE ""

/* Your Gatewway Device Private-key */
#define GATEWAY_AWS_THING_PRIVATE_KEY ""

/* AWS IoT Root CA certificate */
#define AWS_ROOTCA_CERT ""




