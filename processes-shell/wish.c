#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define no_use(x) (void)(x)

const int BUFF_SIZE = 4096;
char buff[BUFF_SIZE];

struct Str {
    char * data;
    size_t size;
    size_t cap;
};

struct Path {
    struct Str item;
    struct Path * next;
};

struct Path g_path;

void * Mmalloc(size_t size);

void add_str(struct Str * dst, const char * src)
{
    if (!src) {
        return;
    }
    assert(dst != NULL);

    size_t add = strlen(src);
    size_t need = dst->size + add + 1;
    if (need > dst->cap) {
        size_t new_cap = need << 1;
        char * ptr = (char *)Mmalloc(new_cap);
        dst->cap = new_cap;

        memcpy(ptr, dst->data, dst->size);
        free(dst->data);
        dst->data = ptr;
    }

    memcpy(dst->data + dst->size, src, add);
    dst->size += add;
    dst->data[dst->size] = 0;
}

struct Str
make_str(const char * s)
{
    struct Str str;
    if (s == NULL) {
        str.data = NULL;
        str.size = str.cap = 0;
        return str;
    }
    str.data = strdup(s);
    str.size = strlen(s);
    str.cap = str.size + 1;
    return str;
}

struct Path * make_path(const char * path)
{
    struct Path * p = (struct Path *) Mmalloc(sizeof(struct Path));
    p->item = make_str(path);
    p->next = NULL;
    return p;
}

void
add_to_path(struct Path * p, const char * path)
{
    assert(p  != NULL);
    if (!path) {
        return;
    }

    struct Path * prev = p;
    while (prev->next) {
        prev = prev->next;
    }
    if (strncmp(path, "./", 2) == 0 || strncmp(path, "/", 1) == 0) {
        prev->next = make_path(path);
    } else {
        struct Path * tmp = make_path("./");
        add_str(&tmp->item, path);
        prev->next = tmp;
    }
}

void
clear_path(struct Path *p)
{
    assert(p != NULL);
    struct Path * cur = p->next;
    while (cur) {
        p->next = cur->next;
        if (cur->item.data) {
            free(cur->item.data);
        }
        free(cur);
        cur = p->next;
    }
}

struct Str get_path_str(const struct Path p) {
    struct Str path;
    path.data = NULL;
    path.size = path.cap = 0;

    struct Path * cur = p.next;
    while (cur) {
        if (path.size) {
            add_str(&path, ":");
        }
        add_str(&path, cur->item.data);
        cur = cur->next;
    }
    return path;
}

void
shell_error()
{
    const char * error_message = "An error has occurred\n";
    fprintf(stderr, "%s", error_message);
}

void *
Mmalloc(size_t size)
{
    void * ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "%s\n", "malloc failed");
        exit(1);
    }
    memset(ptr, 0, size);
    return ptr;
}

void
cd_main(int argc, char ** argv)
{
    assert(argv != NULL);
    if (argc != 2) {
        shell_error();
        return;
    }
    if (chdir(argv[1]) != 0) {
        shell_error();
    }
}

void
path_main(int argc, char **argv)
{
    clear_path(&g_path);
    for (int i = 1; i < argc; ++i) {
        add_to_path(&g_path, argv[i]);
    }
}

void
exit_main(int argc, char **argv)
{
    no_use(argv);
    if (argc != 1) {
        shell_error();
        return;
    }
    exit(0);
}

char **
parse_line(char * line, const char * delim, int * argc)
{
    int total = 0;
    for (int i = 0; line[i]; ++i) {
        for (int j = 0; delim[j]; ++j) {
            if (line[i] == delim[j]) {
                ++total;
                break;
            }
        }
    }
    total += 4;

    char **ap;
    char ** token = (char **)Mmalloc(sizeof(char *) * total);
    for (ap = token; (*ap = strsep(&line, delim)) != NULL; ) {
        if (**ap != 0) {
            if (++ap >= &token[total-1]) {
                break;
            }
        }
    }
    *ap = NULL;
    *argc = ap - token;
    return token;
}


void show_status(int argc, char **argv) {
    no_use(argc);
    no_use(argv);
    if (getcwd(buff, BUFF_SIZE) != NULL) {
        printf("cwd:%s\n", buff);
    }
    struct Str path = get_path_str(g_path);
    if (path.data) {
        printf("path:%s\n", path.data);
        free(path.data);
    }
}

typedef struct Command {
    int argc;
    char ** argv;
    const char * redirect_file;
    char wait;
    void (*builtin) (int argc, char **argv);
}Command;

