#pragma once
#include "ast.h"
#include <memory>

unique_ptr<ExprNode> parseExpr();
unique_ptr<ExprNode> parseStatement();
unique_ptr<ExprNode> parseBlock();
