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
 * Bluetooth Mesh Gateway NVRAM Read/Write Implementation
 */

#include "mbed.h"

#include "KVStore.h"
#include "kvstore_global_api.h"

#include "gateway_nvram.h"

#define err_code(res) MBED_GET_ERROR_CODE(res)

mesh_dct_t mesh_dct_info;
static const char* mesh_key_kvstore = "/kv/mesh";

#ifdef ENABLE_NVRAM_DEBUG
#define MESH_GATEWAY_NVRAM_DEBUG( X )        printf X
#else
#define MESH_GATEWAY_NVRAM_DEBUG( X )
#endif

#define MESH_GATEWAY_NVRAM_INFO( X )         printf X

#if ENABLE_NVRAM_DEBUG
static void print_mesh_dct_info()
{
    uint8_t i,j;
    MESH_GATEWAY_NVRAM_DEBUG(("==== Dumping Mesh NVRAM Data ====\n\n"));

    MESH_GATEWAY_NVRAM_DEBUG(("Node-Authenticate status: %d\n", mesh_dct_info.node_authenticated));
    MESH_GATEWAY_NVRAM_DEBUG(("Chunk-Index in use: %d\n", mesh_dct_info.index_used));

    for (i = 0 ; i < MESH_NV_DATA_MAX_ENTRIES ; i++)
    {
        MESH_GATEWAY_NVRAM_DEBUG(("\n data_len = %d \n" , mesh_dct_info.mesh_nv_data[i].len));
        MESH_GATEWAY_NVRAM_DEBUG(("\n index = %d \n" , mesh_dct_info.mesh_nv_data[i].index));
        MESH_GATEWAY_NVRAM_DEBUG(("\n ====== \n"));
        for (j = 0 ; j < mesh_dct_info.mesh_nv_data[i].len ; j++)
        {
            MESH_GATEWAY_NVRAM_DEBUG(( "  %d " , mesh_dct_info.mesh_nv_data[i].data[j] ));
        }
        MESH_GATEWAY_NVRAM_DEBUG(("\n ====== \n"));
    }

    MESH_GATEWAY_NVRAM_DEBUG(("============= Done ==============\n\n"));
}
#endif

static int find_index(int id)
{
  int index = -1;
  int i;
  for ( i = 0; i < MESH_NV_DATA_MAX_ENTRIES; i++ )
  {
      if(mesh_dct_info.mesh_nv_data[ i ].index == id)
      {
          index = i;
      }
  }
  return index;

}

cy_rslt_t mesh_kvstore_read(void)
{
    int res = MBED_ERROR_NOT_READY;
    kv_info_t info;

    MESH_GATEWAY_NVRAM_INFO(("[App] Reading from NVRAM..."));

    /* Start by getting key's information */
    res = kv_get_info(mesh_key_kvstore, &info);
#if ENABLE_NVRAM_DEBUG
    MESH_GATEWAY_NVRAM_DEBUG(("\n[App] kv_get_info Key: \"%s\" Result: %d info.size: %u,"
            "info.flags: %lu ...", mesh_key_kvstore, err_code(res), info.size, info.flags));
#endif
    if (err_code(res) != 0)
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - failed to read Mesh Key Info\n"));
        return CY_RSLT_MW_ERROR;
    }

    if (info.size != sizeof(mesh_dct_t))
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - NVRAM size Mismatch [expected of %d bytes, read-size: %d \n", sizeof(mesh_dct_t), info.size));
        return CY_RSLT_MW_ERROR;
    }

    size_t actual_size;
    res = kv_get(mesh_key_kvstore, (void *)&mesh_dct_info, sizeof(mesh_dct_t), &actual_size);
    if (err_code(res) != 0)
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - Failed to fetch Mesh data from KVStore\n"));
        return CY_RSLT_MW_ERROR;
    }
    if (actual_size != sizeof(mesh_dct_t))
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - KVStore actual-size(%d) read mismatch\n", actual_size));
        return CY_RSLT_MW_ERROR;
    }
    MESH_GATEWAY_NVRAM_INFO((" Done.\n"));
    return CY_RSLT_SUCCESS;
}


