/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ogs-gtp.h"

ogs_pkbuf_t *ogs_gtp_build_echo_request(
        uint8_t type, uint8_t recovery, uint8_t features)
{
    ogs_gtp_message_t gtp_message;
    ogs_gtp_echo_request_t *req = NULL;

    req = &gtp_message.echo_request;
    memset(&gtp_message, 0, sizeof(ogs_gtp_message_t));

    req->recovery.presence = 1;
    req->recovery.u8 = recovery;

    req->sending_node_features.presence = 1;
    req->sending_node_features.u8 = features;

    gtp_message.h.type = type;
    return ogs_gtp_build_msg(&gtp_message);
}

ogs_pkbuf_t *ogs_gtp_build_echo_response(
        uint8_t type, uint8_t recovery, uint8_t features)
{
    ogs_gtp_message_t gtp_message;
    ogs_gtp_echo_response_t *rsp = NULL;

    rsp = &gtp_message.echo_response;
    memset(&gtp_message, 0, sizeof(ogs_gtp_message_t));

    rsp->recovery.presence = 1;
    rsp->recovery.u8 = recovery;

    rsp->sending_node_features.presence = 1;
    rsp->sending_node_features.u8 = features;

    gtp_message.h.type = type;
    return ogs_gtp_build_msg(&gtp_message);
}

ogs_pkbuf_t *ogs_gtp_build_error_indication(
        uint32_t teid, ogs_sockaddr_t *addr)
{
    ogs_pkbuf_t *pkbuf = NULL;
    int family;
    uint16_t len;

    unsigned char *p = NULL;
    uint16_t uint16 = 0;
    uint32_t uint32 = 0;

    ogs_assert(addr);

    pkbuf = ogs_pkbuf_alloc(
            NULL, 100 /* enough for Error Indiciation; use smaller buffer */);
    ogs_assert(pkbuf);
    ogs_pkbuf_reserve(pkbuf, OGS_GTPV1U_5GC_HEADER_LEN);
    ogs_pkbuf_put(pkbuf, 100-OGS_GTPV1U_5GC_HEADER_LEN);

    p = pkbuf->data;

    /*
     * 8.3 Tunnel Endpoint Identifier Data I
     *
     * Octet 1 : Type = 16 (Decimal)
     * Octet 2-5 : Tunnel Endpoint Identitifer Data I
     */
    *p++ = 16;

    uint32 = htobe32(teid);
    memcpy(p, &uint32, sizeof(uint32));
    p += sizeof(uint32);

    /*
     * 8.4 GTP-U Peer Address
     *
     * Octet 1 : Type = 133 (Decimal)
     * Octet 2-3 : Length
     * Octet 4-n : IPv4 or IPv6 Address
     */
    *p++ = 133;

    family = addr->ogs_sa_family;
    switch(family) {
    case AF_INET:
        len = OGS_IPV4_LEN;
        break;
    case AF_INET6:
        len = OGS_IPV6_LEN;
        break;
    default:
        ogs_fatal("Unknown family(%d)", family);
        ogs_abort();
        return NULL;
    }

    uint16 = htobe16(len);
    memcpy(p, &uint16, sizeof(uint16));
    p += sizeof(uint16);

    memcpy(p, &addr->sa, len);
    p += len;

    ogs_pkbuf_trim(pkbuf, p - pkbuf->data);

    return pkbuf;
}
