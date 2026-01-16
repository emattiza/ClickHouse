-- Test edge cases and error handling for pipe syntax
SET allow_experimental_pipe_syntax = 1;

-- Empty result after filtering
SELECT number FROM system.numbers LIMIT 5 |> WHERE number > 100;

-- Single row result
SELECT number FROM system.numbers LIMIT 10 |> WHERE number = 5;

-- Pipe with no operations (should work as regular query)
SELECT number FROM system.numbers LIMIT 3;

-- Multiple consecutive WHEREs (AND semantics)
SELECT number FROM system.numbers LIMIT 20 |> WHERE number > 5 |> WHERE number < 15 |> WHERE number % 2 = 0;

-- ORDER BY followed by another ORDER BY (second should override)
SELECT number FROM system.numbers LIMIT 10 |> ORDER BY number DESC |> ORDER BY number ASC;

-- LIMIT followed by another LIMIT (second should apply to result of first)
SELECT number FROM system.numbers LIMIT 100 |> LIMIT 10 |> LIMIT 3;

-- SELECT that reduces columns, then EXTEND to add back
SELECT number, number * 2 AS doubled FROM system.numbers LIMIT 5 |> SELECT number |> EXTEND number + 100 AS big;

-- Aggregate without GROUP BY (full table aggregation)
SELECT number FROM system.numbers LIMIT 10 |> AGGREGATE sum(number) AS total;

-- DISTINCT followed by ORDER BY
SELECT number % 3 AS val FROM system.numbers LIMIT 10 |> DISTINCT |> ORDER BY val;

-- Complex expression in WHERE
SELECT number FROM system.numbers LIMIT 20 |> WHERE (number > 5 AND number < 15) OR number = 0;

-- Aliased expressions preserved through pipe
SELECT number AS n, number * 2 AS doubled FROM system.numbers LIMIT 5 |> WHERE n > 2 |> SELECT n, doubled;
