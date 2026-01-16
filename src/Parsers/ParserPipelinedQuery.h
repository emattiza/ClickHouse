#pragma once

#include <Parsers/IParserBase.h>

namespace DB
{

/** Parses a pipelined query using the |> operator.
  *
  * Syntax:
  *   <select_query> |> <pipe_operation> |> <pipe_operation> ...
  *
  * Each pipe operation is one of:
  *   WHERE <expr>
  *   SELECT <expr_list>
  *   ORDER BY <order_list>
  *   LIMIT <n> [OFFSET <m>]
  *   LIMIT <n> BY <expr_list>
  *   AGGREGATE <agg_list> [GROUP BY <expr_list>]
  *   EXTEND <expr AS alias, ...>
  *   JOIN ...
  *   etc.
  *
  * The parser transforms each pipe operation into a subquery chain:
  *   SELECT ... |> WHERE x > 10 |> SELECT a, b
  * becomes:
  *   SELECT a, b FROM (SELECT * FROM (SELECT ...) WHERE x > 10)
  *
  * Requires allow_experimental_pipe_syntax setting to be enabled (checked in executeQuery).
  */
class ParserPipelinedQuery : public IParserBase
{
protected:
    const char * getName() const override { return "pipelined query"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};

}
