#ifndef ARRAY_HPP
#define ARRAY_HPP

/* This file provides an array class, optimized using expression templates and
 * operator overloading. This is probably the most fundamental class in the
 * package, and is the parent to LatticeBase.
 *
 * The reason this has been written from scratch, rather than using the Eigen
 * Array type, is because the Eigen type doesn't support multiplication by
 * generic types, i.e. you can't do something like Array<Matrix3cd> * double.
 *
 * The expression templates that are used by this class can be found in
 * array_expr.hpp
 */

#include <base/array_expr.hpp>
#include <utils/templates.hpp>


namespace pyQCD
{
  template <typename T, template <typename> class Alloc = std::allocator>
  class Array : public ArrayExpr<Array<T, Alloc>, T>
  {
  public:
    Array();
    Array(const Array<T, Alloc>& array);
    Array(Array<T, Alloc>&& array);
    template <typename U>
    Array(const Array<U, T>& expr)
    {
      this->data_.resize(expr.size());
      for (int i = 0; i < expr.size(); ++i) {
        this->data_[i] = expr[i];
      }
    }

    Array<T, Alloc>& operator=(const Array<T, Alloc>& rhs);
    Array<T, Alloc>& operator=(Array<T, Alloc>&& rhs);

#define ARRAY_OPERATOR_ASSIGN_DECL(op)				                    \
    template <typename U,                                         \
              typename std::enable_if<                            \
		!is_instance_of_Array<U, Array>::value>::type*                \
              = nullptr>                                          \
    Array<T, ndim, Alloc>& operator op ## =(const U& rhs);	      \
    template <typename U>                                         \
    Array<T, ndim, Alloc>&                                        \
    operator op ## =(const Array<U, ndim, Alloc>& rhs);

    ARRAY_OPERATOR_ASSIGN_DECL(+);
    ARRAY_OPERATOR_ASSIGN_DECL(-);
    ARRAY_OPERATOR_ASSIGN_DECL(*);
    ARRAY_OPERATOR_ASSIGN_DECL(/);

  protected:
    std::vector<T, Alloc> data_;
  };

#define ARRAY_OPERATOR_ASSIGN_IMPL(op)
}

#endif