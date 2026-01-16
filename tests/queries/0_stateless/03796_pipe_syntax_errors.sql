-- Test error handling for invalid pipe syntax
SET allow_experimental_pipe_syntax = 1;

-- Missing operation after pipe
SELECT number FROM system.numbers LIMIT 5 |>; -- { serverError SYNTAX_ERROR }

-- Invalid operation after pipe
SELECT number FROM system.numbers LIMIT 5 |> INVALID_OP; -- { serverError SYNTAX_ERROR }

-- Pipe at the beginning (no source query)
|> WHERE number > 5; -- { serverError SYNTAX_ERROR }

-- Double pipe
SELECT number FROM system.numbers LIMIT 5 |> |> WHERE number > 2; -- { serverError SYNTAX_ERROR }
