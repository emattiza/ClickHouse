-- Test pipe JOIN operator
SET allow_experimental_pipe_syntax = 1;
SET joined_subquery_requires_alias = 0;

-- Basic INNER JOIN through pipe
SELECT number AS a FROM system.numbers LIMIT 5
    |> JOIN (SELECT number AS b FROM system.numbers LIMIT 5) ON a = b
    |> ORDER BY a;

-- LEFT JOIN through pipe
SELECT number AS a FROM system.numbers LIMIT 5
    |> LEFT JOIN (SELECT number + 2 AS b FROM system.numbers LIMIT 3) ON a = b
    |> ORDER BY a;

-- Multiple JOINs through pipe
SELECT number AS x FROM system.numbers LIMIT 4
    |> JOIN (SELECT number AS y FROM system.numbers LIMIT 4) ON x = y
    |> JOIN (SELECT number AS z FROM system.numbers LIMIT 4) ON x = z
    |> ORDER BY x;

-- JOIN with USING
SELECT number AS id, number * 10 AS value FROM system.numbers LIMIT 5
    |> JOIN (SELECT number AS id, 'label' AS label FROM system.numbers LIMIT 5) USING (id)
    |> ORDER BY id;

-- CROSS JOIN
SELECT number AS a FROM system.numbers LIMIT 2
    |> CROSS JOIN (SELECT number AS b FROM system.numbers LIMIT 2)
    |> ORDER BY a, b;

-- JOIN followed by other operations
SELECT number AS n FROM system.numbers LIMIT 10
    |> JOIN (SELECT number AS m FROM system.numbers LIMIT 10) ON n = m
    |> WHERE n > 5
    |> SELECT n
    |> ORDER BY n;
