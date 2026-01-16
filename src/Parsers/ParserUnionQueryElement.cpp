#include <Parsers/ASTSubquery.h>
#include <Parsers/ExpressionElementParsers.h>
#include <Parsers/ParserPipelinedQuery.h>
#include <Parsers/ParserSelectQuery.h>
#include <Parsers/ParserUnionQueryElement.h>
#include <Common/typeid_cast.h>


namespace DB
{

bool ParserUnionQueryElement::parseImpl(Pos & pos, ASTPtr & node, Expected & expected)
{
    /// Try subquery first, then pipelined query (which handles both regular SELECT and pipe syntax)
    if (!ParserSubquery().parse(pos, node, expected) && !ParserPipelinedQuery().parse(pos, node, expected))
        return false;

    if (const auto * ast_subquery = node->as<ASTSubquery>())
        node = ast_subquery->children.at(0);

    return true;
}

}
