-- Test pipe syntax with FORMAT clause
SET allow_experimental_pipe_syntax = 1;

-- Pipe with FORMAT at the end
SELECT number FROM system.numbers LIMIT 3 |> WHERE number > 0 FORMAT JSONEachRow;

-- Pipe with FORMAT TabSeparated
SELECT number, number * 2 AS doubled FROM system.numbers LIMIT 3 |> SELECT number FORMAT TabSeparated;

-- Pipe with FORMAT CSV
SELECT number FROM system.numbers LIMIT 3 |> ORDER BY number DESC FORMAT CSV;
