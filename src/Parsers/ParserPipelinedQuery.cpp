#include <Parsers/ParserPipelinedQuery.h>

#include <Core/Joins.h>
#include <Parsers/ASTAsterisk.h>
#include <Parsers/ASTColumnsTransformers.h>
#include <Parsers/ASTExpressionList.h>
#include <Parsers/ASTIdentifier.h>
#include <Parsers/ASTSelectQuery.h>
#include <Parsers/ASTSelectWithUnionQuery.h>
#include <Parsers/ASTSubquery.h>
#include <Parsers/ASTTablesInSelectQuery.h>
#include <Parsers/ASTWindowDefinition.h>
#include <Parsers/CommonParsers.h>
#include <Parsers/ExpressionElementParsers.h>
#include <Parsers/ExpressionListParsers.h>
#include <Parsers/ParserSelectQuery.h>
#include <Parsers/ParserSetQuery.h>
#include <Parsers/ParserTablesInSelectQuery.h>


namespace DB
{

namespace ErrorCodes
{
extern const int SYNTAX_ERROR;
}

static ASTPtr wrapInSubquery(ASTPtr inner_query)
{
    ASTPtr select_with_union;
    if (inner_query->as<ASTSelectWithUnionQuery>())
    {
        select_with_union = std::move(inner_query);
    }
    else
    {
        auto select_list = std::make_shared<ASTExpressionList>();
        select_list->children.push_back(inner_query);

        auto union_query = std::make_shared<ASTSelectWithUnionQuery>();
        union_query->list_of_selects = select_list;
        union_query->children.push_back(select_list);

        select_with_union = union_query;
    }

    auto subquery = std::make_shared<ASTSubquery>(std::move(select_with_union));

    auto table_expr = std::make_shared<ASTTableExpression>();
    table_expr->subquery = subquery;
    table_expr->children.push_back(subquery);

    auto tables_elem = std::make_shared<ASTTablesInSelectQueryElement>();
    tables_elem->table_expression = table_expr;
    tables_elem->children.push_back(table_expr);

    auto tables = std::make_shared<ASTTablesInSelectQuery>();
    tables->children.push_back(tables_elem);

    auto select_list = std::make_shared<ASTExpressionList>();
    select_list->children.push_back(std::make_shared<ASTAsterisk>());

    auto select_query = std::make_shared<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    select_query->setExpression(ASTSelectQuery::Expression::TABLES, std::move(tables));

    return select_query;
}


static bool parsePipeWhere(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::WHERE).ignore(pos, expected))
        return false;

    ASTPtr where_expr;
    if (!ParserExpressionWithOptionalAlias(false).parse(pos, where_expr, expected))
        return false;

    auto select_query = node->as<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::WHERE, std::move(where_expr));
    return true;
}

static bool parsePipeSelect(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::SELECT).ignore(pos, expected))
        return false;

    bool distinct = false;
    if (ParserKeyword(Keyword::DISTINCT).ignore(pos, expected))
        distinct = true;

    ASTPtr select_list;
    if (!ParserNotEmptyExpressionList(true).parse(pos, select_list, expected))
        return false;

    auto select_query = node->as<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    if (distinct)
        select_query->distinct = true;
    return true;
}

static bool parsePipeOrderBy(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::ORDER_BY).ignore(pos, expected))
        return false;

    ASTPtr order_list;
    if (!ParserOrderByExpressionList().parse(pos, order_list, expected))
        return false;

    auto select_query = node->as<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::ORDER_BY, std::move(order_list));
    return true;
}

