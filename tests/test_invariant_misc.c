#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * We simulate the vulnerable pattern from src/misc.c:
 *
 *   strcpy(&action_array[action_index], new_text);
 *
 * The invariant is: buffer reads/writes must never exceed the declared
 * length of action_array.  A safe implementation must either truncate
 * or reject new_text when it would overflow the remaining space.
 *
 * We provide a hardened reference implementation and verify that it
 * always respects the buffer boundary, even for adversarial inputs.
 */

#define ACTION_ARRAY_SIZE 256

/* ------------------------------------------------------------------ */
/* Safe (reference) implementation of the copy                         */
/* ------------------------------------------------------------------ */
static int safe_action_copy(char *action_array,
                            size_t action_array_size,
                            size_t action_index,
                            const char *new_text)
{
    /* Invariant: action_index must be within the buffer */
    if (action_index >= action_array_size)
        return -1;

    size_t remaining = action_array_size - action_index;
    size_t text_len  = strlen(new_text);

    /* Reject if new_text does not fit (including NUL terminator) */
    if (text_len + 1 > remaining)
        return -1;

    /* Safe copy */
    memcpy(&action_array[action_index], new_text, text_len + 1);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Helper: generate a string of given length filled with a character   */
/* ------------------------------------------------------------------ */
static char *make_payload(size_t len, char fill)
{
    char *p = (char *)malloc(len + 1);
    if (!p) return NULL;
    memset(p, fill, len);
    p[len] = '\0';
    return p;
}

/* ------------------------------------------------------------------ */
/* Test: oversized inputs must never cause out-of-bounds access        */
/* ------------------------------------------------------------------ */
START_TEST(test_buffer_reads_never_exceed_declared_length)
{
    /* Invariant: safe_action_copy must never write beyond action_array_size */

    const char *static_payloads[] = {
        /* Exact boundary */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", /* 256 A's */

        /* 2x buffer size */
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", /* 512 B's */

        /* Format string attack */
        "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",

        /* SQL injection style */
        "'; DROP TABLE actions; --",

        /* Shell metacharacters */
        "$(cat /etc/passwd)",

        /* Null-byte injection attempt (handled as C string, stops at first NUL) */
        "safe_prefix",

        /* Empty string (edge case) */
        "",

        /* Single character */
        "X",

        /* Exactly one less than buffer */
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC", /* 255 C's */

        /* Path traversal */
        "../../../../etc/passwd",

        /* Unicode / high-byte sequences */
        "\xff\xfe\xfd\xfc\xfb\xfa\xf9\xf8",
    };

    int num_static = (int)(sizeof(static_payloads) / sizeof(static_payloads[0]));

    /* Dynamic payloads: 2x, 10x, 100x the buffer size */
    size_t dynamic_sizes[] = {
        ACTION_ARRAY_SIZE * 2,
        ACTION_ARRAY_SIZE * 10,
        ACTION_ARRAY_SIZE * 100,
        ACTION_ARRAY_SIZE + 1,
        ACTION_ARRAY_SIZE - 1,
        ACTION_ARRAY_SIZE,
        1,
        0,  /* zero-length handled via "" above, but test index edge cases */
    };
    int num_dynamic = (int)(sizeof(dynamic_sizes) / sizeof(dynamic_sizes[0]));

    /* ---- Test static payloads at various action_index offsets ---- */
    for (int i = 0; i < num_static; i++) {
        const char *payload = static_payloads[i];
        size_t payload_len  = strlen(payload);

        /* Canary-guarded buffer */
        uint8_t guarded[ACTION_ARRAY_SIZE + 16];
        memset(guarded, 0xAB, sizeof(guarded)); /* fill with canary */

        char *action_array = (char *)guarded;

        /* Try inserting at index 0 */
        int rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE, 0, payload);

        if (rc == 0) {
            /* Copy succeeded: verify NUL terminator is within bounds */
            ck_assert_msg(payload_len < ACTION_ARRAY_SIZE,
                          "Copy succeeded but payload_len >= ACTION_ARRAY_SIZE");
            ck_assert_msg(action_array[payload_len] == '\0',
                          "NUL terminator missing after successful copy");
        }

        /* Canary bytes after ACTION_ARRAY_SIZE must be untouched */
        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xAB,
                          "Canary byte overwritten at index %d for payload %d", k, i);
        }

        /* Try inserting at a non-zero index (half-way) */
        size_t mid_index = ACTION_ARRAY_SIZE / 2;
        memset(guarded, 0xAB, sizeof(guarded));

        rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE, mid_index, payload);

        if (rc == 0) {
            size_t remaining = ACTION_ARRAY_SIZE - mid_index;
            ck_assert_msg(payload_len + 1 <= remaining,
                          "Copy at mid_index succeeded but payload too large");
        }

        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xAB,
                          "Canary overwritten at index %d (mid test, payload %d)", k, i);
        }

        /* Try inserting at action_index == ACTION_ARRAY_SIZE (boundary) */
        memset(guarded, 0xAB, sizeof(guarded));
        rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE, ACTION_ARRAY_SIZE, payload);
        ck_assert_msg(rc == -1,
                      "Copy at exact boundary index must be rejected");

        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xAB,
                          "Canary overwritten at index %d (boundary test, payload %d)", k, i);
        }

        /* Try inserting at action_index > ACTION_ARRAY_SIZE (out-of-bounds index) */
        memset(guarded, 0xAB, sizeof(guarded));
        rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE,
                              ACTION_ARRAY_SIZE + 1, payload);
        ck_assert_msg(rc == -1,
                      "Copy at out-of-bounds index must be rejected");

        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xAB,
                          "Canary overwritten at index %d (oob index test, payload %d)", k, i);
        }
    }

    /* ---- Test dynamically generated payloads ---- */
    for (int d = 0; d < num_dynamic; d++) {
        size_t sz = dynamic_sizes[d];
        if (sz == 0) continue; /* skip zero-length dynamic; covered by "" */

        char *dyn_payload = make_payload(sz, 'D');
        ck_assert_msg(dyn_payload != NULL, "malloc failed for dynamic payload");

        uint8_t guarded[ACTION_ARRAY_SIZE + 16];
        memset(guarded, 0xCD, sizeof(guarded));
        char *action_array = (char *)guarded;

        int rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE, 0, dyn_payload);

        if (rc == 0) {
            ck_assert_msg(sz < ACTION_ARRAY_SIZE,
                          "Dynamic copy succeeded but sz >= ACTION_ARRAY_SIZE");
            ck_assert_msg(action_array[sz] == '\0',
                          "NUL terminator missing after dynamic copy");
        } else {
            /* Rejected: buffer must be untouched or at least canary intact */
            ck_assert_msg(rc == -1, "Unexpected return code from safe_action_copy");
        }

        /* Canary check */
        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xCD,
                          "Canary overwritten at index %d for dynamic payload size %zu",
                          k, sz);
        }

        free(dyn_payload);
    }
}
END_TEST

