// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef qmake_DEFAULT_VISITOR_H_INCLUDED
#define qmake_DEFAULT_VISITOR_H_INCLUDED

#include "qmake_visitor.h"

namespace QMake
  {

  class default_visitor:  public visitor
    {

    public:
      virtual void visit_arg_list(arg_list_ast *node);
      virtual void visit_function_args(function_args_ast *node);
      virtual void visit_item(item_ast *node);
      virtual void visit_op(op_ast *node);
      virtual void visit_or_op(or_op_ast *node);
      virtual void visit_project(project_ast *node);
      virtual void visit_scope(scope_ast *node);
      virtual void visit_scope_body(scope_body_ast *node);
      virtual void visit_stmt(stmt_ast *node);
      virtual void visit_value(value_ast *node);
      virtual void visit_value_list(value_list_ast *node);
      virtual void visit_variable_assignment(variable_assignment_ast *node);
    };

} // end of namespace QMake

#endif


