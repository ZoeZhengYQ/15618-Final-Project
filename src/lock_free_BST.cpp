//
// Created by Zoeeeee on 4/24/20.
//

#include "lock_free_BST.h"

/* Will need to create record wherever needed */

// object to store information about the tree traversal when looking for a given key (used by the seek function)
// SeekRecordPtr targetRecord := new seek record;
// object to store information about process’ own delete operation
// StateRecordPtr myState := new state;

bool LockFreeBST::search(int v) {
    SeekRecord seek_record;
    seek(v, &seek_record);
    cout << GET_KEY_VAL(seek_record.last_edge.child->key) << endl;
    if(GET_KEY_VAL(seek_record.last_edge.child->key) == v) return true;
    return false;
}


bool LockFreeBST::insert(int v) {
    // TODO: use one target record or individual seek record?
    seek(v, &target_record);
    LFTreeEdge target_edge = target_record.last_edge;
    LFTreeNode* node = target_edge.child;
    if(GET_KEY_VAL(node->key) == v) return false;  // Exists duplicate node

    // Create new node
    EdgeType which_edge = target_record.inject_edge.type;
    LFTreeNode::LFNodeChild* old_child = new LFTreeNode::LFNodeChild(target_record.inject_edge.child);
    SET_NULL_FLG(old_child);
    LFTreeNode::LFNodeChild* new_child = new LFTreeNode::LFNodeChild(new LFTreeNode(v));
    LFTreeNode::LFNodeChild** target_addr = which_edge == EdgeType::LEFT ? &node->left : &node->right;
    bool result = __sync_bool_compare_and_swap(target_addr, old_child, new_child);
    if(result) {
        cout << "succ insert " << v << endl;
        return true;
    }
    cout << "fail to insert " << v << endl;
    // TODO: helper function
    return false;
}


bool LockFreeBST::remove(int v) {
    // TODO: Which seek record to use?
    StateRecord *state = new StateRecord();
    // Initialize remove state record
    state->target_key = v;
    state->current_key = v;
    state->mode = DelMode::INJECT;  // Start with inject mode

    while (1) {
        seek(state->target_key, &target_record);
        LFTreeEdge target_edge = target_record.last_edge;
        LFTreeEdge p_target_edge = target_record.p_last_edge;
//        TODO: Do we still need LFNodeKey struct? steal a bit from int instead?
//        TODO: Implement a get value for key
//        LFNodeKey target_edge_child_key = target_edge.child->key;
        if (state->current_key != get_value(target_edge_child_key)) {
            // the key does not exist in the tree
            if (state->mode == DelMode::INJECT) {
                return false;
            }
            return true;
        }

        // perform appropriate action depending on the mode
        if (state->mode == DelMode::INJECT) {
            // store a reference to the target edge
            state->target_edge = target_edge;
            state->p_target_edge = p_target_edge;
            // attempt to inject the operation at the node Inject( myState );
//            TODO: Define the inject function
//            inject(state);
        }

        // mode would have changed if injection was successful
        if (state->mode != DelMode::INJECT) {
            // check if the target node found by the seek function
            // matches the one stored in the state record

        }
    }
    return false;
}


void LockFreeBST::seek(int target_key, SeekRecord *seek_record) {
    AnchorRecord prev_anchor_record(root_s, GET_KEY_VAL(root_s->key));
    SeekRecord p_seek_record;
    while(true) {
        // Initialize all variables used in traversal
        LFTreeEdge p_last_edge(root_r, root_s, EdgeType::RIGHT);
        LFTreeEdge last_edge(root_s, root_t, EdgeType::RIGHT);
        LFTreeNode *cur = root_t;
        AnchorRecord anchor_record(root_s, GET_KEY_VAL(root_s->key));

        while(true) {
            int cur_key = GET_KEY_VAL(cur->key);
            EdgeType which_edge = target_key > cur_key ? EdgeType::RIGHT : EdgeType::LEFT;
            LFTreeNode::LFNodeChild* raw_child = which_edge == EdgeType::LEFT ? cur->left : cur->right;
            LFTreeNode *next = GET_CHILD_NODE(raw_child);

            // If either key found or no next edge to follow, stop the traversal
            if(cur_key == target_key || GET_NULL_FLG(raw_child)) {
                seek_record->p_last_edge = p_last_edge;
                seek_record->last_edge = last_edge;
                seek_record->inject_edge = LFTreeEdge(cur, next, which_edge);
                if(cur_key == target_key) return;
                else break;
            }

            // Update anchor node if next edge to be traversed is a right edge
            if(which_edge == EdgeType::RIGHT) {
                anchor_record.node = cur;
                anchor_record.key_val = cur_key;
            }

            // Traverse the next edge
            p_last_edge = last_edge;
            last_edge = LFTreeEdge(cur, next, which_edge);
            cur = next;
        }

        // Key was not found; check if can stop
        LFTreeNode* anchor_node = anchor_record.node;
        cout << anchor_node->right << endl;
        if(!GET_DELETE_FLG(anchor_node->right) && !GET_PROMOTE_FLG(anchor_node->right)) {
            // The anchor node is still part of the tree; check if the anchor node’s key has changed
            if(anchor_record.key_val == GET_KEY_VAL(anchor_node->key)) return;
        } else {
            // Check if the anchor record (the node and its key) matches that of the previous traversal
            if(prev_anchor_record == anchor_record) {
                // return the results of the previous traversal
                *seek_record = p_seek_record;
                return;
            }
        }

        // Store the results of the traversal and restart from root
        p_seek_record = *seek_record;
        prev_anchor_record = anchor_record;
    }
}


//TODO
void LockFreeBST::reinitialize() {

}


void LockFreeBST::destroy(LFTreeNode *cur) {

}


vector<int> LockFreeBST::trans2vec() {

}


void LockFreeBST::trans2vec_helper(LFTreeNode *cur, vector<int> &v) {

}
