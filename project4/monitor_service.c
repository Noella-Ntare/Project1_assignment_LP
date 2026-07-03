/*
 * monitor_service.c
 *
 * A simulated background monitoring service that demonstrates
 * POSIX signal handling using sigaction().
 *
 * Signals handled:
 *   SIGINT  (Ctrl-C)  – graceful shutdown
 *   SIGUSR1           – print status report
 *   SIGTERM           – emergency shutdown
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/* ── Global flags ────────────────────────────────────────────────────── */
volatile sig_atomic_t keep_running  = 1;
volatile sig_atomic_t status_requested = 0;

/* ── Helpers ─────────────────────────────────────────────────────────── */

static void print_timestamp(void)
{
    time_t now = time(NULL);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    printf("[%s] ", buf);
}

/* ── Signal handlers ─────────────────────────────────────────────────── */

/*
 * SIGINT – Ctrl-C: request graceful termination.
 * We set a flag rather than calling printf() directly, because
 * printf() is not async-signal-safe. The main loop checks the flag.
 */
static void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;   /* signal main loop to exit cleanly */
}

/*
 * SIGUSR1 – administrator status request.
 * Sets a flag; the message is printed from the main loop where it
 * is safe to do so.
 */
static void handle_sigusr1(int sig)
{
    (void)sig;
    status_requested = 1;
}

/*
 * SIGTERM – emergency shutdown: we write directly and exit.
 * Using write() (async-signal-safe) instead of printf().
 */
static void handle_sigterm(int sig)
{
    (void)sig;
    const char msg[] = "\n[Monitor Service] Emergency shutdown signal received.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    _exit(1);   /* _exit() is async-signal-safe; exit() is not */
}

/* ── Register all handlers using sigaction() ─────────────────────────── */

static void register_signal_handlers(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);

    /* SIGINT */
    sa.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction(SIGINT)");
        exit(EXIT_FAILURE);
    }

    /* SIGUSR1 */
    sa.sa_handler = handle_sigusr1;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction(SIGUSR1)");
        exit(EXIT_FAILURE);
    }

    /* SIGTERM */
    sa.sa_handler = handle_sigterm;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM)");
        exit(EXIT_FAILURE);
    }
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    register_signal_handlers();

    printf("[Monitor Service] Started. PID = %d\n", getpid());
    printf("[Monitor Service] Send signals with:\n");
    printf("  kill -SIGUSR1 %d\n", getpid());
    printf("  kill -SIGINT  %d\n", getpid());
    printf("  kill -SIGTERM %d\n\n", getpid());

    int tick = 0;
    while (keep_running) {
        /* Print heartbeat every 5 seconds */
        if (tick % 5 == 0) {
            print_timestamp();
            printf("[Monitor Service] System running normally...\n");
            fflush(stdout);
        }

        /* Handle pending status request (safe here in main loop) */
        if (status_requested) {
            status_requested = 0;
            print_timestamp();
            printf("[Monitor Service] System status requested by administrator.\n");
            fflush(stdout);
        }

        sleep(1);
        tick++;
    }

    /* Reached only on SIGINT */
    printf("\n[Monitor Service] Monitor Service shutting down safely.\n");
    return 0;
}
