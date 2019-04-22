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

#if 1 /* used for OTA */
#define keyCLIENT_CERTIFICATE_PEM "-----BEGIN CERTIFICATE-----\n"\
"MIIDWjCCAkKgAwIBAgIVAMoxHqvpzJEho9TH5p/RG2Sa8bCKMA0GCSqGSIb3DQEB\n"\
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"\
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xOTAzMjgwODUx\n"\
"MzhaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"\
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDzror42EqPVW/H9NZ8\n"\
"UEuJ3RFWB36WEJXbyRvBIZmGcYkz5rGPFro5omNdU5usNylMqEJp5wkT4EF0SCp4\n"\
"T8a4K6hgV7jjn1mBYSspEa0NjDU/wNbdGq6K7VK5fNsiozFMK5daktxcriK7ni3q\n"\
"4/XtlOLd4Klszf+TlMZZ8CJoAJ6WE9TW58XUqWhxdvJAgj9PAAarCvLxXUQM6Bi6\n"\
"K0ZDOY22RuKSKJZLGOk//Q9E6JWFdXOCzWo8wJ9t5o122r8RGdK2zzP9b+QjZ8rD\n"\
"XIdwFLT2w8EgNKmO+TKYKfmTdw4ljBWl3NzYBNpiR5fOdcGNuIGbrwbfpl3dq24C\n"\
"/AqfAgMBAAGjYDBeMB8GA1UdIwQYMBaAFKfKMHEn1eZasCkL28yq//CZicvZMB0G\n"\
"A1UdDgQWBBT/BrWXVCgj7J40dSAnXORD9ecbkjAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"\
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAGSUzdgG1hyhIn+a6vAEqfDH/\n"\
"1uWOonEQmc1V9UxGrJf54enr065tNmYyesiWb/RM5RyKKBUb17cI6ZXzixPLs8qr\n"\
"K02reTtJ2EpB2mmVXF9yBQpp9xX2+FTH2Y/hnXrA+4BNYiZl/5myRJXaFFvNgscG\n"\
"w4vh9WyW6zUIL10fBUde8/hmXmcys6fVfkGtB88biGk1TD5NOffdv3ZKen2JQiRA\n"\
"qtQww9iie2KevCzXIhVVL0efJAs+DEg4Dowi2fS+CAbICZQJmg/MxgnHCJtI737V\n"\
"Ir9oxW6Txb1fnxLJMa/ZXuIalvpsynqnziKr+5k1QrINMNldOXZ7wPkhc2CagQ==\n"\
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

#if 1 /* used for OTA */
#define keyCLIENT_PRIVATE_KEY_PEM "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEpQIBAAKCAQEA866K+NhKj1Vvx/TWfFBLid0RVgd+lhCV28kbwSGZhnGJM+ax\n"\
"jxa6OaJjXVObrDcpTKhCaecJE+BBdEgqeE/GuCuoYFe4459ZgWErKRGtDYw1P8DW\n"\
"3Rquiu1SuXzbIqMxTCuXWpLcXK4iu54t6uP17ZTi3eCpbM3/k5TGWfAiaACelhPU\n"\
"1ufF1KlocXbyQII/TwAGqwry8V1EDOgYuitGQzmNtkbikiiWSxjpP/0PROiVhXVz\n"\
"gs1qPMCfbeaNdtq/ERnSts8z/W/kI2fKw1yHcBS09sPBIDSpjvkymCn5k3cOJYwV\n"\
"pdzc2ATaYkeXznXBjbiBm68G36Zd3atuAvwKnwIDAQABAoIBAQDswEMDPFlMYwyk\n"\
"cAtHYlfRzPe/5q4lLF382KpffL6FcZ+EznjqdYFe55e0n7uamUlF0iAcdjxKlFHj\n"\
"oQ2qMI/4dWo0mGXouPQkEHz3fNXuWK+PjlLzJK46ChOaZWUehQEraRq3BAKXBRR3\n"\
"xloEYZCZUFMOkOzq6kWKZDxGO1miQjppExAWjAoeDSRormIcm6r3kzQ1gd+73QhT\n"\
"1r5sxZJBcKwXCxPHjElr1CkOqXLfh98lREfTktVrsXM9bEjqYnHGoi8oZeByKgLk\n"\
"NhG9VnDPEKHYF62tl9yYUN+OqZeJcCkV3TbW0CFeq2esIe82oxFgDXpfTIsr7LrF\n"\
"Kr7NOAaZAoGBAP5RWqydNJszBlf9GzmXQ47APYEr4X0EudoQ/mBB10twIlv2r+FX\n"\
"aqCTEKBvWddCxQ9YzSXnJWygJZgEpGXoWGonDBqd3KqyckxJ++XdTmH1ba3T09LQ\n"\
"h33DtqdQYJDmiiEsWPw8WHRPlOAj7qKBaK6binyLgqqzJaH28xlICZe9AoGBAPVL\n"\
"LajkcZnSugxA9utLvXFo+Fdg1KV11hWJffgPlbtLwmpADr3FMXM2K6dSlFg6o/Yu\n"\
"o0oGvJeKRLZSRE/rlEZU2faprLt2Cm0vugFj/nc3/n26PX367u0hDGNLQYF2RoV3\n"\
"7xbGS54QMioQ1BgOZ+brq8z9VE9vVCW8AfvBfDOLAoGBAJK3+pjVBRF4iXNfuUIr\n"\
"azVC4PjRiQAoJtSQs0BzG7MQ2TG3Ctb1KLXEwnXcZ7dRdQCamZgBV0zow2eFXhbQ\n"\
"yZlMN6ZvZmwwENLEaAe/+kV3YzRCndTkqxY6P/kc3Okbp5rly9yRL/LCKsB42mZ0\n"\
"RCFf6LBbiCHUwqRDJRJzxdz9AoGAAwr8tQSoeB0KG87OgVq+LeL8ebEE0kyk7D15\n"\
"Z5Nh6yHkdob259nDZEd+wSOgHXwUvqATfH6a5lJbyds5Z7hnbXvt/EZckzLCrFoU\n"\
"N6Iy7O2v5YVi7+YmAqEPU0gCtwVtmIhFWgX5uCtBRA5TYz0CgvXXesKFwjqCU7mC\n"\
"WMosrUMCgYEAiFgcegyt1pekgYoM8oboSPgAh+R5WYqthwFp6Gh7n6uVRKtox5DH\n"\
"mboDFvnpFI5bP+5LN8vpAugRDC5oSjfEMS/tbCVPJyDO8P5+pQunCn91PSiTA+uk\n"\
"wT6wNp+rt0T/BHMlebrRmqysfimZQdtf4ltz1Pv53yGbhqJeKbeVR50=\n"\
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