static bool parsePipeLimit(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::LIMIT).ignore(pos, expected))
        return false;

    ParserExpressionWithOptionalAlias exp_elem(false);
    ASTPtr limit_length;
    ASTPtr limit_offset;

    if (!exp_elem.parse(pos, limit_length, expected))
        return false;

    if (ParserToken(TokenType::Comma).ignore(pos, expected))
    {
        limit_offset = limit_length;
        if (!exp_elem.parse(pos, limit_length, expected))
            return false;
    }
    else if (ParserKeyword(Keyword::OFFSET).ignore(pos, expected))
    {
        if (!exp_elem.parse(pos, limit_offset, expected))
            return false;
    }

    auto select_query = node->as<ASTSelectQuery>();

    /// Check for LIMIT BY
    if (ParserKeyword(Keyword::BY).ignore(pos, expected))
    {
        ASTPtr limit_by_list;
        if (!ParserNotEmptyExpressionList(false).parse(pos, limit_by_list, expected))
            return false;

        select_query->setExpression(ASTSelectQuery::Expression::LIMIT_BY_LENGTH, std::move(limit_length));
        select_query->setExpression(ASTSelectQuery::Expression::LIMIT_BY_OFFSET, std::move(limit_offset));
        select_query->setExpression(ASTSelectQuery::Expression::LIMIT_BY, std::move(limit_by_list));
    }
    else
    {
        select_query->setExpression(ASTSelectQuery::Expression::LIMIT_LENGTH, std::move(limit_length));
        select_query->setExpression(ASTSelectQuery::Expression::LIMIT_OFFSET, std::move(limit_offset));
    }
    return true;
}

static bool parsePipeOffset(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::OFFSET).ignore(pos, expected))
        return false;

    ASTPtr offset;
    if (!ParserExpressionWithOptionalAlias(false).parse(pos, offset, expected))
        return false;

    auto select_query = node->as<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::LIMIT_OFFSET, std::move(offset));
    return true;
}

static bool parsePipeAggregate(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::AGGREGATE).ignore(pos, expected))
        return false;

    ASTPtr select_list;
    if (!ParserNotEmptyExpressionList(true).parse(pos, select_list, expected))
        return false;

    ASTPtr group_by_list;
    if (ParserKeyword(Keyword::GROUP_BY).ignore(pos, expected))
    {
        if (!ParserNotEmptyExpressionList(true).parse(pos, group_by_list, expected))
            return false;
    }

    auto select_query = node->as<ASTSelectQuery>();

    if (group_by_list)
    {
        /// Prepend group by columns to select list
        auto combined = std::make_shared<ASTExpressionList>();
        for (const auto & child : group_by_list->children)
            combined->children.push_back(child->clone());
        for (const auto & child : select_list->children)
            combined->children.push_back(child->clone());

        select_query->setExpression(ASTSelectQuery::Expression::SELECT, std::move(combined));
        select_query->setExpression(ASTSelectQuery::Expression::GROUP_BY, std::move(group_by_list));
    }
    else
    {
        select_query->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    }
    return true;
}

static bool parsePipeHaving(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::HAVING).ignore(pos, expected))
        return false;

    ASTPtr having_expr;
    if (!ParserExpressionWithOptionalAlias(false).parse(pos, having_expr, expected))
        return false;

    node->as<ASTSelectQuery>()->setExpression(ASTSelectQuery::Expression::HAVING, std::move(having_expr));
    return true;
}

static bool parsePipeExtend(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::EXTEND).ignore(pos, expected))
        return false;

    ASTPtr extend_list;
    if (!ParserNotEmptyExpressionList(true).parse(pos, extend_list, expected))
        return false;

    auto select_list = std::make_shared<ASTExpressionList>();
    select_list->children.push_back(std::make_shared<ASTAsterisk>());
    for (const auto & child : extend_list->children)
        select_list->children.push_back(child->clone());

    node->as<ASTSelectQuery>()->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    return true;
}

static bool parsePipeSet(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::SET).ignore(pos, expected))
        return false;

    ASTPtr set_list;
    if (!ParserNotEmptyExpressionList(true).parse(pos, set_list, expected))
        return false;

    auto select_list = std::make_shared<ASTExpressionList>();
    select_list->children.push_back(std::make_shared<ASTAsterisk>());
    for (const auto & child : set_list->children)
        select_list->children.push_back(child->clone());

    node->as<ASTSelectQuery>()->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    return true;
}