cy_rslt_t mesh_kvstore_write(void)
{
    int res = MBED_ERROR_NOT_READY;
    kv_info_t info;

    /* Start by getting key's information */
    res = kv_get_info(mesh_key_kvstore, &info);

#if ENABLE_NVRAM_DEBUG
    MESH_GATEWAY_NVRAM_DEBUG("[App] Writing to NVRAM...");

    MESH_GATEWAY_NVRAM_DEBUG(("\n[App] kv_get_info Key: \"%s\" Result: %d info.size: %u,"
            "info.flags: %lu ...", mesh_key_kvstore, err_code(res), info.size, info.flags));
#endif

    if (err_code(res) != 0)
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - failed to read Mesh Key Info\n"));
        return CY_RSLT_MW_ERROR;
    }

    if (info.size != sizeof(mesh_dct_t))
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - NVRAM size Mismatch info.size: %d mesh-dct: %d\n", info.size, sizeof(mesh_dct_t)));
        return CY_RSLT_MW_ERROR;
    }

    res = kv_set(mesh_key_kvstore, (const void*)&mesh_dct_info, sizeof(mesh_dct_t), 0);
    if (err_code(res) != 0)
    {
        MESH_GATEWAY_NVRAM_INFO((" Error - Failed to Set Mesh data to KVStore\n"));
        return CY_RSLT_MW_ERROR;
    }

    return CY_RSLT_SUCCESS;
}

cy_rslt_t mesh_read_dct(void)
{
    int result = mesh_kvstore_read();
    if (result != 0)
    {
        return CY_RSLT_MW_ERROR;
    }
#if ENABLE_NVRAM_DEBUG
    print_mesh_dct_info();
#endif
    return CY_RSLT_SUCCESS;
}

cy_rslt_t mesh_write_dct(uint16_t id, uint8_t *packet, uint32_t packet_len)
{
    int index = mesh_dct_info.index_used + 1;
    int dct_index = find_index(id);

    if( dct_index != -1)
    {
        MESH_GATEWAY_NVRAM_DEBUG(("mesh_write_dct ,existing  index: %d index %d\n", id, dct_index));
        memcpy( (uint8_t*) mesh_dct_info.mesh_nv_data[ dct_index ].data, packet, packet_len );
        mesh_dct_info.mesh_nv_data[ dct_index ].len = (uint8_t) packet_len;
        mesh_dct_info.mesh_nv_data[ dct_index ].index = id;
#if ENABLE_NVRAM_DEBUG
        int val = 0;
        uint32_t i, j;
        MESH_GATEWAY_NVRAM_DEBUG (("\n length of data = %d , data = ", packet_len));
        for (j = 0 ; j < mesh_dct_info.mesh_nv_data[dct_index].len ; j++)
        {
            MESH_GATEWAY_NVRAM_DEBUG(( "  %d " , mesh_dct_info.mesh_nv_data[dct_index].data[j] ));
        }
        MESH_GATEWAY_NVRAM_DEBUG (("\n"));
#endif
    }
    else
    {
        if (index >= MESH_NV_DATA_MAX_ENTRIES)
        {
            MESH_GATEWAY_NVRAM_INFO(("[App] Mesh NVRAM Data - exceeding array bound(idx:%d)\n", index));
            return CY_RSLT_MW_ERROR;
        }
        MESH_GATEWAY_NVRAM_INFO(("mesh_write_dct, to a new index , requested index : %d new index  %d\n", id, index));
        memcpy( (uint8_t*) mesh_dct_info.mesh_nv_data[ index ].data, packet, packet_len );
        mesh_dct_info.mesh_nv_data[ index ].len = (uint8_t) packet_len;
        mesh_dct_info.mesh_nv_data[ index ].index = id;
#if ENABLE_NVRAM_DEBUG
        MESH_GATEWAY_NVRAM_DEBUG (("\n length of data = %d , data = ", packet_len));
        for (int j = 0 ; j < mesh_dct_info.mesh_nv_data[index].len ; j++)
        {
            MESH_GATEWAY_NVRAM_DEBUG(("  %d " , mesh_dct_info.mesh_nv_data[index].data[j]));
        }
        printf ("\n");
#endif
        mesh_dct_info.index_used++;
    }

    int result = mesh_kvstore_write();

    if (result)
        return CY_RSLT_MW_ERROR;

    return CY_RSLT_SUCCESS;
}

