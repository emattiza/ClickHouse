-- Test pipe syntax with JOINs
-- Note: JOIN as a pipe operator is not yet implemented
-- This tests using pipe syntax with queries that contain JOINs

SET allow_experimental_pipe_syntax = 1;

-- Pipe operations on a query with JOIN
SELECT a.number AS a_num, b.number AS b_num
FROM (SELECT number FROM system.numbers LIMIT 5) AS a
JOIN (SELECT number FROM system.numbers LIMIT 5) AS b ON a.number = b.number
|> WHERE a_num > 2
|> ORDER BY a_num;

-- Pipe after a self-join
SELECT t1.number AS n1, t2.number AS n2
FROM (SELECT number FROM system.numbers LIMIT 4) AS t1
CROSS JOIN (SELECT number FROM system.numbers LIMIT 2) AS t2
|> WHERE n1 > 0
|> ORDER BY n1, n2;
