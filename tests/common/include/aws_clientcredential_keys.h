#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H

#include <stdint.h>

/*
 * PEM-encoded client certificate
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 */

#if 1 //iot caixue
#define keyCLIENT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\n"\
"MIIDWTCCAkGgAwIBAgIUWdsb996hKfpiA0zTKRYYZf0LZXowDQYJKoZIhvcNAQEL\n"\
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"\
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDMyNDA5NTg1\n"\
"NFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"\
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMQtGaajVpsm6Njs6Doh\n"\
"Cd4H1NwiqZsIN+SUQEX++Ncfnjp4aju8GRNiPeNOXRhSA11oA5BMh3IUyBMDMq9f\n"\
"TNT/B7MRTsqgMsrwPnmh2CN8uRHg4He6fEg/L5ODwhsmF12YRlFKJHXqBaiePtLu\n"\
"sy23l645VgIgx76Y9ibS/12pi/uQfLx1FyXDutmOWjzXU6YcLnJ0KV+wXLpYqxVA\n"\
"YXwWTCx8rEuxqMAvgiPztvR9rTrU2wMvGCQnWtw/9yRO4t8yJrZM0IncdXqrVDub\n"\
"bQbKhzyoYUYhgZzSj/qTX7a9/bsrBhUqea4SDmdKRFyfMX9dM8QA6adqqvx3pOtF\n"\
"9J8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUMxPFq1ezNaqhhXMGoLV0MbNCVAIwHQYD\n"\
"VR0OBBYEFMydI9/zHorE8+TsB+3tTXaLlsAcMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"\
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAeO0CO0v3Z9wG/MZio92gZLFhJ\n"\
"xcdN96btKkR/81pQswltYjd2N78n6+bpcvRxXAYc5LsUVx6sJpalibj4hwxCADPC\n"\
"0wzED+OiW14R6O3XEX+O6HJk9RfKLjR9+VbZIVs6ZvBmt6Yy1mz+Wg58ojAWlhLc\n"\
"ns8zQBpiwoKETkj3sWPUktU0UyWuTNW0Pi5ZA/AKEN1ZYfPw570brebC9winV8MU\n"\
"TfXHuOe8CBgFVzmot2IjNGo0HLPFTBkZCRvTgjprHd7nyQX/TYHksqHzuM1SF9dH\n"\
"wbxoNYIAbiYETyGc93uejk3Zj6ZWQyZ4n+UPhDvh8B4COkPPgsnFn6SwsM2u\n"\
"-----END CERTIFICATE-----";
#else //tls_echo_server
#define keyCLIENT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\n"\
"MIIDjjCCAnYCCQDXxSaNOZfa1zANBgkqhkiG9w0BAQsFADCBiDELMAkGA1UEBhMC\n"\
"VVMxCzAJBgNVBAgMAldBMQ4wDAYDVQQHDAVQbGFjZTEUMBIGA1UECgwLWW91ckNv\n"\
"bXBhbnkxCzAJBgNVBAsMAklUMRYwFAYDVQQDDA13d3cueW91cnMuY29tMSEwHwYJ\n"\
"KoZIhvcNAQkBFhJ4dWUuY2FpQHVuaXNvYy5jb20wHhcNMTkwMzIyMDI0ODI4WhcN\n"\
"MjAwMzIxMDI0ODI4WjCBiDELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAldBMQ4wDAYD\n"\
"VQQHDAVQbGFjZTEUMBIGA1UECgwLWW91ckNvbXBhbnkxCzAJBgNVBAsMAklUMRYw\n"\
"FAYDVQQDDA13d3cueW91cnMuY29tMSEwHwYJKoZIhvcNAQkBFhJ4dWUuY2FpQHVu\n"\
"aXNvYy5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDQNQpwWuEa\n"\
"4KqAxOrQMtkBqvgnDsuzmVSCZbYlM63PC2mTNSGktQxqiwBeL56Xs/TEf6ElLf/X\n"\
"JHMBGjW1AEZV9nNl88A6WCfA8UqlnxBVnxia9+VYZjVBSbmnWfklVSzAkH+ZECgC\n"\
"MwDQlOp5+fam+vRXJ/MgyaLIsyl+Pou2J6wYRccEf4QDcHBbw2p8m3bfAFvtZhy9\n"\
"foc2VYgeJNorB3wkucRbm5hgHxwJ2pRongh8JLyxf9bthhYb8EKr9+8G0xV6cXRh\n"\
"/iEV9fCRRjBYgxSXg228nG6E3NdReVxZevFQn51DkMXINX4aHCmlcwkSOrj8AOQv\n"\
"WK9na9v+sEfFAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAK2+AJzA2Rl3XdLxHyZl\n"\
"xJMYVH7Wo9w91yYNmQdLbG1hRPZce9teiBOfR2eacCl205chb5rjlREQ3iOG2Ej8\n"\
"pY6LDld7bVfldAVnYUflOKmLlp8mfkLEjK8rX3I/QLiPTDSyel4WPrlkmu6OrKou\n"\
"sY/rb5Ocvf8X1fLo2CSFmz+3V1N4zKW1iwJdc0QVfvS4QgOqpYUWDU+a6Af+sh6u\n"\
"o3KeKPK8/a9mTSdUBAr5xPpviCMq8vp5lw+4aDakHMTsHOUHEiNUANW/1r1vtCIm\n"\
"rBKEyBCNzu34qMBwG1+NFLuNYbyEk9i2RSkx8o1LVKALCTuLwMSlZrlz0w2LJswK\n"\
"ZUE=\n"\
"-----END CERTIFICATE-----";
#endif
/*
 * PEM-encoded issuer certificate for AWS IoT Just In Time Registration (JITR).
 * This is required if you're using JITR, since the issuer (Certificate 
 * Authority) of the client certificate is used by the server for routing the 
 * device's initial request. (The device client certificate must always be 
 * sent as well.) For more information about JITR, see:
 *  https://docs.aws.amazon.com/iot/latest/developerguide/jit-provisioning.html, 
 *  https://aws.amazon.com/blogs/iot/just-in-time-registration-of-device-certificates-on-aws-iot/.
 *
 * If you're not using JITR, set below to NULL.
 * 
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 */
#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  NULL

