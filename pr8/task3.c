#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned long long g_cmp_count = 0;

static int cmp_int(const void *a, const void *b) {
    const int x = *(const int *)a;
    const int y = *(const int *)b;
    ++g_cmp_count;
    if (x < y) {
        return -1;
    }
    if (x > y) {
        return 1;
    }
    return 0;
}

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p && n != 0) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

static int is_sorted_non_decreasing(const int *a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) {
            return 0;
        }
    }
    return 1;
}

static void merge_sort_impl(int *a, int *tmp, size_t left, size_t right) {
    if (right - left <= 1) {
        return;
    }

    const size_t mid = left + (right - left) / 2;
    merge_sort_impl(a, tmp, left, mid);
    merge_sort_impl(a, tmp, mid, right);

    size_t i = left;
    size_t j = mid;
    size_t k = left;

    while (i < mid && j < right) {
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i < mid) {
        tmp[k++] = a[i++];
    }
    while (j < right) {
        tmp[k++] = a[j++];
    }

    for (k = left; k < right; ++k) {
        a[k] = tmp[k];
    }
}

static void merge_sort_oracle(int *a, size_t n) {
    int *tmp = xmalloc(n * sizeof(int));
    merge_sort_impl(a, tmp, 0, n);
    free(tmp);
}

static unsigned long long evaluate_comparisons(const int *a, size_t n) {
    int *copy = xmalloc(n * sizeof(int));
    if (n > 0) {
        memcpy(copy, a, n * sizeof(int));
    }

    g_cmp_count = 0;
    qsort(copy, n, sizeof(int), cmp_int);
    const unsigned long long cmp = g_cmp_count;

    free(copy);
    return cmp;
}

static void evaluate_profile(const int *a, size_t n, int repeats,
                             unsigned long long *avg_cmp, double *avg_ms) {
    unsigned long long cmp_sum = 0;
    double ms_sum = 0.0;

    for (int r = 0; r < repeats; ++r) {
        int *copy = xmalloc(n * sizeof(int));
        if (n > 0) {
            memcpy(copy, a, n * sizeof(int));
        }

        g_cmp_count = 0;
        const uint64_t t0 = now_ns();
        qsort(copy, n, sizeof(int), cmp_int);
        const uint64_t t1 = now_ns();

        cmp_sum += g_cmp_count;
        ms_sum += (double)(t1 - t0) / 1000000.0;

        free(copy);
    }

    *avg_cmp = (repeats > 0) ? (cmp_sum / (unsigned long long)repeats) : 0;
    *avg_ms = (repeats > 0) ? (ms_sum / (double)repeats) : 0.0;
}

static int randint(int low, int high) {
    return low + rand() % (high - low + 1);
}

static void fill_ascending(int *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = (int)i;
    }
}

static void fill_descending(int *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = (int)(n - 1 - i);
    }
}

static void fill_all_equal(int *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = 7;
    }
}

static void fill_random(int *a, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = randint(-1000000, 1000000);
    }
}

static void fill_organ_pipe(int *a, size_t n) {
    const size_t mid = n / 2;
    for (size_t i = 0; i < n; ++i) {
        if (i <= mid) {
            a[i] = (int)i;
        } else {
            a[i] = (int)(n - i);
        }
    }
}

static void fill_sawtooth(int *a, size_t n) {
    const int period = 17;
    for (size_t i = 0; i < n; ++i) {
        a[i] = (int)(i % (size_t)period);
    }
}

static void fill_alternating_ends(int *a, size_t n) {
    size_t lo = 0;
    size_t hi = (n == 0) ? 0 : n - 1;
    for (size_t i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            a[i] = (int)lo++;
        } else {
            a[i] = (int)hi--;
        }
    }
}

static void fill_nearly_sorted(int *a, size_t n) {
    fill_ascending(a, n);
    for (size_t i = 0; i + 1 < n; i += 100) {
        const int tmp = a[i];
        a[i] = a[i + 1];
        a[i + 1] = tmp;
    }
}

