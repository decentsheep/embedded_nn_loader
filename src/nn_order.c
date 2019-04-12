#include <string.h>
#include <stdlib.h>
#include "nn_order.h"

static int check_already_ordered(tree_node* cur_node, int* ordered, int ordered_cnt) {
    if (cur_node == NULL || ordered == NULL) {
        return -1;
    }
    for (int i = 0; i < ordered_cnt; i++) {
        if (ordered[i] == cur_node->tok_idx) {
            return 0;
        }
    }
    return 1;
}

static int check_inputs_done(char* json_buf, jsmntok_t* t, tree_node* root_tree,
                             tree_node* cur_node, int* ordered, int ordered_cnt) {
    //first, check if node is inputs node of nn, which has null in inputs field
    int res = jsmn_query_primitive(json_buf, t, cur_node->first_child, "inputs");
    if (res != -1) {
        if (strncmp(&json_buf[t[res].start], "null", strlen("null")) == 0) {
            return 0;
        }
        else {
            //invalid primitive
            return -1;
        }
    }
    else {
        res = jsmn_query_array(json_buf, t, cur_node->first_child, "inputs");
        if (res != -1) {
            tree_node* one_input = root_tree[res].first_child;
            //inputs field with an empty array like [] is allowed, it is same as null
            while(one_input != NULL) {
                int one_input_ordered = 0;
                int one_input_len = t[one_input->tok_idx].end - t[one_input->tok_idx].start;
                for (int i = 0; i < ordered_cnt; i++) {
                    int ordered_t_idx = ordered[i];
                    int one_ordered_output_idx = jsmn_query_array(json_buf, t,
                                                                  root_tree[ordered_t_idx].first_child,
                                                                  "outputs");
                    tree_node* one_ordered_output = root_tree[one_ordered_output_idx].first_child;
                    while(one_ordered_output != NULL) {
                        int one_ordered_output_len = t[one_ordered_output->tok_idx].end - t[one_ordered_output->tok_idx].start;
                        int cmp_len = (one_input_len > one_ordered_output_len ? one_input_len : one_ordered_output_len);
                        if (strncmp(&json_buf[t[one_input->tok_idx].start], &json_buf[t[one_ordered_output->tok_idx].start], cmp_len) == 0) {
                            one_input_ordered = 1;
                            break;
                        }
                        else {
                            one_ordered_output = one_ordered_output->sibling;
                        }
                    }
                }
                if (one_input_ordered == 0) {
                    //cur_node dependencies not done
                    return 1;
                }
                one_input = one_input->sibling;
            }
            //cur_node dependencies done
            return 0;
        }
        else {
            //should never fall into here
            return -1;
        }
    }
}

//1. every child of json root must be a k-v pair stands for a nn layer
//2. every nn layer must have inputs and outpus filed, which must be an array or null(primitive)
static int sanity_check(char* json_buf, jsmntok_t* t, tree_node* root_tree) {
    if (json_buf == NULL || t == NULL || root_tree == NULL || root_tree->child_num <= 0) {
        return -1;
    }
    tree_node *nn_layer = root_tree->first_child;
    //char json_path[64] = {0};
    int nn_layer_cnt = 0;
    while(nn_layer != NULL) {
        nn_layer_cnt++;

        //check inputs field
        if (jsmn_query_array(json_buf, t, nn_layer->first_child, "inputs") == -1 &&
            jsmn_query_primitive(json_buf, t, nn_layer->first_child, "inputs") == -1) {
            return -1;
        }
        //check outpus filed
        if (jsmn_query_array(json_buf, t, nn_layer->first_child, "outputs") == -1 &&
            jsmn_query_primitive(json_buf, t, nn_layer->first_child, "outputs") == -1) {
            return -1;
        }
        nn_layer = nn_layer->sibling;
    }
    return nn_layer_cnt;
}

int nn_order(char* json_buf, jsmntok_t* t, tree_node* root_tree,
             int** nn_ordered_ptr, int* nn_cnt) {
    if (t == NULL || root_tree == NULL ||
        nn_ordered_ptr == NULL || nn_cnt == NULL) {
        return -1;
    }

    int nn_layer_cnt = sanity_check(json_buf, t, root_tree);
    if (nn_layer_cnt == -1) {
        return -1;
    }

    int* nn_ordered = (int *) malloc(sizeof(int) * nn_layer_cnt);
    int nn_ordered_cnt = 0;
    int pre_nn_ordered_cnt = 0;
    tree_node* nn_layer;

    while(nn_ordered_cnt != nn_layer_cnt) {
        nn_layer = root_tree->first_child;

        while(nn_layer != NULL) {
            int nn_layer_already_ordered = check_already_ordered(nn_layer, nn_ordered, nn_ordered_cnt);
            if (nn_layer_already_ordered == -1) {
                return -1;
            }
            else if (nn_layer_already_ordered == 1) {
                int inputs_done = check_inputs_done(json_buf, t, root_tree, nn_layer, nn_ordered, nn_ordered_cnt);
                if (inputs_done == -1) {
                    return -1;
                }
                else if (inputs_done == 0) {
                    nn_ordered[nn_ordered_cnt] = nn_layer->tok_idx;
                    nn_ordered_cnt++;
                }
            }
            nn_layer = nn_layer->sibling;
        }
        //something wrong, maybe there is dependency lock
        if(pre_nn_ordered_cnt == nn_ordered_cnt) {
            return -1;
        }
        pre_nn_ordered_cnt = nn_ordered_cnt;
    }
    *nn_ordered_ptr = nn_ordered;
    *nn_cnt = nn_ordered_cnt;
    return 0;
}