cy_rslt_t mesh_reset_nvram_data(void)
{
    int res = MBED_ERROR_NOT_READY;
    kv_info_t info;

    /* Start by getting key's information */
    MESH_GATEWAY_NVRAM_INFO(("[App] Resetting NVRAM...\n"));
    res = kv_get_info(mesh_key_kvstore, &info);
    if (err_code(res) != 0)
    {
        /* The KVstore is not available - ignore the reset */
    }
    else
    {
        res = kv_remove(mesh_key_kvstore);
        if (err_code(res) != 0)
        {
            MESH_GATEWAY_NVRAM_INFO(("[App] Error - Failed to remove KVStore :\"%s\"", mesh_key_kvstore));
            return CY_RSLT_MW_ERROR;
        }
    }
    return CY_RSLT_SUCCESS;
}

cy_rslt_t mesh_init_nvram_data(void)
{
    int res = MBED_ERROR_NOT_READY;
    kv_info_t info;

    /* Start by getting key's information */
    MESH_GATEWAY_NVRAM_INFO(("[App] Fetching NVRAM details...\n"));
    res = kv_get_info(mesh_key_kvstore, &info);
    if (err_code(res) != 0)
    {
        MESH_GATEWAY_NVRAM_INFO(("[App] Setting up Mesh NVRAM Data for first-time..."));
        /* Writing ["/kv/mesh"] =  0 */

        res = kv_set(mesh_key_kvstore, (const void*)&mesh_dct_info, sizeof(mesh_dct_info), 0);
#if ENABLE_NVRAM_DEBUG
        MESH_GATEWAY_NVRAM_DEBUG("\n[App] kv_set for Key: \"%s\" sizeof:%d (returns: %d) ...",
                mesh_key_kvstore, sizeof(mesh_dct_info), err_code(res));
#endif
        if (err_code(res) != 0)
        {
            MESH_GATEWAY_NVRAM_INFO((" Error - Failed to Set Mesh data to KVStore\n"));
            return CY_RSLT_MW_ERROR;
        }
        MESH_GATEWAY_NVRAM_INFO((" Done.\n"));

        MESH_GATEWAY_NVRAM_INFO(("[App] Re-verifying NVRAM details..."));
        res = kv_get_info(mesh_key_kvstore, &info);
#if ENABLE_NVRAM_DEBUG
        MESH_GATEWAY_NVRAM_DEBUG(("\n[App] kv_get_info Key: \"%s\" Result: %d info.size: %u,"
            "info.flags: %lu ...", mesh_key_kvstore, err_code(res), info.size, info.flags));
#endif
        if (err_code(res) != 0)
        {
            MESH_GATEWAY_NVRAM_INFO((" Error - failed to read Mesh Key Info\n"));
            return CY_RSLT_MW_ERROR;
        }

        MESH_GATEWAY_NVRAM_INFO((" Done.\n"));
    }
    else
    {
        MESH_GATEWAY_NVRAM_INFO(("[App] Loading Mesh NVRAM Data..."));
#if ENABLE_NVRAM_DEBUG
        MESH_GATEWAY_NVRAM_DEBUG(("\n[App] kv_get_info Key: \"%s\" Result: %d info.size: %u,"
            "info.flags: %lu ...", mesh_key_kvstore, err_code(res), info.size, info.flags));
#endif
        if (info.size != sizeof(mesh_dct_t))
        {
            MESH_GATEWAY_NVRAM_INFO((" Error - NVRAM size Mismatch [expected of %d bytes, read-size: %d] \n", sizeof(mesh_dct_t), info.size));
            return CY_RSLT_MW_ERROR;
        }

        size_t actual_size;
        res = kv_get(mesh_key_kvstore, (void *)&mesh_dct_info, sizeof(mesh_dct_t), &actual_size);
        if (err_code(res) != 0)
        {
            MESH_GATEWAY_NVRAM_INFO((" Error - Failed to fetch Mesh data from KVStore\n"));
            return CY_RSLT_MW_ERROR;
        }

        if (actual_size != sizeof(mesh_dct_t))
        {
            MESH_GATEWAY_NVRAM_INFO((" Error - KVStore actual-size(%d) read mismatch\n", actual_size));
            return CY_RSLT_MW_ERROR;
        }
        MESH_GATEWAY_NVRAM_INFO((" Done.\n"));
#if ENABLE_NVRAM_DEBUG
        print_mesh_dct_info();
#endif
    }
    return CY_RSLT_SUCCESS;
}
