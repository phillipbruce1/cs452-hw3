#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "Command.h"
#include "error.h"

typedef struct {
    char *file;
    char **argv;
    T_redir redir;
} *CommandRep;

typedef struct _hist_entry {
    char *line;
    char *data;
} HIST_ENTRY;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd = 0;
static char *cwd = 0;

static void builtin_args(CommandRep r, int n) {
    char **argv = r->argv;
    for (n++; *argv++; n--);
    if (n)
        ERROR("wrong number of arguments to builtin command"); // warn
}

static void outputToFile(char *file) {
    int fd = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0 || dup2(fd, 1) < 0) {
        ERROR("Failed to open file for redirection");
        exit(0);
    }
    close(fd);
}

static void inputFromFile(char *file) {
    int fd = open(file, O_RDONLY);
    if (fd < 0 || dup2(fd, 0) < 0) {
        ERROR("Failed to open file for redirection");
        exit(0);
    }
    close(fd);
}

BIDEFN(exit) {
    builtin_args(r, 0);
    *eof = 1;
}

BIDEFN(pwd) {
    builtin_args(r, 0);
    if (!cwd)
        cwd = getcwd(0, 0);
    printf("%s\n", cwd);
}

BIDEFN(cd) {
    builtin_args(r, 1);
    if (strcmp(r->argv[1], "-") == 0) {
        char *twd = cwd;
        cwd = owd;
        owd = twd;
    } else {
        if (owd) free(owd);
        owd = cwd;
        cwd = strdup(r->argv[1]);
    }
    if (cwd && chdir(cwd))
        ERROR("chdir() failed"); // warn
}

BIDEFN(history) {
    builtin_args(r, 0);
    register HIST_ENTRY **list;
    list = history_list();
    if (list) {
        for (int i = 0; list[i]; i++)
            printf("%s\n", list[i]->line);
    }
}

static int builtin(BIARGS) {
    typedef struct {
        char *s;

        void (*f)(BIARGS);
    } Builtin;
    static const Builtin builtins[] = {
            BIENTRY(exit),
            BIENTRY(pwd),
            BIENTRY(cd),
            BIENTRY(history),
            {0, 0}
    };
    int i;
    for (i = 0; builtins[i].s; i++)
        if (!strcmp(r->file, builtins[i].s)) {
            if (i != 0 && i != 2 && r->redir) {
                int stdout_copy = dup(STDOUT_FILENO);
                int stdin_copy = dup(STDIN_FILENO);
                if (!strcmp(r->redir->redir, ">"))
                    outputToFile(r->redir->word->s);
                if (!strcmp(r->redir->redir, "<"))
                    inputFromFile(r->redir->word->s);
                builtins[i].f(r, eof, jobs);
                dup2(stdout_copy, 1);
                dup2(stdin_copy, 0);
            } else
                builtins[i].f(r, eof, jobs);
            return 1;
        }
    return 0;
}

static char **getargs(T_words words) {
    int n = 0;
    T_words p = words;
    while (p) {
        p = p->words;
        n++;
    }
    char **argv = (char **) malloc(sizeof(char *) * (n + 1));
    if (!argv)
        ERROR("malloc() failed");
    p = words;
    int i = 0;
    while (p) {
        argv[i++] = strdup(p->word->s);
        p = p->words;
    }
    argv[i] = 0;
    return argv;
}

extern Command newCommand(T_words words, T_redir redir) {
    CommandRep r = (CommandRep) malloc(sizeof(*r));
    if (!r)
        ERROR("malloc() failed");
    r->argv = getargs(words);
    r->file = r->argv[0];
    if (redir) {
        r->redir = redir;
    }
    return r;
}

static void child(CommandRep r, int fg) {
    int eof = 0;
    Jobs jobs = newJobs();
    if (builtin(r, &eof, jobs))
        return;
    if (r->redir) {
        if (!strcmp(r->redir->redir, ">"))
            outputToFile(r->redir->word->s);
        else if (!strcmp(r->redir->redir, "<"))
            inputFromFile(r->redir->word->s);
    }
    execvp(r->argv[0], r->argv);
    ERROR("execvp() failed");
    exit(0);
}

extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
                        int *jobbed, int *eof, int fg) {
    CommandRep r = command;
    if (fg && builtin(r, eof, jobs))
        return;
    if (!*jobbed) {
        *jobbed = 1;
        addJobs(jobs, pipeline);
    }
    int pid = fork();
    if (pid == -1)
        ERROR("fork() failed");
    if (pid == 0)
        child(r, fg);
}

extern void freeCommand(Command command) {
    CommandRep r = command;
    char **argv = r->argv;
    while (*argv)
        free(*argv++);
    free(r->argv);
    free(r);
}

extern void freestateCommand() {
    if (cwd) free(cwd);
    if (owd) free(owd);
}
