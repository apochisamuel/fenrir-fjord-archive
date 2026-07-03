#include "internal.h"

fr_status_t fr_build_caches(fr_document_t *doc) {
    fr_status_t status;
    if (doc == NULL) return FR_E_ARGUMENT;
    if ((status = fr_prepare_name_cache(doc)) != FR_OK) return status;
    if ((status = fr_prepare_route_cache(doc)) != FR_OK) return status;
    if ((status = fr_prepare_metric_cache(doc)) != FR_OK) return status;
    if ((status = fr_prepare_group_cache(doc)) != FR_OK) return status;
    if ((status = fr_prepare_resource_cache(doc)) != FR_OK) return status;
    doc->state_bits |= 1024u;
    return FR_OK;
}
