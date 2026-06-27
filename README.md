# Ultraner C SDK

One API for payments across Africa: mobile money, cards, PayPal and wallets. Live in Tanzania and Rwanda, expanding across the continent.

Requires **libcurl**. JSON is passed and returned as raw strings, so you can pair it with any JSON library (cJSON, jansson).

- Docs: https://ultraner.com/docs
- OpenAPI: https://ultraner.com/openapi.json
- For AI: https://ultraner.com/ai

## Build

```bash
cc -c ultraner.c
# link your program with libcurl:
cc your_app.c ultraner.o -lcurl -o your_app
```

## Usage

```c
#include "ultraner.h"
#include <stdio.h>

int main(void) {
    ultraner_client *c = ultraner_new("sk_live_...", NULL);

    const char *body =
        "{\"amount\":5000,\"currency\":\"TZS\",\"provider\":\"Vodacom\","
        "\"accountNumber\":\"255700000000\",\"externalId\":\"order_1001\"}";

    ultraner_response r = ultraner_create_mobile_money(c, body);
    if (r.error) {
        fprintf(stderr, "error: %s\n", r.error);
    } else {
        printf("status=%ld body=%s\n", r.status, r.body ? r.body : "");
    }
    ultraner_response_free(&r);

    ultraner_free(c);
    return 0;
}
```

`ultraner_response` carries the HTTP `status` and the raw JSON `body` (free it with `ultraner_response_free`). Parse the body with your JSON library of choice.

## License

MIT
