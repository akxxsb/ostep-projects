#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct text_node {
    char * data;
    size_t size;
} text_node;

typedef struct list_node {
    void * data;
    struct list_node * next;
}list_node;

typedef struct list {
    list_node * head;
}list;

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

list *
create_text_list()
{
    list * tlist = (list *)Mmalloc(sizeof(list));
    tlist->head = (list_node *) Mmalloc(sizeof(list_node));
    tlist->head->next = NULL;
    tlist->head->data = NULL;
    return tlist;
}

void
free_text_list(list * tlist)
{
    if (tlist == NULL) {
        return;
    }
    for (list_node * cur = tlist->head; cur != NULL;) {
        if (cur->data) {
            text_node * node = (text_node *) cur->data;
            if (node->data) {
                free(node->data);
            }
            free(node);
        }
        list_node * next = (list_node *)cur->next;
        free(cur);
        cur = next;
    }
    free(tlist);
}

void
insert_text_list(list * tlist, text_node * data)
{
    assert(tlist != NULL);
    list_node * node = (list_node *) Mmalloc(sizeof(list_node));
    node->data = data;
    node->next = tlist->head->next;
    tlist->head->next = node;
}

void
print_text_list(list * tlist, FILE * fp)
{
    assert(tlist != NULL);
    assert(tlist->head != NULL);
    for (list_node * cur = tlist->head->next; cur != NULL; cur = cur->next) {
        text_node * node = (text_node *) cur->data;
        if (node && node->data) {
            fprintf(fp, "%s", (char *) node->data);
        }
    }
}

void
reverse_main(const char * filename, const char * outfile)
{
    FILE * fp = stdin, *ofp = stdout;
    if (filename) {
        fp = fopen(filename, "r");
    }
    if (fp == NULL) {
        fprintf(stderr, "reverse: cannot open file '%s'\n", filename);
        exit(1);
    }

    if (outfile) {
        ofp = fopen(outfile, "w");
    }
    if (ofp == NULL) {
        fprintf(stderr, "reverse: cannot open file '%s'\n", outfile);
        exit(1);
    }

    list * tlist = create_text_list();
    while (1) {
        text_node * node = (text_node *) Mmalloc(sizeof(text_node));
        node->data = NULL;
        node->size = 0;
        int rc = getline(&node->data, &node->size, fp);
        insert_text_list(tlist, node);
        if (rc == -1 || rc == EOF) {
            break;
        }
    }
    print_text_list(tlist, ofp);
    free_text_list(tlist);
    fclose(fp);
    fclose(ofp);
}

int
main(int argc, char **argv)
{
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    } else if (argc == 3) {
        if (strcmp(argv[1], argv[2]) == 0) {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
        reverse_main(argv[1], argv[2]);
    } else if (argc == 2) {
        reverse_main(argv[1], NULL);
    } else {
        reverse_main(NULL, NULL);
    }
    return 0;
}
