
#include <getopt.h>

#include "types.h"
#include "http.h"
#include "err.h"

static const char *req_usage =
    "usage: req [-h | --help] [-v | --version] [-C <path>] [args] <file> ...\n"
    "  --                            specify end of arguments\n"
    "      --stdin                   read message from stdin\n"
    "  -C             <path>         chdir to <path> at initialisation\n"
    "  -c, --config   <file>         load configuration from <file>\n"
    "  -e             <key>=<val>    evaluate config variable <key> to <val>\n"
    "  -H, --host     <host>         send request to <host>\n"
    "  -h, --help                    show this help message\n"
    "  -V, --verbose                 print request details\n"
    "  -v, --version                 show version information";

#define OPT_CFILE   'c'
#define OPT_HOST    'H'
#define OPT_HELP    'h'
#define OPT_VERBOSE 'V'
#define OPT_VERSION 'v'

#define OPT_STDIN   0xff

#define OPT_NOOPT   ':'
#define OPT_UNKNOWN '?'

static const char *short_options = ":hvVc:H:";
static struct option long_options[] = {
    { "config",   required_argument, NULL, OPT_CFILE   },
    { "host",     required_argument, NULL, OPT_HOST    },
    { "help",     no_argument,       NULL, OPT_HELP    },
    { "stdin",    no_argument,       NULL, OPT_STDIN   },
    { "verbose",  no_argument,       NULL, OPT_VERBOSE },
    { "version",  no_argument,       NULL, OPT_VERSION },
    { 0, 0, 0, 0 }
};

struct options {
    char   *config;
    char   *file;
    char   *host;
    int     port;
    bool_t  help;
    bool_t  stdin;
    bool_t  verbose;
    bool_t  version;
};

static struct options DEFAULT_OPTS = {
    .config     = "",
    .file       = "",
    .host       = "",
    .port       = 0,
    .help       = FALSE,
    .stdin      = FALSE,
    .verbose    = FALSE,
    .version    = FALSE,
};

void readfile(FILE *f, char **msg) {
    char buffer[BUFLEN];
    size_t content_len = 1;

    if (*msg == NULL) {
        die("failed to allocate message");
    }

    *msg[0] = '\0'; // make null-terminated

    while (fgets(buffer, BUFLEN, f)) {
        char *old = *msg;
        content_len += strlen(buffer);
        *msg = realloc(*msg, content_len);

        if (*msg == NULL) {
            free(old);
            die("failed to reallocate message");
        }

        strcat(*msg, buffer);
    }

    if (ferror(f)) {
        free(*msg);
        die("error reading from file");
    }
}

int main(int argc, char **const argv) {
    struct options *opts = malloc(sizeof(struct options));
    memcpy(opts, &DEFAULT_OPTS, sizeof(struct options));

    FILE *in;
    int c;
    opterr = 0;
    char *msg;

    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
            case OPT_CFILE:   opts->config    = optarg; break;
            case OPT_HOST:    opts->host      = optarg; break;
            case OPT_HELP:    opts->help      = TRUE;   break;
            case OPT_VERBOSE: opts->verbose   = TRUE;   break;
            case OPT_VERSION: opts->version   = TRUE;   break;
            case OPT_STDIN:   opts->stdin     = TRUE;   break;

            case OPT_NOOPT:   die("option '%" PFMT8s "' requires an arguement", argv[optind - 1]); break;
            case OPT_UNKNOWN: die("unknown option '%" PFMT8s "'", argv[optind - 1]); break;
        }
    }

    if (optind < argc) {
        opts->file = argv[optind];
    }

    if (opts->help) {
        puts(req_usage);
    } else if (opts->version) {
        printf("%s version %s\n", AX_BINNAME, AX_VERSION);

    } else {
        if (!*opts->file && !opts->stdin) {
            die("no input file specified");
        }

        if (opts->stdin) {
            in = stdin;
        } else {
            in = fopen(opts->file, "r");
        }

        if (!in) {
            die("error opening input file '%" PFMT8s "'", opts->file);
        }

        msg = malloc(BUFLEN);
        readfile(in, &msg);

        if (opts->verbose) {
            printf("%s", msg);
        }

        if (!*opts->host) {
            opts->host = "127.0.0.1";
            opts->port = 8181;
        }

        do_request(opts->port, opts->host, msg);

        free(msg);
        fclose(in);
    }

    free(opts);

    return 0;
}
