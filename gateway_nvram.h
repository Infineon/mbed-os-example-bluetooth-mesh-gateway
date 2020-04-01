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

#include <stdint.h>
#include "cy_result_mw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MESH_NV_DATA_MAX_ENTRIES    (15)
#define MESH_NV_DATA_MAX_PAYLOAD    (200)

typedef struct
{
    uint8_t len;
    uint16_t index;
    uint8_t data[MESH_NV_DATA_MAX_PAYLOAD];
}mesh_chunk_t;

typedef struct
{
    uint8_t       node_authenticated;
    uint8_t       index_used;
    mesh_chunk_t  mesh_nv_data[MESH_NV_DATA_MAX_ENTRIES];
}mesh_dct_t;

cy_rslt_t mesh_read_dct(void);
cy_rslt_t mesh_write_dct(uint16_t id, uint8_t *packet, uint32_t packet_len);
cy_rslt_t mesh_reset_nvram_data(void);
cy_rslt_t mesh_init_nvram_data(void);
cy_rslt_t mesh_kvstore_read(void);
cy_rslt_t mesh_kvstore_write(void);


#ifdef __cplusplus
} /*extern "C" */
#endif
