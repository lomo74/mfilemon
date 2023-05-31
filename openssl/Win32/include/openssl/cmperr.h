/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2023 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_CMPERR_H
# define OPENSSL_CMPERR_H
# pragma once

# include <openssl/opensslconf.h>
# include <openssl/symhacks.h>
# include <openssl/cryptoerr_legacy.h>


# ifndef OPENSSL_NO_CMP


/*
 * CMP reason codes.
 */
#  define CMP_R_ALGORITHM_NOT_SUPPORTED                    139
#  define CMP_R_BAD_CHECKAFTER_IN_POLLREP                  167
#  define CMP_R_BAD_REQUEST_ID                             108
#  define CMP_R_CERTHASH_UNMATCHED                         156
#  define CMP_R_CERTID_NOT_FOUND                           109
#  define CMP_R_CERTIFICATE_NOT_ACCEPTED                   169
#  define CMP_R_CERTIFICATE_NOT_FOUND                      112
#  define CMP_R_CERTREQMSG_NOT_FOUND                       157
#  define CMP_R_CERTRESPONSE_NOT_FOUND                     113
#  define CMP_R_CERT_AND_KEY_DO_NOT_MATCH                  114
#  define CMP_R_CHECKAFTER_OUT_OF_RANGE                    181
#  define CMP_R_ENCOUNTERED_KEYUPDATEWARNING               176
#  define CMP_R_ENCOUNTERED_WAITING                        162
#  define CMP_R_ERROR_CALCULATING_PROTECTION               115
#  define CMP_R_ERROR_CREATING_CERTCONF                    116
#  define CMP_R_ERROR_CREATING_CERTREP                     117
#  define CMP_R_ERROR_CREATING_CERTREQ                     163
#  define CMP_R_ERROR_CREATING_ERROR                       118
#  define CMP_R_ERROR_CREATING_GENM                        119
#  define CMP_R_ERROR_CREATING_GENP                        120
#  define CMP_R_ERROR_CREATING_PKICONF                     122
#  define CMP_R_ERROR_CREATING_POLLREP                     123
#  define CMP_R_ERROR_CREATING_POLLREQ                     124
#  define CMP_R_ERROR_CREATING_RP                          125
#  define CMP_R_ERROR_CREATING_RR                          126
#  define CMP_R_ERROR_PARSING_PKISTATUS                    107
#  define CMP_R_ERROR_PROCESSING_MESSAGE                   158
#  define CMP_R_ERROR_PROTECTING_MESSAGE                   127
#  define CMP_R_ERROR_SETTING_CERTHASH                     128
#  define CMP_R_ERROR_UNEXPECTED_CERTCONF                  160
#  define CMP_R_ERROR_VALIDATING_PROTECTION                140
#  define CMP_R_ERROR_VALIDATING_SIGNATURE                 171
#  define CMP_R_FAILED_BUILDING_OWN_CHAIN                  164
#  define CMP_R_FAILED_EXTRACTING_PUBKEY                   141
#  define CMP_R_FAILURE_OBTAINING_RANDOM                   110
#  define CMP_R_FAIL_INFO_OUT_OF_RANGE                     129
#  define CMP_R_INVALID_ARGS                               100
#  define CMP_R_INVALID_OPTION                             174
#  define CMP_R_MISSING_CERTID                             165
#  define CMP_R_MISSING_KEY_INPUT_FOR_CREATING_PROTECTION  130
#  define CMP_R_MISSING_KEY_USAGE_DIGITALSIGNATURE         142
#  define CMP_R_MISSING_P10CSR                             121
#  define CMP_R_MISSING_PBM_SECRET                         166
#  define CMP_R_MISSING_PRIVATE_KEY                        131
#  define CMP_R_MISSING_PRIVATE_KEY_FOR_POPO               190
#  define CMP_R_MISSING_PROTECTION                         143
#  define CMP_R_MISSING_PUBLIC_KEY                         183
#  define CMP_R_MISSING_REFERENCE_CERT                     168
#  define CMP_R_MISSING_SECRET                             178
#  define CMP_R_MISSING_SENDER_IDENTIFICATION              111
#  define CMP_R_MISSING_TRUST_ANCHOR                       179
#  define CMP_R_MISSING_TRUST_STORE                        144
#  define CMP_R_MULTIPLE_REQUESTS_NOT_SUPPORTED            161
#  define CMP_R_MULTIPLE_RESPONSES_NOT_SUPPORTED           170
#  define CMP_R_MULTIPLE_SAN_SOURCES                       102
#  define CMP_R_NO_STDIO                                   194
#  define CMP_R_NO_SUITABLE_SENDER_CERT                    145
#  define CMP_R_NULL_ARGUMENT                              103
#  define CMP_R_PKIBODY_ERROR                              146
#  define CMP_R_PKISTATUSINFO_NOT_FOUND                    132
#  define CMP_R_POLLING_FAILED                             172
#  define CMP_R_POTENTIALLY_INVALID_CERTIFICATE            147
#  define CMP_R_RECEIVED_ERROR                             180
#  define CMP_R_RECIPNONCE_UNMATCHED                       148
#  define CMP_R_REQUEST_NOT_ACCEPTED                       149
#  define CMP_R_REQUEST_REJECTED_BY_SERVER                 182
#  define CMP_R_SENDER_GENERALNAME_TYPE_NOT_SUPPORTED      150
#  define CMP_R_SRVCERT_DOES_NOT_VALIDATE_MSG              151
#  define CMP_R_TOTAL_TIMEOUT                              184
#  define CMP_R_TRANSACTIONID_UNMATCHED                    152
#  define CMP_R_TRANSFER_ERROR                             159
#  define CMP_R_UNEXPECTED_PKIBODY                         133
#  define CMP_R_UNEXPECTED_PKISTATUS                       185
#  define CMP_R_UNEXPECTED_PVNO                            153
#  define CMP_R_UNKNOWN_ALGORITHM_ID                       134
#  define CMP_R_UNKNOWN_CERT_TYPE                          135
#  define CMP_R_UNKNOWN_PKISTATUS                          186
#  define CMP_R_UNSUPPORTED_ALGORITHM                      136
#  define CMP_R_UNSUPPORTED_KEY_TYPE                       137
#  define CMP_R_UNSUPPORTED_PROTECTION_ALG_DHBASEDMAC      154
#  define CMP_R_VALUE_TOO_LARGE                            175
#  define CMP_R_VALUE_TOO_SMALL                            177
#  define CMP_R_WRONG_ALGORITHM_OID                        138
#  define CMP_R_WRONG_CERTID                               189
#  define CMP_R_WRONG_CERTID_IN_RP                         187
#  define CMP_R_WRONG_PBM_VALUE                            155
#  define CMP_R_WRONG_RP_COMPONENT_COUNT                   188
#  define CMP_R_WRONG_SERIAL_IN_RP                         173

# endif
#endif
