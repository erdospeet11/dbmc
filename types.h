#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    COLUMN_TYPE_INT,
    COLUMN_TYPE_STRING,
    COLUMN_TYPE_FLOAT
} column_type_t;

typedef struct {
    char name[64];
    column_type_t type;
    size_t size;
    int is_nullable;
    int is_primary_key;
} column_def_t;

typedef struct {
    char table_name[64];
    column_def_t *columns;
    int column_count;
    int max_columns;
} table_schema_t;

typedef struct {
    void **column_data;
    size_t *column_sizes;
    int is_deleted;
} row_t;

#define PAGE_SIZE 4096
#define MAX_ROWS_PER_PAGE 100

typedef struct {
    uint32_t page_id;
    uint32_t next_page;
    uint16_t row_count;
    uint16_t free_space_offset;
    row_t rows[MAX_ROWS_PER_PAGE];
    char data[PAGE_SIZE - sizeof(uint32_t)*2 - sizeof(uint16_t)*2 - sizeof(row_t)*MAX_ROWS_PER_PAGE];
} page_t;

typedef struct {
    table_schema_t schema;
    page_t **pages;
    int page_count;
    int max_pages;
    FILE *data_file;
} table_t;

typedef struct {
    table_t **tables;
    int table_count;
    int max_tables;
    char db_name[64];
} database_t;

table_t* create_table(const char* name, column_def_t* columns, int column_count);
int insert_row(table_t *table, void **column_values);
void scan_table(table_t *table);

#endif // TYPES_H