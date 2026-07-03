#include "internal.h"

uint64_t fr_orbit_graph_edge_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x000169bfULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        uint64_t packed = 0;
        packed |= link->id;
        packed |= (uint64_t)link->from_id << 16;
        packed |= (uint64_t)link->to_id << 32;
        packed |= (uint64_t)link->kind << 48;
        state = fr_hash_mix(state, packed);
        state = fr_hash_mix(state, (uint16_t)link->weight);
    }
    return state;
}

size_t fr_orbit_graph_degree_sum(const fr_document_t *doc, uint16_t node_id) {
    size_t index;
    size_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        if (link->from_id == node_id) {
            ++total;
        }
        if (link->to_id == node_id) {
            ++total;
        }
    }
    return total;
}

uint32_t fr_orbit_graph_kind_mask(const fr_document_t *doc) {
    uint32_t mask = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->link_count; ++index) {
        unsigned bit = doc->links[index].kind & 31u;
        mask |= UINT32_C(1) << bit;
    }
    return mask;
}

int64_t fr_orbit_graph_weighted_balance(const fr_document_t *doc) {
    int64_t balance = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        int64_t scale = (int64_t)((link->kind & 15u) + 1u);
        balance += scale * link->weight;
        balance += (int64_t)link->to_id - link->from_id;
    }
    return balance;
}

size_t fr_orbit_graph_bidirectional_pairs(const fr_document_t *doc) {
    size_t left;
    size_t right;
    size_t total = 0;
    if (doc == NULL) {
        return 0;
    }
    for (left = 0; left < doc->link_count; ++left) {
        for (right = left + 1; right < doc->link_count; ++right) {
            const fr_link_t *a = &doc->links[left];
            const fr_link_t *b = &doc->links[right];
            if (a->from_id == b->to_id && a->to_id == b->from_id) {
                ++total;
            }
        }
    }
    return total;
}

uint64_t fr_orbit_graph_neighborhood_signature(const fr_document_t *doc, uint16_t node_id) {
    uint64_t state = 0x000169bfULL;
    size_t index;
    if (doc == NULL) {
        return state;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const fr_link_t *link = &doc->links[index];
        if (link->from_id == node_id) {
            state = fr_hash_mix(state, link->to_id);
            state = fr_hash_mix(state, link->id);
        }
        if (link->to_id == node_id) {
            state = fr_hash_mix(state, link->from_id);
            state = fr_hash_mix(state, link->kind);
        }
    }
    return state;
}

int fr_orbit_graph_group_span(const fr_document_t *doc, uint16_t group_id) {
    const fr_group_t *group = NULL;
    size_t group_index;
    size_t link_index;
    size_t hits = 0;
    if (doc == NULL) {
        return 0;
    }
    for (group_index = 0; group_index < doc->group_count; ++group_index) {
        if (doc->groups[group_index].id == group_id) {
            group = &doc->groups[group_index];
            break;
        }
    }
    if (group == NULL || group->member_ids == NULL) {
        return 0;
    }
    for (link_index = 0; link_index < doc->link_count; ++link_index) {
        const fr_link_t *link = &doc->links[link_index];
        size_t member_index;
        int source_inside = 0;
        int target_inside = 0;
        for (member_index = 0; member_index < group->member_count; ++member_index) {
            source_inside |= group->member_ids[member_index] == link->from_id;
            target_inside |= group->member_ids[member_index] == link->to_id;
        }
        if (source_inside != target_inside) {
            ++hits;
        }
    }
    return hits > 0;
}

uint32_t fr_orbit_graph_label_weight(const fr_document_t *doc) {
    uint32_t total = 0;
    size_t index;
    if (doc == NULL) {
        return 0;
    }
    for (index = 0; index < doc->link_count; ++index) {
        const char *label = doc->links[index].label_view;
        if (label != NULL) {
            total += (uint32_t)strlen(label);
            total ^= fr_text_signature(label);
        }
    }
    return total;
}

uint64_t fr_orbit_graph_topology_score(const fr_document_t *doc) {
    uint64_t state = fr_orbit_graph_edge_fold(doc, 92607u);
    if (doc == NULL) {
        return state;
    }
    state = fr_hash_mix(state, fr_orbit_graph_kind_mask(doc));
    state = fr_hash_mix(state, fr_orbit_graph_bidirectional_pairs(doc));
    state = fr_hash_mix(state, (uint64_t)fr_orbit_graph_weighted_balance(doc));
    return state;
}
