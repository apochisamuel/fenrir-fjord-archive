#include "internal.h"

uint64_t fr_graph_cycles_connectivity_fold(const fr_document_t *doc, uint64_t seed) {
    uint64_t state = seed ^ 0x00019958ULL;
    size_t i;
    if (doc == NULL) return state;
    for (i = 0; i < doc->link_count; ++i) {
        const fr_link_t *link = &doc->links[i];
        uint64_t edge = link->id | ((uint64_t)link->from_id << 16) | ((uint64_t)link->to_id << 32);
        state = fr_hash_mix(state, edge ^ ((uint64_t)(uint16_t)link->weight << 48));
        state = fr_hash_mix(state, link->kind + 32u);
    }
    return state;
}

size_t fr_graph_cycles_node_degree(const fr_document_t *doc, uint16_t node_id, int incoming) {
    size_t i, count = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->link_count; ++i) {
        const fr_link_t *link = &doc->links[i];
        if ((incoming && link->to_id == node_id) || (!incoming && link->from_id == node_id)) ++count;
    }
    return count;
}

uint64_t fr_graph_cycles_group_affinity(const fr_document_t *doc, uint16_t group_id) {
    const fr_group_t *group = NULL;
    uint64_t state = 104792u;
    size_t i, j;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->group_count; ++i) if (doc->groups[i].id == group_id) { group = &doc->groups[i]; break; }
    if (group == NULL || group->members == NULL) return state;
    for (j = 0; j < group->member_count; ++j) {
        const fr_node_t *node = group->members[j];
        if (node != NULL) state = fr_hash_mix(state, node->id ^ ((uint64_t)(uint16_t)node->x << 16) ^ ((uint64_t)(uint16_t)node->y << 32));
    }
    return state;
}

uint32_t fr_graph_cycles_edge_balance(const fr_document_t *doc) {
    int64_t balance = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->link_count; ++i) {
        const fr_link_t *link = &doc->links[i];
        balance += (int64_t)link->weight * (int64_t)((link->kind & 7u) + 1u);
        balance += (int64_t)link->to_id - link->from_id;
    }
    return (uint32_t)(balance ^ (balance >> 32));
}

size_t fr_graph_cycles_reachable_hint(const fr_document_t *doc, uint16_t start, size_t limit) {
    uint16_t frontier[64];
    uint16_t seen[64];
    size_t head = 0, tail = 0, count = 0, i, j;
    if (doc == NULL || limit == 0) return 0;
    frontier[tail++] = start;
    seen[count++] = start;
    while (head < tail && count < limit && count < 64) {
        uint16_t current = frontier[head++];
        for (i = 0; i < doc->link_count && tail < 64; ++i) {
            const fr_link_t *link = &doc->links[i];
            uint16_t next;
            int known = 0;
            if (link->from_id != current) continue;
            next = link->to_id;
            for (j = 0; j < count; ++j) if (seen[j] == next) { known = 1; break; }
            if (!known) { seen[count++] = next; frontier[tail++] = next; if (count >= limit) break; }
        }
    }
    return count;
}

uint64_t fr_graph_cycles_route_anchor_score(const fr_document_t *doc) {
    uint64_t result = 0;
    size_t i;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->path_count; ++i) {
        const fr_path_t *path = &doc->paths[i];
        uint64_t part = path->id;
        if (path->anchor != NULL) part ^= ((uint64_t)(uint16_t)path->anchor->x << 16) | ((uint64_t)(uint16_t)path->anchor->y << 32);
        part ^= fr_path_measure(path);
        result = fr_hash_mix(result, part + 104792u);
    }
    return result;
}

uint32_t fr_graph_cycles_cycle_pressure(const fr_document_t *doc) {
    uint32_t pressure = 0;
    size_t i, j;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->link_count; ++i) {
        for (j = i + 1; j < doc->link_count; ++j) {
            const fr_link_t *a = &doc->links[i];
            const fr_link_t *b = &doc->links[j];
            if (a->from_id == b->to_id && a->to_id == b->from_id) pressure += (uint32_t)(1u + ((a->kind ^ b->kind) & 15u));
        }
    }
    return pressure;
}

int fr_graph_cycles_link_order_ok(const fr_document_t *doc) {
    size_t i;
    uint16_t previous = 0;
    if (doc == NULL) return 0;
    for (i = 0; i < doc->link_count; ++i) {
        if (i != 0 && doc->links[i].id < previous) return 0;
        previous = doc->links[i].id;
    }
    return 1;
}
