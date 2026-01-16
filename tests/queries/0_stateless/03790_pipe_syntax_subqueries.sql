-- Test pipe syntax with subqueries
SET allow_experimental_pipe_syntax = 1;

-- Basic pipe syntax (subqueries are created internally)
SELECT number FROM system.numbers LIMIT 10 |> WHERE number > 5 |> ORDER BY number;

-- Pipe with multiple operations creates nested subqueries
SELECT number, number * 2 AS doubled FROM system.numbers LIMIT 10
    |> WHERE number > 3
    |> SELECT doubled
    |> WHERE doubled < 16;
