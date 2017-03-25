/*
 * Copyright (c) 2017, CellWire Group
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define TRACE_MODULE _gtptypes

#include "core_debug.h"
#include "core_lib.h"
#include "gtp_types.h"

/* 8.13 Protocol Configuration Options (PCO) 
 * 10.5.6.3 Protocol configuration options in 3GPP TS 24.008 */
c_int16_t gtp_parse_pco(gtp_pco_t *pco, tlv_octet_t *octet)
{
    gtp_pco_t *source = (gtp_pco_t *)octet->data;
    c_int16_t size = 0;
    int i = 0;

    d_assert(pco, return -1, "Null param");
    d_assert(octet, return -1, "Null param");

    memset(pco, 0, sizeof(gtp_pco_t));

    pco->ext = source->ext;
    pco->configuration_protocol = source->configuration_protocol;
    size++;

    while(size < octet->len && i < GTP_MAX_PROTOCOL_OR_CONTAINER_ID)
    {
        gtp_protocol_or_container_id_t *id = &pco->ids[i];
        d_assert(size + sizeof(id->id) <= octet->len, 
                return -1, "decode error");
        memcpy(&id->id, &octet->data[size], sizeof(id->id));
        id->id = ntohs(id->id);
        size += sizeof(id->id);

        d_assert(size + sizeof(id->length) <= octet->len, 
                return -1, "decode error");
        memcpy(&id->length, &octet->data[size], sizeof(id->length));
        size += sizeof(id->length);

        id->contents = octet->data + size;
        size += id->length;

        i++;
    }
    pco->num_of_id = i;
    d_assert(size == octet->len, return -1, "decode error");
    
    return size;
}
c_int16_t gtp_build_pco(
        tlv_octet_t *octet, gtp_pco_t *pco, void *data, int data_len)
{
    gtp_pco_t target;
    c_int16_t size = 0;
    int i = 0;

    d_assert(pco, return -1, "Null param");
    d_assert(octet, return -1, "Null param");
    d_assert(data, return -1, "Null param");
    d_assert(data_len, return -1, "Null param");

    octet->data = data;
    memcpy(&target, pco, sizeof(gtp_pco_t));

    d_assert(size + 1 <= data_len, return -1, "encode error");
    memcpy(&octet->data[size], &target, 1);
    size += 1;

    d_assert(target.num_of_id <= GTP_MAX_PROTOCOL_OR_CONTAINER_ID, 
            return -1, "encode error");
    for (i = 0; i < target.num_of_id; i++)
    {
        gtp_protocol_or_container_id_t *id = &target.ids[i];

        d_assert(size + sizeof(id->id) <= data_len, 
                return -1, "encode error");
        id->id = htons(id->id);
        memcpy(&octet->data[size], &id->id, sizeof(id->id));
        size += sizeof(id->id);

        d_assert(size + sizeof(id->length) <= data_len, 
                return -1, "encode error");
        memcpy(&octet->data[size], &id->length, sizeof(id->length));
        size += sizeof(id->length);

        d_assert(size + id->length <= data_len, return -1, "encode error");
        memcpy(&octet->data[size], id->contents, id->length);
        size += id->length;
    }

    octet->len = size;

    return octet->len;
}

/* 8.15 Bearer Quality of Service (Bearer QoS) */
c_int16_t gtp_parse_bearer_qos(
    gtp_bearer_qos_t *bearer_qos, tlv_octet_t *octet)
{
    gtp_bearer_qos_t *source = (gtp_bearer_qos_t *)octet->data;
    c_int16_t size = 0;

    d_assert(bearer_qos, return -1, "Null param");
    d_assert(octet, return -1, "Null param");
    d_assert(octet->len == GTP_BEARER_QOS_LEN, return -1, "Null param");

    memset(bearer_qos, 0, sizeof(gtp_bearer_qos_t));

    bearer_qos->pci = source->pci;
    bearer_qos->pl = source->pl;
    bearer_qos->pvi = source->pvi;
    size++;

    bearer_qos->qci = source->qci;
    size++;

    bearer_qos->ul_mbr = core_buffer_to_uint64(&octet->data[size], 5);
    size += 5;
    bearer_qos->dl_mbr = core_buffer_to_uint64(&octet->data[size], 5);
    size += 5;
    bearer_qos->ul_gbr = core_buffer_to_uint64(&octet->data[size], 5);
    size += 5;
    bearer_qos->dl_gbr = core_buffer_to_uint64(&octet->data[size], 5);
    size += 5;

    d_assert(size == octet->len, return -1, "decode error");
    
    return size;
}
c_int16_t gtp_build_bearer_qos(
    tlv_octet_t *octet, gtp_bearer_qos_t *bearer_qos, void *data, int data_len)
{
    gtp_bearer_qos_t target;
    c_int16_t size = 0;

    d_assert(bearer_qos, return -1, "Null param");
    d_assert(octet, return -1, "Null param");
    d_assert(data, return -1, "Null param");
    d_assert(data_len >= GTP_BEARER_QOS_LEN, return -1, "Null param");

    octet->data = data;
    memcpy(&target, bearer_qos, sizeof(gtp_bearer_qos_t));

    memcpy(&octet->data[size], &target, 2);
    size += 2;

    core_uint64_to_buffer(target.ul_mbr, 5, &octet->data[size]);
    size += 5;
    core_uint64_to_buffer(target.dl_mbr, 5, &octet->data[size]);
    size += 5;
    core_uint64_to_buffer(target.ul_gbr, 5, &octet->data[size]);
    size += 5;
    core_uint64_to_buffer(target.dl_gbr, 5, &octet->data[size]);
    size += 5;

    octet->len = size;

    return octet->len;
}

