/*
 * Ultraner C SDK
 * One API for payments across Africa: mobile money, cards, PayPal, wallets.
 * Docs: https://ultraner.com/docs  ·  Spec: https://ultraner.com/openapi.json
 *
 * Requires libcurl. JSON bodies are passed/returned as raw strings so you can
 * use whatever JSON library you prefer (cJSON, jansson, etc.).
 */
#ifndef ULTRANER_H
#define ULTRANER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *api_key;
    char *base_url;  /* defaults to https://api.ultraner.com */
    long timeout_s;  /* request timeout in seconds (default 30) */
} ultraner_client;

typedef struct {
    long status;   /* HTTP status code, 0 on transport error */
    char *body;    /* malloc'd response body (JSON), NUL-terminated; may be NULL */
    char *error;   /* malloc'd transport error message, or NULL */
} ultraner_response;

/* Create/destroy a client. base_url may be NULL for the default. */
ultraner_client *ultraner_new(const char *api_key, const char *base_url);
void ultraner_free(ultraner_client *client);

/* Generic request. method e.g. "GET"/"POST". json_body may be NULL. */
ultraner_response ultraner_request(ultraner_client *client, const char *method,
                                   const char *path, const char *json_body);
void ultraner_response_free(ultraner_response *res);

/* Convenience helpers (json_body is a raw JSON string). */
ultraner_response ultraner_create_mobile_money(ultraner_client *c, const char *json_body);
ultraner_response ultraner_payment_status(ultraner_client *c, const char *reference);
ultraner_response ultraner_create_disbursement(ultraner_client *c, const char *json_body);
ultraner_response ultraner_wallet(ultraner_client *c);
ultraner_response ultraner_transactions(ultraner_client *c, int page, int limit);
ultraner_response ultraner_create_escrow(ultraner_client *c, const char *json_body);
ultraner_response ultraner_release_escrow(ultraner_client *c, const char *escrow_code);

#ifdef __cplusplus
}
#endif

#endif /* ULTRANER_H */
