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
 * Bluetooth Mesh Gateway implementation for fetching AWS security credentials
 * Developers can define methods in which these variables will
 * be filled(from Secure storage etc.)
 */
#include "gateway_config.h"
#include "gateway_aws_config.h"

#if APP_CONFIG_AWS_CLOUD

const char* aws_thing_name = "";

const char* aws_thing_certificate = "";

const char* aws_thing_private_key = "";

const char* aws_root_cert = "";

#endif
