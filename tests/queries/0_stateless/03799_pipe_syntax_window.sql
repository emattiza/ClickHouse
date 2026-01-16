-- Test pipe WINDOW operator
SET allow_experimental_pipe_syntax = 1;

-- Window functions with inline window definition
SELECT number, number % 3 AS grp FROM system.numbers LIMIT 9
    |> SELECT number, grp, row_number() OVER (PARTITION BY grp ORDER BY number) AS rn
    |> ORDER BY grp, rn;

-- Multiple window functions
SELECT number, number % 2 AS odd FROM system.numbers LIMIT 6
    |> SELECT number, odd, 
        row_number() OVER (ORDER BY number) AS global_rn,
        row_number() OVER (PARTITION BY odd ORDER BY number) AS partition_rn
    |> ORDER BY number;

-- Window with aggregation
SELECT number % 4 AS grp, number FROM system.numbers LIMIT 12
    |> SELECT grp, number, sum(number) OVER (PARTITION BY grp ORDER BY number) AS running_sum
    |> ORDER BY grp, number;
