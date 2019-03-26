/*
 * Amazon FreeRTOS
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */
#ifndef AWS_TEST_TCP_H
#define AWS_TEST_TCP_H

/* Non-Encrypted Echo Server.
 * Update tcptestECHO_SERVER_ADDR# and
 * tcptestECHO_PORT with IP address
 * and port of unencrypted TCP echo server. */
#define tcptestECHO_SERVER_ADDR0         192
#define tcptestECHO_SERVER_ADDR1         168
#define tcptestECHO_SERVER_ADDR2         1
#define tcptestECHO_SERVER_ADDR3         101
#define tcptestECHO_PORT                 9001

/* Encrypted Echo Server.
 * If tcptestSECURE_SERVER is set to 1, the following must be updated:
 * 1. aws_clientcredential.h to use a valid AWS endpoint.
 * 2. aws_clientcredential_keys.h with corresponding AWS keys.
 * 3. tcptestECHO_SERVER_TLS_ADDR0-3 with the IP address of an
 * echo server using TLS.
 * 4. tcptestECHO_PORT_TLS, with the port number of the echo server
 * using TLS.
 * 5. tcptestECHO_HOST_ROOT_CA with the trusted root certificate of the
 * echo server using TLS. */
#define tcptestSECURE_SERVER             1

#define tcptestECHO_SERVER_TLS_ADDR0     192
#define tcptestECHO_SERVER_TLS_ADDR1     168
#define tcptestECHO_SERVER_TLS_ADDR2     1
#define tcptestECHO_SERVER_TLS_ADDR3     101
#define tcptestECHO_PORT_TLS             9000

/* Number of times to retry a connection if it fails. */
#define tcptestRETRY_CONNECTION_TIMES    6

/* The root certificate used for the encrypted echo server.
 * This certificate is self-signed, and not in the trusted catalog. */
static const char tcptestECHO_HOST_ROOT_CA[] = "-----BEGIN CERTIFICATE-----\n"
"MIID5TCCAs2gAwIBAgIJAMx1XBAkaJssMA0GCSqGSIb3DQEBCwUAMIGIMQswCQYD\n"
"VQQGEwJVUzELMAkGA1UECAwCV0ExDjAMBgNVBAcMBVBsYWNlMRQwEgYDVQQKDAtZ\n"
"b3VyQ29tcGFueTELMAkGA1UECwwCSVQxFjAUBgNVBAMMDXd3dy55b3Vycy5jb20x\n"
"ITAfBgkqhkiG9w0BCQEWEnh1ZS5jYWlAdW5pc29jLmNvbTAeFw0xOTAzMjIwMjI5\n"
"MjBaFw0yMDAzMjEwMjI5MjBaMIGIMQswCQYDVQQGEwJVUzELMAkGA1UECAwCV0Ex\n"
"DjAMBgNVBAcMBVBsYWNlMRQwEgYDVQQKDAtZb3VyQ29tcGFueTELMAkGA1UECwwC\n"
"SVQxFjAUBgNVBAMMDXd3dy55b3Vycy5jb20xITAfBgkqhkiG9w0BCQEWEnh1ZS5j\n"
"YWlAdW5pc29jLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMu5\n"
"K41UahwX0WpK3JNdezrzVE7VhWlX/pR4KblYpdtgaN6KL2hA0vLMhrKRLY0dteIT\n"
"/XEKr8RX14GeJ0jEBpB3kfbFDxOG00Tdj4Pa1tVLdbqoAHJUr6uFSKIVDYCwf5RK\n"
"UK2pXcFOhfkrKg4Eu7Eg4cgPQ4zzw84inDUsLxHc+rFqbhqIqGfSWhjbRwDJra0k\n"
"ENkIcKW7ML/Q+R24wF8diIKv25CzoaqeHW9B/ga50axb0m60oMe8DtCDrGOx2dXR\n"
"OlOhxxq/9mLMsPWi0wR08bzgciVjdT4JEnRtDtQoOCAYK8djfD1MaRUuEQvU11WB\n"
"ZNPkkV4hJRSR0HxfMIkCAwEAAaNQME4wHQYDVR0OBBYEFF15oO1cakRGDvvBtjSx\n"
"BoPWMFlyMB8GA1UdIwQYMBaAFF15oO1cakRGDvvBtjSxBoPWMFlyMAwGA1UdEwQF\n"
"MAMBAf8wDQYJKoZIhvcNAQELBQADggEBAL1YqIIZh1i9MEXqT1qlvjss07GbvvuA\n"
"tKLmm6pPMYm8uuCxE88d5tP1bI4ElSRk6lpvQlK1rIOAQHffOB53HCrVnX3j663W\n"
"fz1BDFhkCaKl5VY70Fmr85AcWDcOYJnTEGv2iusFbuS/4jx0WqlAx0vssYqdqTrO\n"
"OLiN21aVL1wHkj8YXT1V9EdqzUcioWFFLKVyIXcHQMfp0ZHpu7j5gPpdnHCobOUr\n"
"gpZ6XQziUBeeFEVsCj9PM5wfu4wVyLP1XqLClL8Xk2o2ikLz6MvU13TJTHeQSnQT\n"
"9jZDXhe7UbsElC8YO7D+sEPkk6OL48GwxowR7FJAAHwt0x38ElWywYA=\n"
"-----END CERTIFICATE-----";

#endif /* ifndef AWS_TEST_TCP_H */