/*
 * PEM-encoded client private key.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----\n"
 */

#if 1 //iot caixue
#define keyCLIENT_PRIVATE_KEY_PEM "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEowIBAAKCAQEAxC0ZpqNWmybo2OzoOiEJ3gfU3CKpmwg35JRARf741x+eOnhq\n"\
"O7wZE2I9405dGFIDXWgDkEyHchTIEwMyr19M1P8HsxFOyqAyyvA+eaHYI3y5EeDg\n"\
"d7p8SD8vk4PCGyYXXZhGUUokdeoFqJ4+0u6zLbeXrjlWAiDHvpj2JtL/XamL+5B8\n"\
"vHUXJcO62Y5aPNdTphwucnQpX7BculirFUBhfBZMLHysS7GowC+CI/O29H2tOtTb\n"\
"Ay8YJCda3D/3JE7i3zImtkzQidx1eqtUO5ttBsqHPKhhRiGBnNKP+pNftr39uysG\n"\
"FSp5rhIOZ0pEXJ8xf10zxADpp2qq/Hek60X0nwIDAQABAoIBAQCWE0qESICAwQtw\n"\
"qAwgLBY4XRViMsI9b9QScbeZ1KzZJb8OTAA5InSsd4OQ2OovKM6aLnBLe8BCbdOB\n"\
"d/jWYLmOWGuzeZIlQNial5+zvTR0MX4DR11xOqDJRE1JNIrR6hoF5/AjT+0Ep7sc\n"\
"e7Lj+ufoGYNYLVS23AzyqmHk4ZWqOCkMpKK3yrvtWTZudjkL4g5FeJZsjvNw1mXQ\n"\
"1ZBQa03zETArPIwnGxIg0zsn9gegI69B0qMzMs60tc0Iq//BNPg+1QAw24VaURrR\n"\
"YwwOmW/hSG7gfOJLf8v+BT7be1eH2BfHyCmWcEiBN2H8vxAxqWnwGSti/BghNcWj\n"\
"jtsa/tPBAoGBAP0MMTXYvbp37q+0Y8qgMdmQ5HbVoEpE+hfcwiroK/ZKtUC53xs8\n"\
"qM+6QWMbZQvPknXJxVWB/GqK1tavPs7db1mhjUTkikQTz+3QPsf2jOaOBPQNFJZ4\n"\
"BISNPOXxO20LZHN82kZ6YDYSwzwfI7kH2b2oBLBwhbW3T4tk+OVVzGhDAoGBAMZ3\n"\
"Cwy9Kvx6vq4NXlGsmjuBOlo/Ijg82i6bWsYULy7eDT9DKnUj6Y0bJOuMKpdCmVvE\n"\
"33UipJiQbWM38l4rubPeHNHG1Zfo7ITgEct0X19878/XfOcSEzptU5YuIx1r/rwe\n"\
"3LCUU5AG/HQlGMs3EOHMds4ZZZPyUo+socNANpp1AoGAKMoCVK+vRZN3ghMN0uen\n"\
"oW5dpVoiEFa2CYqoMcpi67x3IwvTDszkWTeoupT74L+A2EPB71qYmYPvxYNSGH5R\n"\
"KJ2F7Aj60KZw6eCbfhgb9j4GRmClZOvLrqIlotsTvSwM8SC6/olTYOP9XH+Yfobp\n"\
"bkzbFKXOp8/4nnuUM0N1Q4kCgYB8ISy6FP7z11m3xwi9ELWOji8PDvm6LuVPzT8j\n"\
"MLCqIZwrezjs9z82UEnxQUPKQ49jxGj2/GnyOjMQoYdnuvV1OHDSDAi/8KEuIm8+\n"\
"z1Rvhtb/KgonBDkejA6Bme3AkzFKPAtQLhVtudDgW+t7/r8CWz3bfir4M9HLpu+u\n"\
"DVWZmQKBgE9RgKna65B8w1ul2i59dXFmjipW64EbscQFpF2o0oy5v43WprlGC8MP\n"\
"bdKN6BKGR6bwDY6tRPYFehsl5cbvV7HdIN/lwUp9y68glI9OshyWFtqtnAM/0Lcc\n"\
"7jS+63c3KM01lypad7qfvGaofN7olXsV2Fmfq4FLvCcgi5pqMpEF\n"\
"-----END RSA PRIVATE KEY-----";
#else //tls_echo_server
#define keyCLIENT_PRIVATE_KEY_PEM "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEpQIBAAKCAQEA0DUKcFrhGuCqgMTq0DLZAar4Jw7Ls5lUgmW2JTOtzwtpkzUh\n"\
"pLUMaosAXi+el7P0xH+hJS3/1yRzARo1tQBGVfZzZfPAOlgnwPFKpZ8QVZ8Ymvfl\n"\
"WGY1QUm5p1n5JVUswJB/mRAoAjMA0JTqefn2pvr0VyfzIMmiyLMpfj6LtiesGEXH\n"\
"BH+EA3BwW8NqfJt23wBb7WYcvX6HNlWIHiTaKwd8JLnEW5uYYB8cCdqUaJ4IfCS8\n"\
"sX/W7YYWG/BCq/fvBtMVenF0Yf4hFfXwkUYwWIMUl4NtvJxuhNzXUXlcWXrxUJ+d\n"\
"Q5DFyDV+GhwppXMJEjq4/ADkL1ivZ2vb/rBHxQIDAQABAoIBAFzHhgNTPARSISiI\n"\
"l2p3fiQ0H6in+zXpGbORx30EbmtY1o72mitfUk0X6+4MoktPsb7ucMy0ltE0ONtA\n"\
"5rXljXI5BGtmFz7oka5016K+IvVElB5aYSTnRsv1InnkqhVswyh8O5/lhy6Ts8U+\n"\
"1s3MCZGXOtKrxrfRomx1CgRrsq5UaWbLKNg/iXGDErnZ3Ezm3dPXsVQieNUZ3UBD\n"\
"Wyrq1U2jAQofZ6r+Aa8z4Zr6D2PyBGOVWgRYS++xRydwG/4ZHGBqg5P/4FXsAwWd\n"\
"M1/ybenbcR/6swV+CotJJlSIDCiMnJEna7Y1WCvhWqLaq2paoAB6mCvRYuEfSXg5\n"\
"1mU6HnECgYEA8maqcZGImDxTvPJYjhu00abgWyD/tVZIuTHWV0IBnUm14Ep0/lyg\n"\
"tW2IYFas+cmDNNSWD9L+jo7gR6Pvlk8mu9fKdPZIx6GsJva462sJYGBk442qcaFY\n"\
"6D4XcctdQmq6qABbJo0uakE21bYIf6FMD9BRjLe+4+bu66Fz0svfupMCgYEA2+NJ\n"\
"f5DV4E2Bt5yABT0Su0keGXMhLgH1KWjy8Os7iPbYXsMEfGlQQrsjrqGUP+3hbLGJ\n"\
"BBe+ZXAHVCkLsDNBzYVM03EWyhVzqjuRX+tQkm4PcExmZ8kQro2RPiHycIjiZMDA\n"\
"efWGT+AdADlAqn+GAruGJblAb0XDri2ohgiO80cCgYEAnD10axNFGer4hncmgSJ2\n"\
"oBOR1OsmJrng0g2/fCq9Y6ZZJqKXWkRWCdr55i63+6DZgznrJ3P2Q+jvwv11tkp6\n"\
"SuVCR7VfH92jWtH4SzWIq6g2Hz32uJso9TjgX6aXC/JDMXpZRjYFztMOBx33yFws\n"\
"7Jx3k93zmm4FXbeJMjvajyUCgYEAg8Hvupao8Zy/y6vlMj5kSQEJHirUiOL3anbO\n"\
"i/oIVIvFHxMwTvQz+ah1OyDq2lvKJf1DosYvekzkt1NZA53TIjlrzY8IHtM32ZuU\n"\
"AqvYPcXhTmMZQbtDWbTOgTKKOVBsh/7P1sR+VMJv3kBwNTnwrRZ8zYHx0Ds3g9ks\n"\
"EBR0w+UCgYEAll2gJjx2cMyhruniSaW4dEc5wur49J7du8ybykamAvMdgB+EmCrM\n"\
"dOrp03pABjbL0HpFFKbWppOTEyVM/tLJ62rM0NAh8/P0wGLsja1+SOpHYXgkEtko\n"\
"th0MYlZlB6xUG+wu3ZO/XjzLpEHjf6/Hl1+ePzP82BBu+v2Za6kwV28=\n"\
"-----END RSA PRIVATE KEY-----";
#endif
/* The constants above are set to const char * pointers defined in aws_dev_mode_key_provisioning.c,
 * and externed here for use in C files.  NOTE!  THIS IS DONE FOR CONVENIENCE
 * DURING AN EVALUATION PHASE AND IS NOT GOOD PRACTICE FOR PRODUCTION SYSTEMS 
 * WHICH MUST STORE KEYS SECURELY. */
extern const char clientcredentialCLIENT_CERTIFICATE_PEM[];
extern const char *clientcredentialJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM;
extern const char clientcredentialCLIENT_PRIVATE_KEY_PEM[];
extern const uint32_t clientcredentialCLIENT_CERTIFICATE_LENGTH;
extern const uint32_t clientcredentialCLIENT_PRIVATE_KEY_LENGTH;

#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
