//

#include <stdio.h>
#include <stdlib.h>
#include <uzmtp/uzmtp.h>

const unsigned char *server_cert = (const unsigned char*)
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDkDCCAngCCQDxue2fZd7DJjANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC\n"
    "VVMxCzAJBgNVBAgMAk5ZMREwDwYDVQQHDAhCcm9va2x5bjEQMA4GA1UECgwHc2hl\n"
    "cGFyZDENMAsGA1UECwwEdGVzdDEPMA0GA1UEAwwGdGhvbWFzMSgwJgYJKoZIhvcN\n"
    "AQkBFhl0aG9tYXMuY2hpYW50aWFAZ21haWwuY29tMB4XDTE2MDgxODE4MDYxNVoX\n"
    "DTE2MDkxNzE4MDYxNVowgYkxCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJOWTERMA8G\n"
    "A1UEBwwIQnJvb2tseW4xEDAOBgNVBAoMB3NoZXBhcmQxDTALBgNVBAsMBHRlc3Qx\n"
    "DzANBgNVBAMMBnRob21hczEoMCYGCSqGSIb3DQEJARYZdGhvbWFzLmNoaWFudGlh\n"
    "QGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANQ3IHUs\n"
    "CUkNsPclTmExwJO1Igd64pJkJ2MCGHFP0753USE5yXWOzf9OyTI44L1qt+kIE7Nt\n"
    "tRk4Jtksd8gSf2tkrChu1jmrLZwdEPNH0C+UhKkjhl+R9l0dG0mPv0d/OZ/8Hq6e\n"
    "t/42h7fYutiC/8l0q7WPO6dEwNAYkdQFqdIfH94LFiIvc5DgbourUTRGAP4dGDcQ\n"
    "F2SI+56FB8IgRoZIv21J/hu/8CCyW4Oe1ijIjVY5rDPzI6yY5SO2iApmFX8/jbg2\n"
    "J0adZLaG90HTUX2FhmSjiWNj4A6MrpP+656riPg1HcvtbaITBHFnFvG2tUKpSzhv\n"
    "UUD1WLBTvC0DodMCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAYrbb+VaQ5kwsyIxV\n"
    "OtSDTPLsJRt4XcPsvdpl9bJZ5Sv3w5v0gJcsb2sYvl0kuxppqIMkq9dmyF6sNcNZ\n"
    "i8k9H86RX8/5ZTO88xEvm8jBXu8qO7/V9ibvKoEHVYI/eFDJDSZ1l+uJ/uXDoGSh\n"
    "0iO1mBkEe0Xr1c2vwiDOWD+exF62vxWbLA8TdyZam0VNzYlxZdAfYYsM/9vZn7hz\n"
    "zht6WXs3qrbTBE1MeTeUFQqAtbEdX8LoB5azxRf5FopFSlEKSHJmgOiPtZ6W1Rmh\n"
    "ffWfuqmOnPnn+KvmhqBaJr/SBsI5LPUCOBAWxBQRey3CUYuopv2KCmujFJegzkHe\n"
    "tjWFFg==\n"
    "-----END CERTIFICATE-----";

int main(int argc, char *argv[]);
int print_usage();
int die_with_error(int, char *);

int main(int argc, char *argv[]) {
    if (argc != 3 || (!(argv[1] && !memcmp(argv[1], "tcp://", 6)))) {
	print_usage();
	return -1;
    }

    TlsCtx *ctx = uzmtp_dealer_tls_new();
    uzmtp_dealer_tls_free(&ctx);

    // Allocate dealer socket ctx.
    UzmtpDealer *d = uzmtp_dealer_new();
    if (!d) return die_with_error(-1, "memory");

    // Optional setup tls...
    uzmtp_dealer_use_tls(d, NULL);
    uzmtp_dealer_use_server_pem(d, server_cert, strlen((char *)server_cert));

    // Connect to rep socket.
    if (uzmtp_dealer_connect_endpoint(d, argv[1])) {
	uzmtp_dealer_free(&d);
	return die_with_error(-2, "Can't connect to server!");
    }

    // Allocate message
    UzmtpMsg *msg = uzmtp_msg_from_const_data(0, argv[2], strlen(argv[2]));
    if (!msg) {
	uzmtp_dealer_free(&d);
	return die_with_error(-3, "memory");
    }

    // Send message
    if (uzmtp_dealer_send(d, msg)) {
	uzmtp_dealer_free(&d);
	uzmtp_msg_destroy(&msg);
	return die_with_error(-4, "network error!");
    }

    // Get response.
    UzmtpMsg *resp = uzmtp_dealer_recv(d);
    if (resp) {
	printf("%.*s\n", (int)uzmtp_msg_size(resp), uzmtp_msg_data(resp));
	uzmtp_msg_destroy(&resp);
    }

    // Clean
    uzmtp_dealer_free(&d);
    uzmtp_msg_destroy(&msg);
    return 0;
}
int print_usage() {
    printf("%s\n",
	   "USAGE: uzmtp-client tcp://x.x.x.x:y '{\"hello\":\"world\"}'");
    return 0;
}
int die_with_error(int code, char *err) {
    char errmsg[100];
    snprintf(errmsg, 100, "error[%d] [%s]", code, err);
    printf("%s\n", errmsg);
    return code;
}