/* 8.21 User Location Information (ULI) */
c_int16_t gtp_parse_uli(gtp_uli_t *uli, tlv_octet_t *octet)
{
    gtp_uli_t *source = (gtp_uli_t *)octet->data;
    c_int16_t size = 0;

    d_assert(uli, return -1, "Null param");
    d_assert(octet, return -1, "Null param");

    memset(uli, 0, sizeof(gtp_uli_t));

    uli->flags = source->flags;
    size++;

    if (uli->flags.cgi)
    {
        d_assert(size + sizeof(uli->cgi) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->cgi, &octet->data[size], sizeof(uli->cgi));
        uli->cgi.lac = ntohs(uli->cgi.lac);
        uli->cgi.ci = ntohs(uli->cgi.ci);
        size += sizeof(uli->cgi);
    }
    if (uli->flags.sai)
    {
        d_assert(size + sizeof(uli->sai) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->sai, &octet->data[size], sizeof(uli->sai));
        uli->sai.lac = ntohs(uli->sai.lac);
        uli->sai.sac = ntohs(uli->sai.sac);
        size += sizeof(uli->sai);
    }
    if (uli->flags.rai)
    {
        d_assert(size + sizeof(uli->rai) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->rai, &octet->data[size], sizeof(uli->rai));
        uli->rai.lac = ntohs(uli->rai.lac);
        uli->rai.rac = ntohs(uli->rai.rac);
        size += sizeof(uli->rai);
    }
    if (uli->flags.tai)
    {
        d_assert(size + sizeof(uli->tai) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->tai, &octet->data[size], sizeof(uli->tai));
        uli->tai.tac = ntohs(uli->tai.tac);
        size += sizeof(uli->tai);
    }
    if (uli->flags.ecgi)
    {
        d_assert(size + sizeof(uli->ecgi) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->ecgi, &octet->data[size], sizeof(uli->ecgi));
        uli->ecgi.eci = ntohl(uli->ecgi.eci);
        size += sizeof(uli->ecgi);
    }
    if (uli->flags.lai)
    {
        d_assert(size + sizeof(uli->lai) <= octet->len, 
                return -1, "decode error");
        memcpy(&uli->lai, &octet->data[size], sizeof(uli->lai));
        uli->lai.lac = ntohs(uli->lai.lac);
        size += sizeof(uli->lai);
    }

    d_assert(size == octet->len, return -1, "decode error");
    
    return size;
}
c_int16_t gtp_build_uli(
        tlv_octet_t *octet, gtp_uli_t *uli, void *data, int data_len)
{
    gtp_uli_t target;
    c_int16_t size = 0;

    d_assert(uli, return -1, "Null param");
    d_assert(octet, return -1, "Null param");
    d_assert(data, return -1, "Null param");
    d_assert(data_len, return -1, "Null param");

    octet->data = data;
    memcpy(&target, uli, sizeof(gtp_uli_t));

    d_assert(size + sizeof(target.flags) <= data_len, 
            return -1, "encode error");
    memcpy(&octet->data[size], &target.flags, sizeof(target.flags));
    size += sizeof(target.flags);

    if (target.flags.cgi)
    {
        d_assert(size + sizeof(target.cgi) <= data_len, 
                return -1, "encode error");
        target.cgi.lac = htons(target.cgi.lac);
        target.cgi.ci = htons(target.cgi.ci);
        memcpy(&octet->data[size], &target.cgi, sizeof(target.cgi));
        size += sizeof(target.cgi);
    }
    if (target.flags.sai)
    {
        d_assert(size + sizeof(target.sai) <= data_len, 
                return -1, "encode error");
        target.sai.lac = htons(target.sai.lac);
        target.sai.sac = htons(target.sai.sac);
        memcpy(&octet->data[size], &target.sai, sizeof(target.sai));
        size += sizeof(target.sai);
    }
    if (target.flags.rai)
    {
        d_assert(size + sizeof(target.rai) <= data_len, 
                return -1, "encode error");
        target.rai.lac = htons(target.rai.lac);
        target.rai.rac = htons(target.rai.rac);
        memcpy(&octet->data[size], &target.rai, sizeof(target.rai));
        size += sizeof(target.rai);
    }
    if (target.flags.tai)
    {
        d_assert(size + sizeof(target.tai) <= data_len, 
                return -1, "encode error");
        target.tai.tac = htons(target.tai.tac);
        memcpy(&octet->data[size], &target.tai, sizeof(target.tai));
        size += sizeof(target.tai);
    }
    if (target.flags.ecgi)
    {
        d_assert(size + sizeof(target.ecgi) <= data_len, 
                return -1, "encode error");
        target.ecgi.eci = htonl(target.ecgi.eci);
        memcpy(&octet->data[size], &target.ecgi, sizeof(target.ecgi));
        size += sizeof(target.ecgi);
    }
    if (target.flags.lai)
    {
        d_assert(size + sizeof(target.lai) <= data_len, 
                return -1, "encode error");
        target.lai.lac = htons(target.lai.lac);
        memcpy(&octet->data[size], &target.lai, sizeof(target.lai));
        size += sizeof(target.lai);
    }

    octet->len = size;

    return octet->len;
}