/* ------------------------------------------------------------------ */
/* Additional test: action_index arithmetic overflow                   */
/* ------------------------------------------------------------------ */
START_TEST(test_action_index_overflow_rejected)
{
    /* Invariant: integer overflow in index arithmetic must not bypass bounds check */

    uint8_t guarded[ACTION_ARRAY_SIZE + 16];
    memset(guarded, 0xEF, sizeof(guarded));
    char *action_array = (char *)guarded;

    const char *payload = "OVERFLOW";

    /* Wrap-around index values */
    size_t overflow_indices[] = {
        SIZE_MAX,
        SIZE_MAX - 1,
        SIZE_MAX / 2,
        (size_t)UINT32_MAX,
        (size_t)UINT32_MAX + 1,
        ACTION_ARRAY_SIZE,
        ACTION_ARRAY_SIZE + 1,
    };
    int n = (int)(sizeof(overflow_indices) / sizeof(overflow_indices[0]));

    for (int i = 0; i < n; i++) {
        int rc = safe_action_copy(action_array, ACTION_ARRAY_SIZE,
                                  overflow_indices[i], payload);
        ck_assert_msg(rc == -1,
                      "Overflow index %zu must be rejected", overflow_indices[i]);

        for (int k = ACTION_ARRAY_SIZE; k < (int)sizeof(guarded); k++) {
            ck_assert_msg((uint8_t)guarded[k] == 0xEF,
                          "Canary overwritten at index %d for overflow index test %d",
                          k, i);
        }
    }
}
END_TEST

/* ------------------------------------------------------------------ */
/* Suite and runner                                                     */
/* ------------------------------------------------------------------ */
Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s       = suite_create("Security_CWE120_BufferOverread");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_reads_never_exceed_declared_length);
    tcase_add_test(tc_core, test_action_index_overflow_rejected);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite   *s;
    SRunner *sr;

    s  = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}