static bool parsePipeDrop(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::DROP).ignore(pos, expected))
        return false;

    ASTPtr drop_list;
    if (!ParserNotEmptyExpressionList(false).parse(pos, drop_list, expected))
        return false;

    auto asterisk = std::make_shared<ASTAsterisk>();
    auto except_transformer = std::make_shared<ASTColumnsExceptTransformer>();
    for (const auto & child : drop_list->children)
        except_transformer->children.push_back(child->clone());

    auto transformers = std::make_shared<ASTExpressionList>();
    transformers->children.push_back(except_transformer);
    asterisk->transformers = transformers;
    asterisk->children.push_back(transformers);

    auto select_list = std::make_shared<ASTExpressionList>();
    select_list->children.push_back(asterisk);

    node->as<ASTSelectQuery>()->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    return true;
}

static bool parsePipeDistinct(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    if (!ParserKeyword(Keyword::DISTINCT).ignore(pos, expected))
        return false;

    node->as<ASTSelectQuery>()->distinct = true;
    return true;
}

static bool parsePipeOperation(IParser::Pos & pos, ASTPtr & node, Expected & expected)
{
    return parsePipeWhere(pos, node, expected) || parsePipeSelect(pos, node, expected) || parsePipeOrderBy(pos, node, expected)
        || parsePipeLimit(pos, node, expected) || parsePipeOffset(pos, node, expected) || parsePipeAggregate(pos, node, expected)
        || parsePipeHaving(pos, node, expected) || parsePipeExtend(pos, node, expected) || parsePipeSet(pos, node, expected)
        || parsePipeDrop(pos, node, expected) || parsePipeDistinct(pos, node, expected) || parsePipeJoin(pos, node, expected);
}


static ASTPtr buildSelectStarFrom(ASTPtr table_expr)
{
    auto table_expression = std::make_shared<ASTTableExpression>();
    if (table_expr->as<ASTTableExpression>())
        table_expression = std::static_pointer_cast<ASTTableExpression>(table_expr);
    else
    {
        table_expression->database_and_table_name = table_expr;
        table_expression->children.push_back(table_expr);
    }

    auto tables_elem = std::make_shared<ASTTablesInSelectQueryElement>();
    tables_elem->table_expression = table_expression;
    tables_elem->children.push_back(table_expression);

    auto tables = std::make_shared<ASTTablesInSelectQuery>();
    tables->children.push_back(tables_elem);

    auto select_list = std::make_shared<ASTExpressionList>();
    select_list->children.push_back(std::make_shared<ASTAsterisk>());

    auto select_query = std::make_shared<ASTSelectQuery>();
    select_query->setExpression(ASTSelectQuery::Expression::SELECT, std::move(select_list));
    select_query->setExpression(ASTSelectQuery::Expression::TABLES, std::move(tables));

    return select_query;
}

bool ParserPipelinedQuery::parseImpl(Pos & pos, ASTPtr & node, Expected & expected)
{
    ASTPtr current_query;
    bool has_pipe_syntax = false;

    if (ParserKeyword(Keyword::FROM).ignore(pos, expected))
    {
        /// FROM-first syntax: FROM table |> ...
        ASTPtr table_expr;
        if (!ParserTableExpression().parse(pos, table_expr, expected))
            return false;

        current_query = buildSelectStarFrom(std::move(table_expr));
        has_pipe_syntax = true;
    }
    else if (ParserSelectQuery().parse(pos, current_query, expected))
    {
        /// Regular SELECT, may or may not have pipe operators following
    }
    else
    {
        return false;
    }

    ParserToken pipe_arrow(TokenType::PipeArrow);

    while (pipe_arrow.ignore(pos, expected))
    {
        has_pipe_syntax = true;
        current_query = wrapInSubquery(std::move(current_query));

        if (!parsePipeOperation(pos, current_query, expected))
            return false;
    }

    if (has_pipe_syntax)
    {
        if (auto * select_query = current_query->as<ASTSelectQuery>())
            select_query->from_pipe_syntax = true;
    }

    node = std::move(current_query);
    return true;
}

}
