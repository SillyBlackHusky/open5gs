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

ogs_pkbuf_t *ogs_gtp_build_err_ind(uint8_t qfi, uint32_t teid)
{
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_gtp_header_t *gtp_h = NULL;
    ogs_gtp_extension_header_t *ext_h = NULL;

    pkbuf = ogs_pkbuf_alloc(NULL,
            100 /* enough for ERR_IND; use smaller buffer */);
    ogs_assert(pkbuf);
    ogs_pkbuf_put(pkbuf, 100);
    memset(pkbuf->data, 0, pkbuf->len);

    gtp_h = (ogs_gtp_header_t *)pkbuf->data;

    /* Add GTP-U header */
    if (qfi) {
        /* Bits    8  7  6  5  4  3  2  1
         *        +--+--+--+--+--+--+--+--+
         *        |version |PT| 1| E| S|PN|
         *        +--+--+--+--+--+--+--+--+
         *         0  0  1   1  0  1  0  0
         */
        gtp_h->flags = 0x34;
        gtp_h->type = OGS_GTPU_MSGTYPE_ERR_IND;
        gtp_h->teid = htobe32(teid);

        ext_h = (ogs_gtp_extension_header_t *)(
                pkbuf->data + OGS_GTPV1U_HEADER_LEN);
        ext_h->type = OGS_GTP_EXTENSION_HEADER_TYPE_PDU_SESSION_CONTAINER;
        ext_h->len = 1;
        ext_h->pdu_type =
            OGS_GTP_EXTENSION_HEADER_PDU_TYPE_DL_PDU_SESSION_INFORMATION;
        ext_h->qos_flow_identifier = qfi;
        ext_h->next_type =
            OGS_GTP_EXTENSION_HEADER_TYPE_NO_MORE_EXTENSION_HEADERS;
    } else {
        /* Bits    8  7  6  5  4  3  2  1
         *        +--+--+--+--+--+--+--+--+
         *        |version |PT| 1| E| S|PN|
         *        +--+--+--+--+--+--+--+--+
         *         0  0  1   1  0  0  0  0
         */
        gtp_h->flags = 0x30;
        gtp_h->type = OGS_GTPU_MSGTYPE_ERR_IND;
        gtp_h->teid = htobe32(teid);
    }

    return pkbuf;
}