Command *
parse_command(int argc, char ** argv, int * n)
{
    assert(n != NULL);
    *n = 0;
    Command * cmd = (Command *) Mmalloc(sizeof(Command) * argc);
    int l = 0, r = l + 1, ith = 0;
    for (; l < argc;) {
        Command tmp;
        tmp.argc = 0;
        tmp.argv = &argv[l];
        tmp.wait = 1;
        tmp.redirect_file = NULL;
        tmp.builtin = NULL;

        if (strcmp(argv[l], "&") == 0 || strcmp(argv[l], ">") == 0) {
            ++l;
            continue;
        } else if (strcmp(argv[l], "cd") == 0) {
            tmp.builtin = cd_main;
        } else if (strcmp(argv[l], "path") == 0) {
            tmp.builtin = path_main;
        } else if (strcmp(argv[l], "exit") == 0) {
            tmp.builtin = exit_main;
        } else if (strcmp(argv[l], "show") == 0) {
            tmp.builtin = show_status;
        }

        ++tmp.argc;
        for (r = l + 1; r < argc; ++r) {
            if (strcmp(argv[r], ">") == 0) {
                argv[r] = 0;
                if (r + 1 >= argc) {
                    free(cmd);
                    shell_error();
                    return NULL;
                }
                tmp.redirect_file = argv[++r];
                if (r+1 < argc && strncmp(argv[r+1], "&", 1) != 0) {
                        shell_error();
                        return NULL;
                }
            } else if (strcmp(argv[r], "&") == 0) {
                argv[r] = 0;
                tmp.wait = 0;
                ++r;
                break;
            }
            ++tmp.argc;
        }
        l = r;
        cmd[ith++] = tmp;
    }
    *n = ith;
    return cmd;
}

void
excetue_builtin(Command * cmd)
{
    assert(cmd != NULL && cmd->builtin != NULL);
    cmd->builtin(cmd->argc, cmd->argv);
}

pid_t
excetue_other(Command * cmd)
{
    assert(cmd != NULL);
    char **argv = cmd->argv;

    struct Path * cur = g_path.next;
    struct Str path;
    char * abs_path = NULL;

    if (argv[0][0] == '/' || (strncmp(argv[0], "./", 2) == 0)) {
        abs_path = strdup(argv[0]);
    }

    while (cur && abs_path == NULL) {
        path = make_str(cur->item.data);
        if (path.size) {
            if(path.data[path.size-1] != '/') {
                add_str(&path, "/");
            }
            add_str(&path, argv[0]);
            if (access(path.data, X_OK) == 0) {
                abs_path = path.data;
                break;
            }
        }
        free(path.data);
        cur = cur->next;
    }
    if (abs_path == NULL) {
        shell_error();
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (cmd->redirect_file) {
            freopen(cmd->redirect_file, "w", stdout);
            freopen(cmd->redirect_file, "w", stderr);
        }
        execv(abs_path, argv);
        shell_error();
    }
    if (cmd->wait &&  waitpid(pid, NULL, 0) < 0) {
        shell_error();
    }
    free(abs_path);
    return pid;
}

int
excetue_commands(Command * cmds, int n) {
    if (cmds == NULL) {
        return -1;
    }
    pid_t * pids = (pid_t *)Mmalloc(sizeof(pid_t) * n);
    int idx = 0;
    for (int i = 0; i < n; ++i) {
        if (cmds[i].builtin != NULL) {
            excetue_builtin(&cmds[i]);
            continue;
        }
        pid_t pid = excetue_other(&cmds[i]);
        if (pid > 0) {
            pids[idx++] = pid;
        }
    }

    for (int i = 0; i < idx; ++i) {
        waitpid(pids[i], NULL, 0);
    }
    free(pids);
    return 0;
}

void preprocess_input(char * buff)
{
    assert(buff != NULL);
    static char tmp[BUFF_SIZE];
    int idx = 0;
    for (int i = 0; buff[i]; ++i) {
        if (buff[i] == '>' || buff[i] == '&') {
            tmp[idx++] = ' ';
            tmp[idx++] = buff[i];
            tmp[idx++] = ' ';
            continue;
        }
        tmp[idx++] = buff[i];
    }
    memcpy(buff, tmp, idx);
    buff[idx] = 0;
}

int
main(int argc, char **argv)
{
    g_path.next = make_path("/bin");
    if (argc > 2) {
        shell_error();
        exit(1);
    }

    FILE * istream = stdin;
    if (argc == 2) {
        istream = fopen(argv[1], "r");
        if (istream == NULL) {
            shell_error();
            exit(1);
        }
    }

    while (1) {
        if (argc == 1) {
            printf("wish> ");
        }
        if (fgets(buff, BUFF_SIZE, istream) == NULL) {
            exit(0);
        }
        preprocess_input(buff);
        int n_parts = 0;
        char ** token = parse_line(buff, " \t\n;", &n_parts);
        if (argc == 0) {
            free(token);
            continue;
        }
        int n_cmds = 0;
        Command * cmds = parse_command(n_parts, token, &n_cmds);
        excetue_commands(cmds, n_cmds);
        free(cmds);
        free(token);
    }
    return 0;
}
