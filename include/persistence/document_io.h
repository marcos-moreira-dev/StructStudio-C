#ifndef SS_PERSISTENCE_DOCUMENT_IO_H
#define SS_PERSISTENCE_DOCUMENT_IO_H

#include "core/model.h"

int ss_document_save_json(const SsDocument *document, const char *path, SsError *error);
int ss_document_load_json(SsDocument *document, const char *path, SsError *error);

#endif
