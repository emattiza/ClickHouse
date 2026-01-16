-- Test that pipe syntax is disabled by default
-- This should fail with an error about the setting being disabled

SELECT number FROM system.numbers LIMIT 5 |> WHERE number > 2; -- { serverError SUPPORT_IS_DISABLED }
