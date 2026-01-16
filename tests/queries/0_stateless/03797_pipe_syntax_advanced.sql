-- Test advanced pipe syntax features
SET allow_experimental_pipe_syntax = 1;

-- Test HAVING after AGGREGATE
SELECT number % 5 AS grp, number FROM system.numbers LIMIT 50
    |> AGGREGATE sum(number) AS total, count() AS cnt GROUP BY grp
    |> HAVING total > 100
    |> ORDER BY grp;

-- Test DROP columns
SELECT number, number * 2 AS doubled, number * 3 AS tripled FROM system.numbers LIMIT 5
    |> DROP doubled
    |> ORDER BY number;

-- Test multiple HAVING conditions
SELECT number % 3 AS grp, number FROM system.numbers LIMIT 30
    |> AGGREGATE avg(number) AS avg_val, max(number) AS max_val GROUP BY grp
    |> HAVING avg_val > 5 AND max_val > 20
    |> ORDER BY grp;
