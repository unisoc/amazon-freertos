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
#ifndef _AWS_TLS_TEST_H_
#define _AWS_TLS_TEST_H_

/*
* PEM-encoded client certificate.
*
* Certificate for P-256 elliptic curve key.
*/

static const char tlstestCLIENT_CERTIFICATE_PEM_EC[] = "-----BEGIN CERTIFICATE-----\n"
"MIIC9jCCAd6gAwIBAgIVAMhrfBctky+K+ZXh8cGZQceDBYjVMA0GCSqGSIb3DQEB\n"
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xOTAzMjQxMDMw\n"
"NTdaFw00OTEyMzEyMzU5NTlaMIGEMQswCQYDVQQGEwJDTjEQMA4GA1UECAwHVGlh\n"
"bmppbjEQMA4GA1UEBwwHVGlhbmppbjEPMA0GA1UECgwGdW5pc29jMQwwCgYDVQQL\n"
"DAN3Y24xDzANBgNVBAMMBmNhaXh1ZTEhMB8GCSqGSIb3DQEJARYSeHVlLmNhaUB1\n"
"bmlzb2MuY29tMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEqGtFiBzeCj8qOeCZ\n"
"hw3LjS2qbGUpk7kIYM4IzvF20xLEsh8HReU0sbspxQRvs062mkS9RBb6+h5dR36A\n"
"Ekx1daNgMF4wHwYDVR0jBBgwFoAUPWdhLvuC3n2QU0+AaWUZphboBIUwHQYDVR0O\n"
"BBYEFE4e8gtDxeMjy2fH+A1dKxPSv6TxMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/\n"
"BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBciPNtkNmQ6vUX6khtk9GkiRq8JdYd\n"
"0RMHlqNlq7ZYxEu2OFIMGIT40th5DtyB8Nc56sjpATMbx8vG0KKoes7M0Kr7xWPC\n"
"pnjgJA01cnHLDBN5xU5zW2sieleFLD58NBz6A5OBxvHeJp9Kf0FU9BA9BmhHkwHo\n"
"93Pewe+LytRmjJIZnLBMJZ8s0PSQNN4hfbxmnH0ys+jhyDvSSqUbEaGq8NEWxVTe\n"
"MFSM5/961O37tkEBZfW5BZ1RdgdRNQv6GiaQMypI2UHLRJ2lAXuL7GyNPW3A5b41\n"
"y2m1iObz5jKZMh/+nCQ147OgELPVXQKe5ZV5Bb1BJ6fOL8d6RPx7iq9i\n"
"-----END CERTIFICATE-----";

/*
* PEM-encoded client private key.
*
* This is a P-256 elliptic curve key.
*/

static const char tlstestCLIENT_PRIVATE_KEY_PEM_EC[] = "-----BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIA3Jvm6Wo9R2PoPD2gaPV6kxGCBHPS7hJNX7O8KqDOZMoAoGCCqGSM49\n"
"AwEHoUQDQgAEqGtFiBzeCj8qOeCZhw3LjS2qbGUpk7kIYM4IzvF20xLEsh8HReU0\n"
"sbspxQRvs062mkS9RBb6+h5dR36AEkx1dQ==\n"
"-----END EC PRIVATE KEY-----";

/* One character of this certificate has been changed in the issuer
 * name from Amazon Web Services to Amazon Web Cervices. */