static void mutate_candidate(int *a, size_t n) {
    if (n < 2) {
        return;
    }

    const int mutation_type = randint(0, 2);

    if (mutation_type == 0) {
        const size_t i = (size_t)randint(0, (int)n - 1);
        const size_t j = (size_t)randint(0, (int)n - 1);
        const int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
        return;
    }

    if (mutation_type == 1) {
        size_t i = (size_t)randint(0, (int)n - 1);
        size_t j = (size_t)randint(0, (int)n - 1);
        if (i > j) {
            const size_t t = i;
            i = j;
            j = t;
        }
        while (i < j) {
            const int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            ++i;
            --j;
        }
        return;
    }

    const size_t pos = (size_t)randint(0, (int)n - 1);
    if (randint(0, 1) == 0) {
        a[pos] = randint(-50, 50);
    } else {
        a[pos] = randint(-1000000, 1000000);
    }
}

static void search_worst_case(size_t n, int iterations,
                              int *best_out,
                              unsigned long long *best_cmp_out) {
    int *current = xmalloc(n * sizeof(int));
    int *trial = xmalloc(n * sizeof(int));

    fill_random(current, n);
    unsigned long long best_cmp = evaluate_comparisons(current, n);
    memcpy(best_out, current, n * sizeof(int));

    for (int step = 0; step < iterations; ++step) {
        memcpy(trial, current, n * sizeof(int));
        mutate_candidate(trial, n);

        const unsigned long long cmp_trial = evaluate_comparisons(trial, n);
        const unsigned long long cmp_current = evaluate_comparisons(current, n);

        if (cmp_trial >= cmp_current) {
            memcpy(current, trial, n * sizeof(int));
        }
        if (cmp_trial > best_cmp) {
            best_cmp = cmp_trial;
            memcpy(best_out, trial, n * sizeof(int));
        }
    }

    *best_cmp_out = best_cmp;
    free(current);
    free(trial);
}

static int test_qsort_on_array(const int *src, size_t n, const char *label) {
    int *actual = xmalloc(n * sizeof(int));
    int *expected = xmalloc(n * sizeof(int));

    if (n > 0) {
        memcpy(actual, src, n * sizeof(int));
        memcpy(expected, src, n * sizeof(int));
    }

    if (n > 0) {
        qsort(actual, n, sizeof(int), cmp_int);
        merge_sort_oracle(expected, n);
    }

    int ok = 1;
    if (!is_sorted_non_decreasing(actual, n)) {
        ok = 0;
        fprintf(stderr, "[FAIL] %s: array is not sorted in non-decreasing order\n", label);
    }

    for (size_t i = 0; i < n && ok; ++i) {
        if (actual[i] != expected[i]) {
            ok = 0;
            fprintf(stderr, "[FAIL] %s: mismatch with oracle at index %zu\n", label, i);
        }
    }

    if (ok) {
        printf("[PASS] %s\n", label);
    }

    free(actual);
    free(expected);
    return ok;
}

