#include <stdlib.h>
#include <stdio.h>
#include "jsmn_util.h"
#include "nn_order.h"

int main() {
    int r;
    jsmntok_t* t;
    tree_node* root_tree;

    FILE* fp = fopen("example_cnn.json", "r");
    if (fp == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *json_buf = malloc(sizeof(char) * fsize);
    fread(json_buf, sizeof(char), fsize, fp);

    r = json_parse(json_buf, fsize, &t, &root_tree);
#if 0
    for (int i = 0; i < r; i++) {
        printf("********** tok: %d **********\n", i);
        printf("%.*s\n", t[i].end - t[i].start, &json_buf[t[i].start]);
        printf("parent: %d\n", t[i].parent);
    }
#endif

    int* nn_ordered = NULL;
    int nn_cnt = 0;
    int ret = nn_order(json_buf, t, root_tree, &nn_ordered, &nn_cnt);
    if (ret == 0) {
        printf("********** reordered network **********\n");
        for (int i = 0; i < nn_cnt; i++) {
            printf("%.*s----->", t[nn_ordered[i]].end - t[nn_ordered[i]].start, &json_buf[t[nn_ordered[i]].start]);
        }
        printf("\n");
    }


    fclose(fp);
    free(json_buf);
    free(t);
    free(root_tree);
    free(nn_ordered);
    return 0;
}

