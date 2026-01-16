-- Test basic pipe syntax
SET allow_experimental_pipe_syntax = 1;

-- Test 1: Simple SELECT with WHERE pipe
SELECT number FROM system.numbers LIMIT 10 |> WHERE number > 5;

-- Test 2: Multiple pipe operations
SELECT number FROM system.numbers LIMIT 10 |> WHERE number > 3 |> WHERE number < 8;

-- Test 3: Pipe with ORDER BY
SELECT number FROM system.numbers LIMIT 10 |> WHERE number > 5 |> ORDER BY number DESC;

-- Test 4: Pipe with LIMIT
SELECT number FROM system.numbers LIMIT 100 |> WHERE number > 50 |> LIMIT 5;

-- Test 5: Pipe with SELECT (projection)
SELECT number, number * 2 AS doubled FROM system.numbers LIMIT 10 |> SELECT number, doubled |> WHERE number > 5;

-- Test 6: Pipe with EXTEND (add columns)
SELECT number FROM system.numbers LIMIT 5 |> EXTEND number * 2 AS doubled, number * 3 AS tripled;

-- Test 7: Pipe with AGGREGATE
SELECT number FROM system.numbers LIMIT 100 |> AGGREGATE sum(number) AS total, count() AS cnt;

-- Test 8: Pipe with AGGREGATE and GROUP BY
SELECT number % 10 AS grp, number FROM system.numbers LIMIT 100 |> AGGREGATE sum(number) AS total GROUP BY grp |> ORDER BY grp;

-- Test 9: Pipe with DISTINCT
SELECT number % 5 AS val FROM system.numbers LIMIT 20 |> DISTINCT |> ORDER BY val;

-- Test 10: Complex pipeline
SELECT number FROM system.numbers LIMIT 1000
    |> WHERE number > 100
    |> EXTEND number * number AS square
    |> WHERE square < 200000
    |> AGGREGATE avg(number) AS avg_num, max(square) AS max_square GROUP BY number % 10 AS mod_10
    |> WHERE avg_num > 150
    |> ORDER BY avg_num DESC
    |> LIMIT 5;