static int run_qsort_tests(void) {
    int all_ok = 1;

    all_ok &= test_qsort_on_array(NULL, 0, "empty array");

    int one[] = {42};
    all_ok &= test_qsort_on_array(one, sizeof(one) / sizeof(one[0]), "single element");

    int sorted[] = {-3, -1, 0, 4, 9, 11};
    all_ok &= test_qsort_on_array(sorted, sizeof(sorted) / sizeof(sorted[0]), "already sorted");

    int reversed[] = {9, 7, 5, 3, 1, -1, -3};
    all_ok &= test_qsort_on_array(reversed, sizeof(reversed) / sizeof(reversed[0]), "reverse sorted");

    int duplicates[] = {5, 5, 5, 2, 2, 2, 9, 1, 1, 0, 0, 0, 0};
    all_ok &= test_qsort_on_array(duplicates, sizeof(duplicates) / sizeof(duplicates[0]), "many duplicates");

    int mixed[] = {2147483647, -2147483647, 0, 10, -10, 999, -999};
    all_ok &= test_qsort_on_array(mixed, sizeof(mixed) / sizeof(mixed[0]), "mixed magnitudes");

    const int random_tests = 200;
    const int max_len = 256;
    for (int t = 0; t < random_tests; ++t) {
        const size_t n = (size_t)randint(0, max_len);
        int *arr = xmalloc(n * sizeof(int));
        for (size_t i = 0; i < n; ++i) {
            arr[i] = randint(-10000, 10000);
        }

        char label[64];
        snprintf(label, sizeof(label), "random test #%d (n=%zu)", t + 1, n);
        all_ok &= test_qsort_on_array(arr, n, label);
        free(arr);

        if (!all_ok) {
            break;
        }
    }

    if (all_ok) {
        printf("All qsort correctness tests passed.\n");
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

static void run_experiments(size_t n, int repeats, int search_iters) {
    printf("Experiment size n=%zu, repeats=%d, search_iters=%d\n\n", n, repeats, search_iters);

    struct pattern_case {
        const char *name;
        void (*fill)(int *, size_t);
    } cases[] = {
        {"ascending", fill_ascending},
        {"descending", fill_descending},
        {"all_equal", fill_all_equal},
        {"random", fill_random},
        {"organ_pipe", fill_organ_pipe},
        {"sawtooth", fill_sawtooth},
        {"alternating_ends", fill_alternating_ends},
        {"nearly_sorted", fill_nearly_sorted},
    };

    int *arr = xmalloc(n * sizeof(int));

    printf("%-20s | %-15s | %-10s\n", "pattern", "avg comparisons", "avg ms");
    printf("---------------------+-----------------+-----------\n");

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        cases[i].fill(arr, n);

        unsigned long long avg_cmp = 0;
        double avg_ms = 0.0;
        evaluate_profile(arr, n, repeats, &avg_cmp, &avg_ms);

        printf("%-20s | %-15llu | %-10.3f\n", cases[i].name, avg_cmp, avg_ms);
    }

    int *best = xmalloc(n * sizeof(int));
    unsigned long long best_cmp = 0;
    search_worst_case(n, search_iters, best, &best_cmp);

    unsigned long long avg_cmp = 0;
    double avg_ms = 0.0;
    evaluate_profile(best, n, repeats, &avg_cmp, &avg_ms);

    printf("%-20s | %-15llu | %-10.3f\n", "auto_searched", avg_cmp, avg_ms);

    printf("\nTop 32 elements of auto_searched candidate:\n");
    const size_t preview = (n < 32) ? n : 32;
    for (size_t i = 0; i < preview; ++i) {
        printf("%d%s", best[i], (i + 1 == preview) ? "\n" : " ");
    }

    printf("(Best single-run comparison count during search: %llu)\n", best_cmp);

    free(best);
    free(arr);
}

int main(int argc, char **argv) {
    srand((unsigned int)time(NULL));

    if (argc >= 2 && strcmp(argv[1], "--tests") == 0) {
        return run_qsort_tests();
    }

    size_t n = 10000;
    int repeats = 3;
    int search_iters = 120;

    if (argc >= 2 && strcmp(argv[1], "--experiment") == 0) {
        if (argc >= 3) {
            n = (size_t)strtoull(argv[2], NULL, 10);
        }
        if (argc >= 4) {
            repeats = atoi(argv[3]);
        }
        if (argc >= 5) {
            search_iters = atoi(argv[4]);
        }
    } else if (argc >= 2) {
        fprintf(stderr,
                "Usage:\n"
                "  %s --experiment [n] [repeats] [search_iters]\n"
                "  %s --tests\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    run_experiments(n, repeats, search_iters);
    return run_qsort_tests();
}