static const char tlstestCLIENT_CERTIFICATE_PEM_MALFORMED[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUWdsb996hKfpiA0zTKRYYZf0LZXowDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBDZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDMyNDA5NTg1\n"
"NFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMQtGaajVpsm6Njs6Doh\n"
"Cd4H1NwiqZsIN+SUQEX++Ncfnjp4aju8GRNiPeNOXRhSA11oA5BMh3IUyBMDMq9f\n"
"TNT/B7MRTsqgMsrwPnmh2CN8uRHg4He6fEg/L5ODwhsmF12YRlFKJHXqBaiePtLu\n"
"sy23l645VgIgx76Y9ibS/12pi/uQfLx1FyXDutmOWjzXU6YcLnJ0KV+wXLpYqxVA\n"
"YXwWTCx8rEuxqMAvgiPztvR9rTrU2wMvGCQnWtw/9yRO4t8yJrZM0IncdXqrVDub\n"
"bQbKhzyoYUYhgZzSj/qTX7a9/bsrBhUqea4SDmdKRFyfMX9dM8QA6adqqvx3pOtF\n"
"9J8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUMxPFq1ezNaqhhXMGoLV0MbNCVAIwHQYD\n"
"VR0OBBYEFMydI9/zHorE8+TsB+3tTXaLlsAcMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAeO0CO0v3Z9wG/MZio92gZLFhJ\n"
"xcdN96btKkR/81pQswltYjd2N78n6+bpcvRxXAYc5LsUVx6sJpalibj4hwxCADPC\n"
"0wzED+OiW14R6O3XEX+O6HJk9RfKLjR9+VbZIVs6ZvBmt6Yy1mz+Wg58ojAWlhLc\n"
"ns8zQBpiwoKETkj3sWPUktU0UyWuTNW0Pi5ZA/AKEN1ZYfPw570brebC9winV8MU\n"
"TfXHuOe8CBgFVzmot2IjNGo0HLPFTBkZCRvTgjprHd7nyQX/TYHksqHzuM1SF9dH\n"
"wbxoNYIAbiYETyGc93uejk3Zj6ZWQyZ4n+UPhDvh8B4COkPPgsnFn6SwsM2u\n"
"-----END CERTIFICATE-----";


/* Certificate which is not trusted by the broker. */
static const char tlstestCLIENT_UNTRUSTED_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIBnTCCAUMCCQDnSd5ZLPDMQDAKBggqhkjOPQQDAjBWMQswCQYDVQQGEwJBVTET\n"
"MBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQ\n"
"dHkgTHRkMQ8wDQYDVQQDDAZjYWl4dWUwHhcNMTkwMzI2MDIzNDM5WhcNMjAwODA3\n"
"MDIzNDM5WjBXMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8G\n"
"A1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRAwDgYDVQQDDAdpdnk1NjYx\n"
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE2r/TJ8xZSU0zt4IE8jcL+EFRbG2l\n"
"CK7qf3OIfOThaJ0PHQ0PSuISKr8NnhC5drGtI4U1TKL4tPt44spD5EPqpTAKBggq\n"
"hkjOPQQDAgNIADBFAiBKnD+ZO8g06dEtcd+1p3xR6l2HMY99z+g6iILJTGjYxgIh\n"
"ALBAt5DUxKYefRhvixK0r35/xqmz+eZebihe4C9jvF6E\n"
"-----END CERTIFICATE-----";

/* Private key corresponding to the untrusted certificate. */
static const char tlstestCLIENT_UNTRUSTED_PRIVATE_KEY_PEM[] =
"-----BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIIId4OqUE/2VipbS5YoEo5rNOAZ91T2KfLlYtbiN3KC/oAoGCCqGSM49\n"
"AwEHoUQDQgAE2r/TJ8xZSU0zt4IE8jcL+EFRbG2lCK7qf3OIfOThaJ0PHQ0PSuIS\n"
"Kr8NnhC5drGtI4U1TKL4tPt44spD5EPqpQ==\n"
"-----END EC PRIVATE KEY-----";

/* Device certificate created using BYOC instructions. */
static const char tlstestCLIENT_BYOC_CERTIFICATE_PEM[] =
    "-----BEGIN CERTIFICATE-----\n"
"MIIBnTCCAUMCCQDCPDxiUQ0zBTAKBggqhkjOPQQDAjBWMQswCQYDVQQGEwJBVTET\n"
"MBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQ\n"
"dHkgTHRkMQ8wDQYDVQQDDAZjYWl4dWUwHhcNMTkwMzI2MDIwOTM4WhcNMjAwODA3\n"
"MDIwOTM4WjBXMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8G\n"
"A1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRAwDgYDVQQDDAdpdnk1NjYx\n"
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEScb1m+JFmzDj1Q0QCkWFe5PpfFx7\n"
"9s1+j90ZHiFAiqsWDf17+FN4PgA8fuTjnmbOEMcRJVCZn5RmBKgYdJsUsTAKBggq\n"
"hkjOPQQDAgNIADBFAiEAvohCg6W+BMxGjBCBASWr9XcUISYROXJdS3spQEPspBMC\n"
"IAmGXCouFRRSj1GxuGQm2LV+U/d9KWbUNUagxypFUuC5\n"
"-----END CERTIFICATE-----";

/* Device private key created using BYOC instructions. */
static const char tlstestCLIENT_BYOC_PRIVATE_KEY_PEM[] =
"-----BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIMC+dX1rcZqYTH4Eu1VjVUr+e/Wl0S3N979hA2/aoxILoAoGCCqGSM49\n"
"AwEHoUQDQgAEScb1m+JFmzDj1Q0QCkWFe5PpfFx79s1+j90ZHiFAiqsWDf17+FN4\n"
"PgA8fuTjnmbOEMcRJVCZn5RmBKgYdJsUsQ==\n"
"-----END EC PRIVATE KEY-----";

#endif
