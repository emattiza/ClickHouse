-- Additional edge cases for pipe syntax
SET allow_experimental_pipe_syntax = 1;

-- 1. Nested pipes in subqueries
SELECT * FROM (
    FROM system.numbers |> LIMIT 10 |> WHERE number % 2 = 0
) |> WHERE number > 5;

-- 2. Pipe with CTE (Common Table Expression)
WITH cte AS (
    SELECT number FROM system.numbers LIMIT 20
)
SELECT * FROM cte |> WHERE number > 10 |> LIMIT 5;

-- 3. Pipe with UNION ALL (inside subquery)
SELECT * FROM (
    SELECT number FROM system.numbers LIMIT 5
    UNION ALL
    SELECT number + 10 FROM system.numbers LIMIT 5
) |> WHERE number > 2 |> ORDER BY number;

-- 4. Pipe with JOIN and complex aliases
SET joined_subquery_requires_alias = 0;
FROM system.numbers AS n1
|> LIMIT 5
|> SELECT number AS a
|> JOIN (SELECT number AS b FROM system.numbers LIMIT 5) AS n2 ON a = b
|> EXTEND a + b AS sum_ab
|> WHERE sum_ab > 4;

-- 5. Pipe with WINDOW functions and multiple steps
FROM system.numbers
|> LIMIT 10
|> EXTEND number % 2 AS g
|> SELECT number, g, sum(number) OVER (PARTITION BY g ORDER BY number) AS running_sum
|> WHERE running_sum > 5;

-- 6. Pipe with WHERE (PREWHERE is not supported on subqueries)
FROM system.numbers |> WHERE number > 10 |> LIMIT 5;

-- 7. Pipe with SET and DROP
FROM system.numbers
|> LIMIT 5
|> EXTEND number AS x, number AS y
|> SET x = x * 10
|> DROP y
|> SELECT x;

-- 8. Persistence check (SHOW CREATE VIEW)
-- This test documents that pipe syntax is currently stored as desugared nested subqueries.
DROP TABLE IF EXISTS v_pipe_persistence;
CREATE VIEW v_pipe_persistence AS FROM system.numbers |> LIMIT 5;
SHOW CREATE VIEW v_pipe_persistence;
SELECT * FROM v_pipe_persistence;
DROP TABLE v_pipe_persistence;
