/*
 * Ultraner C SDK implementation (libcurl).
 * Build: cc -c ultraner.c -lcurl   (link your program with -lcurl)
 */
#include "ultraner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define ULTRANER_DEFAULT_BASE "https://api.ultraner.com"

static char *ul_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

struct ul_buf {
    char *data;
    size_t len;
};

static size_t ul_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t add = size * nmemb;
    struct ul_buf *b = (struct ul_buf *)userdata;
    char *grown = (char *)realloc(b->data, b->len + add + 1);
    if (!grown) return 0;
    b->data = grown;
    memcpy(b->data + b->len, ptr, add);
    b->len += add;
    b->data[b->len] = '\0';
    return add;
}

ultraner_client *ultraner_new(const char *api_key, const char *base_url) {
    if (!api_key || !*api_key) return NULL;
    ultraner_client *c = (ultraner_client *)calloc(1, sizeof(ultraner_client));
    if (!c) return NULL;
    c->api_key = ul_strdup(api_key);
    c->base_url = ul_strdup(base_url && *base_url ? base_url : ULTRANER_DEFAULT_BASE);
    c->timeout_s = 30;
    return c;
}

void ultraner_free(ultraner_client *c) {
    if (!c) return;
    free(c->api_key);
    free(c->base_url);
    free(c);
}

ultraner_response ultraner_request(ultraner_client *c, const char *method,
                                   const char *path, const char *json_body) {
    ultraner_response res = {0, NULL, NULL};
    if (!c) {
        res.error = ul_strdup("ultraner: client is NULL");
        return res;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        res.error = ul_strdup("ultraner: failed to init curl");
        return res;
    }

    size_t url_len = strlen(c->base_url) + strlen(path) + 1;
    char *url = (char *)malloc(url_len);
    snprintf(url, url_len, "%s%s", c->base_url, path);

    char auth[512];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", c->api_key);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth);

    struct ul_buf buf = {NULL, 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, c->timeout_s);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ul_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    if (json_body) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    }

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        res.error = ul_strdup(curl_easy_strerror(rc));
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
        res.body = buf.data; /* ownership transferred to caller */
        buf.data = NULL;
    }

    free(buf.data);
    free(url);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res;
}

void ultraner_response_free(ultraner_response *res) {
    if (!res) return;
    free(res->body);
    free(res->error);
    res->body = NULL;
    res->error = NULL;
}

ultraner_response ultraner_create_mobile_money(ultraner_client *c, const char *json_body) {
    return ultraner_request(c, "POST", "/v1/payments/express/mno", json_body);
}

ultraner_response ultraner_payment_status(ultraner_client *c, const char *reference) {
    char path[256];
    snprintf(path, sizeof(path), "/v1/payments/express/status/%s", reference ? reference : "");
    return ultraner_request(c, "GET", path, NULL);
}

ultraner_response ultraner_create_disbursement(ultraner_client *c, const char *json_body) {
    return ultraner_request(c, "POST", "/v1/disbursements", json_body);
}

ultraner_response ultraner_wallet(ultraner_client *c) {
    return ultraner_request(c, "GET", "/v1/wallet", NULL);
}

ultraner_response ultraner_transactions(ultraner_client *c, int page, int limit) {
    char path[128];
    snprintf(path, sizeof(path), "/v1/transactions?page=%d&limit=%d", page > 0 ? page : 1, limit > 0 ? limit : 20);
    return ultraner_request(c, "GET", path, NULL);
}

ultraner_response ultraner_create_escrow(ultraner_client *c, const char *json_body) {
    return ultraner_request(c, "POST", "/v1/escrow", json_body);
}

ultraner_response ultraner_release_escrow(ultraner_client *c, const char *escrow_code) {
    char path[256];
    snprintf(path, sizeof(path), "/v1/escrow/%s/release", escrow_code ? escrow_code : "");
    return ultraner_request(c, "POST", path, NULL);
}
