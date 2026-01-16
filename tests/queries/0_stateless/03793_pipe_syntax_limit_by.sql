-- Test pipe syntax with LIMIT BY (ClickHouse-specific feature)
SET allow_experimental_pipe_syntax = 1;

-- Basic LIMIT BY through pipe
SELECT number % 3 AS group_id, number
FROM system.numbers
LIMIT 20
|> LIMIT 2 BY group_id
|> ORDER BY group_id, number;

-- LIMIT BY with OFFSET
SELECT number % 4 AS grp, number
FROM system.numbers
LIMIT 20
|> LIMIT 1 OFFSET 1 BY grp
|> ORDER BY grp, number;

-- Combining LIMIT BY with regular LIMIT
SELECT number % 5 AS category, number
FROM system.numbers
LIMIT 30
|> LIMIT 3 BY category
|> LIMIT 10
|> ORDER BY category, number;
