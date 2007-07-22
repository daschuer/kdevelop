// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_VISITOR_H_INCLUDED
#define qmake_VISITOR_H_INCLUDED

#include "qmake_ast.h"

namespace QMake
  {

  class visitor
    {
      typedef void (visitor::*parser_fun_t)(ast_node *);
      static parser_fun_t _S_parser_table[];

    public:
      virtual ~visitor()
      {}

      virtual void visit_node(ast_node *node)
      {
        if  (node)
          (this->*_S_parser_table[node->kind -  1000])(node);
      }

      virtual void visit_arg_list(arg_list_ast *)
    {}

      virtual void visit_function_args(function_args_ast *)
      {}

      virtual void visit_item(item_ast *)
      {}

      virtual void visit_op(op_ast *)
      {}

      virtual void visit_or_op(or_op_ast *)
      {}

      virtual void visit_project(project_ast *)
      {}

      virtual void visit_scope(scope_ast *)
      {}

      virtual void visit_scope_body(scope_body_ast *)
      {}

      virtual void visit_stmt(stmt_ast *)
      {}

      virtual void visit_value(value_ast *)
      {}

      virtual void visit_value_list(value_list_ast *)
      {}

      virtual void visit_variable_assignment(variable_assignment_ast *)
      {}

    }

  ;

} // end of namespace QMake

#endif


