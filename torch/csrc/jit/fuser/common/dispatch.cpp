#include <torch/csrc/jit/fuser/common/ir.h>
#include <torch/csrc/jit/fuser/common/tensor.h>
#include <torch/csrc/jit/fuser/common/type.h>

#include <torch/csrc/jit/fuser/common/dispatch.h>

namespace torch {
namespace jit {
namespace fuser {

template <typename T>
T* ptr(T& obj) {
  return &obj;
}

template <typename T>
T* ptr(T* obj) {
  return obj;
}

/*
 * Generic dispatch for any handler that does not modify the IR directly.
 * For example we may want to walk the graph to construct a topologically sorted
 * set of exprs. This doesn't modify the IR directly. We also use this to print
 * the IR itself.
 * This dispatch is paired with a class that implements the functions:
 * template <typenname node_type>
 * int handler(node_type* node)
 *
 * handler should call:
 * dispatch(this, node_to_dispatch)
 *
 * It could also implement:
 * int handler(Statement* stmt){
 *   dispatch(this, stmt);
 * }
 *
 * And therefore dispatch should never call:
 * ptr(mutator)->handle(static_cast<Statement*>(this));
 */

template <typename T>
void Val::dispatch(T handler, Val* val) {
  switch (*(val->getValType())) {
    case ValType::IterDomain:
      ptr(handler)->handle(static_cast<IterDomain*>(val));
      return;
    case ValType::TensorDomain:
      ptr(handler)->handle(static_cast<TensorDomain*>(val));
      return;
    case ValType::Tensor:
      ptr(handler)->handle(static_cast<Tensor*>(val));
      return;
    case ValType::TensorView:
      ptr(handler)->handle(static_cast<TensorView*>(val));
      return;
    case ValType::Scalar:
      switch (*(val->getDataType())) {
        case DataType::Float:
          ptr(handler)->handle(static_cast<Float*>(val));
          return;
        case DataType::Int:
          ptr(handler)->handle(static_cast<Int*>(val));
          return;
        default:
          break;
      }
    default:
      break;
  }
  TORCH_INTERNAL_ASSERT(false, "Unknown valtype in dispatch!");
}

template <typename T>
void Expr::dispatch(T handler, Expr* expr) {
  switch (*(expr->getExprType())) {
    case ExprType::Split:
      ptr(handler)->handle(static_cast<Split*>(expr));
      return;
    case ExprType::Merge:
      ptr(handler)->handle(static_cast<Merge*>(expr));
      return;
    case ExprType::Reorder:
      ptr(handler)->handle(static_cast<Reorder*>(expr));
      return;
    case ExprType::UnaryOp:
      ptr(handler)->handle(static_cast<UnaryOp*>(expr));
      return;
    case ExprType::BinaryOp:
      ptr(handler)->handle(static_cast<BinaryOp*>(expr));
      return;
    case ExprType::ForLoop:
      ptr(handler)->handle(static_cast<ForLoop*>(expr));
      return;
    case ExprType::IfThenElse:
      ptr(handler)->handle(static_cast<IfThenElse*>(expr));
      return;
    default:
      TORCH_INTERNAL_ASSERT(false, "Unknown exprtype in dispatch!");
  }
}

template <typename T>
void Statement::dispatch(T handler, Statement* stmt) {
  if (stmt->isVal()) {
    ptr(handler)->handle(static_cast<Val*>(stmt));
  } else if (stmt->isExpr()) {
    ptr(handler)->handle(static_cast<Expr*>(stmt));
  } else
    TORCH_INTERNAL_ASSERT(false, "Unknown stmttype in dispatch!");
}

template <typename T>
void Val::const_dispatch(T handler, const Val* const val) {
  switch (*(val->getValType())) {
    case ValType::IterDomain:
      ptr(handler)->handle(static_cast<const IterDomain* const>(val));
      return;
    case ValType::TensorDomain:
      ptr(handler)->handle(static_cast<const TensorDomain* const>(val));
      return;
    case ValType::Tensor:
      ptr(handler)->handle(static_cast<const Tensor* const>(val));
      return;
    case ValType::TensorView:
      ptr(handler)->handle(static_cast<const TensorView* const>(val));
      return;
    case ValType::Scalar:
      switch (*(val->getDataType())) {
        case DataType::Float:
          ptr(handler)->handle(static_cast<const Float* const>(val));
          return;
        case DataType::Int:
          ptr(handler)->handle(static_cast<const Int* const>(val));
          return;
        default:
          break;
      }
    default:
      break;
  }
  TORCH_INTERNAL_ASSERT(false, "Unknown valtype in dispatch!");
}

template <typename T>
void Expr::const_dispatch(T handler, const Expr* const expr) {
  switch (*(expr->getExprType())) {
    case ExprType::Split:
      ptr(handler)->handle(static_cast<const Split* const>(expr));
      return;
    case ExprType::Merge:
      ptr(handler)->handle(static_cast<const Merge* const>(expr));
      return;
    case ExprType::Reorder:
      ptr(handler)->handle(static_cast<const Reorder* const>(expr));
      return;
    case ExprType::UnaryOp:
      ptr(handler)->handle(static_cast<const UnaryOp* const>(expr));
      return;
    case ExprType::BinaryOp:
      ptr(handler)->handle(static_cast<const BinaryOp* const>(expr));
      return;
    case ExprType::ForLoop:
      ptr(handler)->handle(static_cast<const ForLoop* const>(expr));
      return;
    case ExprType::IfThenElse:
      ptr(handler)->handle(static_cast<const IfThenElse* const>(expr));
      return;
    default:
      TORCH_INTERNAL_ASSERT(false, "Unknown exprtype in dispatch!");
  }
}

template <typename T>
void Statement::const_dispatch(T handler, const Statement* const stmt) {
  if (stmt->isVal()) {
    ptr(handler)->handle(static_cast<const Val* const>(stmt));
  } else if (stmt->isExpr()) {
    ptr(handler)->handle(static_cast<const Expr* const>(stmt));
  } else
    TORCH_INTERNAL_ASSERT(false, "Unknown stmttype in dispatch!");
}

/*
 * Generic mutator_dispatch for any handler that modifies the IR. This could be
 * a transformation on loop structures, or parallelizing a loop. This
 * mutator_dispatch is paired with a class that implements the functions
 * template <typenname node_type> Statement* mutate(node_type* node) mutate
 * should call (statement* node_to_dispatch)->mutator_dispatch() It could also
 * implement Statement* mutate(Statement* stmt){ stmt->mutator_dispatch(this);
 * }
 * And therefore dispatch should never call:
 *   ptr(mutator)->mutate(static_cast<Statement*>(this));
 */
template <typename T>
Statement* Val::mutator_dispatch(T mutator, Val* val) {
  switch (*(val->getValType())) {
    case ValType::IterDomain:
      return ptr(mutator)->mutate(static_cast<IterDomain*>(val));
    case ValType::TensorDomain:
      return ptr(mutator)->mutate(static_cast<TensorDomain*>(val));
    case ValType::Tensor:
      return ptr(mutator)->mutate(static_cast<Tensor*>(val));
    case ValType::TensorView:
      return ptr(mutator)->mutate(static_cast<TensorView*>(val));
    case ValType::Scalar:
      switch (*(val->getDataType())) {
        case DataType::Float:
          return ptr(mutator)->mutate(static_cast<Float*>(val));
        case DataType::Int:
          return ptr(mutator)->mutate(static_cast<Int*>(val));
        default:
          break;
      }
    default:
      break;
  }
  TORCH_INTERNAL_ASSERT(false, "Unknown valtype in dispatch!");
}

template <typename T>
Statement* Expr::mutator_dispatch(T mutator, Expr* expr) {
  switch (*(expr->getExprType())) {
    case ExprType::Split:
      return ptr(mutator)->mutate(static_cast<Split*>(expr));
    case ExprType::Merge:
      return ptr(mutator)->mutate(static_cast<Merge*>(expr));
    case ExprType::Reorder:
      return ptr(mutator)->mutate(static_cast<Reorder*>(expr));
    case ExprType::UnaryOp:
      return ptr(mutator)->mutate(static_cast<UnaryOp*>(expr));
    case ExprType::BinaryOp:
      return ptr(mutator)->mutate(static_cast<BinaryOp*>(expr));
    case ExprType::ForLoop:
      return ptr(mutator)->mutate(static_cast<ForLoop*>(expr));
    case ExprType::IfThenElse:
      return ptr(mutator)->mutate(static_cast<ForLoop*>(expr));
    default:
      TORCH_INTERNAL_ASSERT(false, "Unknown exprtype in dispatch!");
  }
}

template <typename T>
Statement* Statement::mutator_dispatch(T mutator, Statement* stmt) {
  if (stmt->isVal()) {
    return ptr(mutator)->mutate(static_cast<Val*>(stmt));
  }
  if (stmt->isExpr()) {
    return ptr(mutator)->mutate(static_cast<Expr*>(stmt));
  }
  TORCH_INTERNAL_ASSERT(false, "Unknown stmttype in dispatch!");
}

/*
 * Handler template instantiations. These should only have to be done on base
 * classes. Actual visitors/mutators should inhereit from these classes and call
 * ->dispatch(this) to avoid needing an explicit instantiation.
 */
template void Statement::dispatch(OptOutDispatch, Statement*);
template void Statement::dispatch(OptOutDispatch*, Statement*);
template void Val::dispatch(OptOutDispatch, Val*);
template void Val::dispatch(OptOutDispatch*, Val*);
template void Expr::dispatch(OptOutDispatch, Expr*);
template void Expr::dispatch(OptOutDispatch*, Expr*);

template void Statement::dispatch(OptInDispatch, Statement*);
template void Statement::dispatch(OptInDispatch*, Statement*);
template void Val::dispatch(OptInDispatch, Val*);
template void Val::dispatch(OptInDispatch*, Val*);
template void Expr::dispatch(OptInDispatch, Expr*);
template void Expr::dispatch(OptInDispatch*, Expr*);

template void Statement::const_dispatch(
    OptInConstDispatch,
    const Statement* const);
template void Statement::const_dispatch(
    OptInConstDispatch*,
    const Statement* const);
template void Val::const_dispatch(OptInConstDispatch, const Val* const);
template void Val::const_dispatch(OptInConstDispatch*, const Val* const);
template void Expr::const_dispatch(OptInConstDispatch, const Expr* const);
template void Expr::const_dispatch(OptInConstDispatch*, const Expr* const);

template Statement* Statement::mutator_dispatch(OptOutMutator, Statement*);
template Statement* Statement::mutator_dispatch(OptOutMutator*, Statement*);
template Statement* Val::mutator_dispatch(OptOutMutator, Val*);
template Statement* Val::mutator_dispatch(OptOutMutator*, Val*);
template Statement* Expr::mutator_dispatch(OptOutMutator, Expr*);
template Statement* Expr::mutator_dispatch(OptOutMutator*, Expr*);

template Statement* Statement::mutator_dispatch(OptInMutator, Statement*);
template Statement* Statement::mutator_dispatch(OptInMutator*, Statement*);
template Statement* Val::mutator_dispatch(OptInMutator, Val*);
template Statement* Val::mutator_dispatch(OptInMutator*, Val*);
template Statement* Expr::mutator_dispatch(OptInMutator, Expr*);
template Statement* Expr::mutator_dispatch(OptInMutator*, Expr*);

void OptOutDispatch::handle(Statement* s) {
  Statement::dispatch(this, s);
}
void OptOutDispatch::handle(Expr* e) {
  Expr::dispatch(this, e);
}
void OptOutDispatch::handle(Val* v) {
  Val::dispatch(this, v);
}

void OptInDispatch::handle(Statement* s) {
  Statement::dispatch(this, s);
}
void OptInDispatch::handle(Expr* e) {
  Expr::dispatch(this, e);
}
void OptInDispatch::handle(Val* v) {
  Val::dispatch(this, v);
}

void OptInConstDispatch::handle(const Statement* const s) {
  Statement::const_dispatch(this, s);
}
void OptInConstDispatch::handle(const Expr* const e) {
  Expr::const_dispatch(this, e);
}
void OptInConstDispatch::handle(const Val* const v) {
  Val::const_dispatch(this, v);
}

Statement* OptOutMutator::mutate(Statement* s) {
  return Statement::mutator_dispatch(this, s);
}
Statement* OptOutMutator::mutate(Expr* e) {
  return Expr::mutator_dispatch(this, e);
}
Statement* OptOutMutator::mutate(Val* v) {
  return Val::mutator_dispatch(this, v);
}

} // namespace fuser
} // namespace jit
} // namespace torch