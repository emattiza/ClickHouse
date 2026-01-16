-- Test pipe syntax with UNION operations
SET allow_experimental_pipe_syntax = 1;

-- Pipe operations followed by UNION
SELECT number FROM system.numbers LIMIT 5 |> WHERE number > 2
UNION ALL
SELECT number FROM system.numbers LIMIT 5 |> WHERE number < 2;
