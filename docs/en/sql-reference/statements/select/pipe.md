---
description: 'Documentation for Pipe Syntax'
sidebar_label: 'Pipe Syntax'
slug: /sql-reference/statements/select/pipe
title: 'Pipe Syntax'
doc_type: 'reference'
---

# Pipe Syntax

Pipe syntax allows you to write queries as a chain of operations using the `|>` operator. Instead of nesting subqueries or writing complex single statements, you can express data transformations as a sequence of steps that read naturally from left to right and top to bottom.

:::note
This is an experimental feature. To enable it, use `SET allow_experimental_pipe_syntax = 1`.
:::

## Syntax

```sql
FROM <table> |> <pipe_operator> [|> <pipe_operator> ...]
```

Or starting with a SELECT:

```sql
<select_query> |> <pipe_operator> [|> <pipe_operator> ...]
```

## Motivation

Traditional SQL often requires nested subqueries for multi-step transformations:

```sql
SELECT * FROM (
    SELECT * FROM (
        SELECT * FROM t WHERE x > 10
    ) ORDER BY y
) LIMIT 5
```

With pipe syntax, the same query becomes:

```sql
FROM t |> WHERE x > 10 |> ORDER BY y |> LIMIT 5
```

## Supported Pipe Operators

### Filtering and Selection

| Operator | Description | Example |
|----------|-------------|---------|
| `WHERE` | Filter rows | `|> WHERE x > 10` |
| `SELECT` | Project columns | `|> SELECT a, b, c` |
| `DISTINCT` | Remove duplicate rows | `|> DISTINCT` |

### Ordering and Limiting

| Operator | Description | Example |
|----------|-------------|---------|
| `ORDER BY` | Sort rows | `|> ORDER BY x DESC` |
| `LIMIT` | Limit number of rows | `|> LIMIT 10` |
| `OFFSET` | Skip rows | `|> OFFSET 5` |
| `LIMIT BY` | Limit rows per group | `|> LIMIT 3 BY category` |

### Aggregation

| Operator | Description | Example |
|----------|-------------|---------|
| `AGGREGATE` | Aggregate with optional grouping | `|> AGGREGATE sum(x) AS total GROUP BY y` |
| `HAVING` | Filter after aggregation | `|> HAVING total > 100` |

### Column Manipulation

| Operator | Description | Example |
|----------|-------------|---------|
| `EXTEND` | Add computed columns | `|> EXTEND x * 2 AS doubled` |
| `SET` | Replace column values | `|> SET x = x + 1` |
| `DROP` | Remove columns | `|> DROP temp_col` |

### Joins

| Operator | Description | Example |
|----------|-------------|---------|
| `JOIN` | Join with another table | `|> JOIN other_table ON t.id = other_table.id` |

All join types are supported: `INNER`, `LEFT`, `RIGHT`, `FULL`, `CROSS`, `SEMI`, `ANTI`, `ASOF`, with optional `GLOBAL` modifier.

:::note
`PREWHERE` and named `WINDOW` clauses are currently not supported as standalone pipe operators due to the underlying subquery transformation. Use `WHERE` and inline window functions instead.
:::

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `allow_experimental_pipe_syntax` | `false` | Enable pipe syntax support |

## Examples

### FROM-First Syntax

```sql
SET allow_experimental_pipe_syntax = 1;

FROM system.numbers
    |> WHERE number > 50
    |> ORDER BY number DESC
    |> LIMIT 10;
```

### FROM with Subquery

```sql
SET allow_experimental_pipe_syntax = 1;

FROM (SELECT number, number * 2 AS doubled FROM system.numbers LIMIT 100)
    |> WHERE doubled > 100
    |> ORDER BY number
    |> LIMIT 5;
```

### With CTE

```sql
SET allow_experimental_pipe_syntax = 1;

WITH source AS (SELECT number FROM system.numbers LIMIT 100)
SELECT * FROM source
    |> WHERE number > 50
    |> LIMIT 10;
```

### Aggregation with Grouping

```sql
SET allow_experimental_pipe_syntax = 1;

FROM system.numbers
    |> LIMIT 50
    |> EXTEND number % 5 AS grp
    |> AGGREGATE sum(number) AS total, count() AS cnt GROUP BY grp
    |> ORDER BY grp;
```

### Adding Computed Columns

```sql
SET allow_experimental_pipe_syntax = 1;

FROM system.numbers
    |> LIMIT 5
    |> EXTEND number * 2 AS doubled, number * 3 AS tripled
    |> SELECT number, doubled, tripled;
```

### Joining Tables

```sql
SET allow_experimental_pipe_syntax = 1;
SET joined_subquery_requires_alias = 0;

FROM system.numbers
    |> LIMIT 5
    |> SELECT number AS a
    |> JOIN (SELECT number AS b FROM system.numbers LIMIT 5) ON a = b
    |> SELECT a, b;
```

### Complex Pipeline

```sql
SET allow_experimental_pipe_syntax = 1;

FROM system.numbers
    |> LIMIT 1000
    |> WHERE number > 100
    |> EXTEND number * number AS square
    |> WHERE square < 200000
    |> AGGREGATE avg(number) AS avg_num, max(square) AS max_sq GROUP BY number % 10 AS mod
    |> WHERE avg_num > 200
    |> ORDER BY avg_num DESC
    |> LIMIT 5;
```

### Window Functions

```sql
SET allow_experimental_pipe_syntax = 1;

FROM system.numbers
    |> LIMIT 9
    |> EXTEND number % 3 AS grp
    |> SELECT number, grp, row_number() OVER (PARTITION BY grp ORDER BY number) AS rn
    |> ORDER BY grp, rn;
```

## See Also

- [SELECT](/sql-reference/statements/select)
- [WHERE](/sql-reference/statements/select/where)
- [ORDER BY](/sql-reference/statements/select/order-by)
- [GROUP BY](/sql-reference/statements/select/group-by)
- [JOIN](/sql-reference/statements/select/join)
