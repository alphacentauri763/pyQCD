#ifndef LATTICE_HPP
#define LATTICE_HPP

/* This file declares and defines the Lattice class. This is basically an Array
 * but with a Layout member specifying the relationship between the sites and
 * the Array index. In addition, there are operator() implementations to access
 * elements using site coordinates or a lexicographic index.
 */

#include <cassert>
#include <vector>

#include "array.hpp"
#include "layout.hpp"


namespace pyQCD
{
  template <typename T, template <typename> class Alloc = std::allocator>
  class Lattice : public Array<T, Alloc, Lattice<T, Alloc> >
  {
  public:
    Lattice(const Layout* layout)
      : Array<T, Alloc, Lattice<T, Alloc> >(), layout_(layout)
    {
      this->data_.resize(layout->volume());
    }
    Lattice(const Layout* layout, const T& val)
      : Array<T, Alloc, Lattice<T, Alloc> >(layout->volume(), val),
        layout_(layout)
    {}
    Lattice(const Lattice<T, Alloc>& lattice) = default;
    template <typename U1, typename U2>
    Lattice(const ArrayExpr<U1, U2>& expr)
    {
      this->data_.resize(expr.size());
      for (unsigned long i = 0; i < expr.size(); ++i) {
        this->data_[i] = static_cast<T>(expr[i]);
      }
      layout_ = expr.layout();
    }
    Lattice(Lattice<T, Alloc>&& lattice) = default;

    T& operator()(const int i)
    { return this->data_[layout_->get_array_index(i)]; }
    const T& operator()(const int i) const
    { return this->data_[layout_->get_array_index(i)]; }
    template <typename U>
    T& operator()(const U& site)
    { return this->data_[layout_->get_array_index(site)]; }
    template <typename U>
    const T& operator()(const U& site) const
    { return this->data_[layout_->get_array_index(site)]; }

    Lattice<T, Alloc>& operator=(const Lattice<T, Alloc>& lattice);
    Lattice<T, Alloc>& operator=(Lattice<T, Alloc>&& lattice) = default;

    unsigned int volume() const { return layout_->volume(); }
    unsigned int num_dims() const { return layout_->num_dims(); }
    const Layout* layout() const { return layout_; }

  protected:
    const Layout* layout_;
  };


  template <typename T, template <typename> class Alloc>
  Lattice<T, Alloc>& Lattice<T, Alloc>::operator=(
    const Lattice<T, Alloc>& lattice)
  {
    assert (lattice.volume() == volume());
    if (&lattice != this) {
      for (unsigned int i = 0; i < volume(); ++i) {
        (*this)(lattice.layout_->get_site_index(i)) = lattice[i];
      }
    }
    return *this;
  }
}

#endif