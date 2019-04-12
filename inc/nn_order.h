#ifndef _NN_ORDER_H_
#define _NN_ORDER_H_

#include "jsmn_util.h"


#ifdef __cplusplus
extern "C" {
#endif

int nn_order(char* json_buf, jsmntok_t* t, tree_node* root_tree, int** nn_ordered_ptr, int* nn_cnt);

#ifdef __cplusplus
}
#endif

#endif /* _NN_ORDER_H_*/
