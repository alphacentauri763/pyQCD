#ifndef PYQCD_WILSON_ACTION_HPP
#define PYQCD_WILSON_ACTION_HPP

/*
 * This file is part of pyQCD.
 *
 * pyQCD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * pyQCD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *
 * Created by Matt Spraggs on 10/02/16.
 *
 *
 * Here we implement the Wilson gauge action.
 */

#include "gauge_action.hpp"


namespace pyQCD
{
  namespace Gauge
  {
    template <typename Real, int Nc>
    class WilsonAction : public Action<Real, Nc>
    {
    public:
      WilsonAction(const Real beta, const Layout& layout);

      typename Action<Real, Nc>::GaugeLink compute_staples(
        const typename Action<Real, Nc>::GaugeField& gauge_field,
        const Int site_index) const;

      Real local_action(
        const typename Action<Real, Nc>::GaugeField& gauge_field,
        const Int site_index) const;

    private:
      std::vector<std::vector<Int>> links_;
    };

    template <typename Real, int Nc>
    WilsonAction<Real, Nc>::WilsonAction(const Real beta, const Layout& layout)
      : Action<Real, Nc>(beta, layout)
    {
      // Determine which link indices belong to which link staples
      links_.resize(layout.local_size());
      for (unsigned site_index = 0; site_index < layout.local_size(); ++site_index)
      {
        Site link_coords = layout.compute_site_coords(site_index);
        auto num_dims = layout.num_dims() - 1;

        // Determine which plane the link does not contribute to.
        Site planes(num_dims - 1);
        int j = 0;
        for (unsigned i = 0; i < num_dims; ++i) {
          if (link_coords[num_dims] != i) {
            planes[j++] = i;
          }
        }

        links_[site_index].resize(6 * (num_dims - 1));

        for (unsigned i = 0; i < num_dims - 1; ++i) {
          std::vector<int> link_coords_copy(
            link_coords.begin(), link_coords.end());

          /* First compute the loop above the link. Essentially we're interested
           * in:
           *
           * U_\nu (x + \mu) U^\dagger_\mu (x + \nu) U^\dagger_\nu (x)
           */
          // First link in U_\nu (x + \mu)
          link_coords_copy[num_dims] = planes[i];
          link_coords_copy[link_coords[num_dims]]++;
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i] = layout.get_array_index(link_coords_copy);
          // Next link is U^\dagger_\mu (x + \nu)
          link_coords_copy[num_dims] = link_coords[num_dims];
          link_coords_copy[link_coords[num_dims]]--;
          link_coords_copy[planes[i]]++;
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i + 1]
            = layout.get_array_index(link_coords_copy);
          // Next link is U^\dagger_\nu (x)
          link_coords_copy[planes[i]]--;
          link_coords_copy[num_dims] = planes[i];
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i + 2]
            = layout.get_array_index(link_coords_copy);

          /* Now we want to compute the link below the link. Essentially:
           *
           * U^\dagger _\nu (x + \mu - \nu) U^\dagger_\mu (x - \nu)
           *   U^\dagger_\nu (x - \nu)
           */
          // First link is U^\dagger_\nu (x + \mu - \nu)
          link_coords_copy[link_coords[num_dims]]++;
          link_coords_copy[planes[i]]--;
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i + 3]
            = layout.get_array_index(link_coords_copy);
          // Next link is U^\dagger_\mu (x - \nu)
          link_coords_copy[num_dims] = link_coords_copy[num_dims];
          link_coords_copy[link_coords[num_dims]]--;
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i + 4]
            = layout.get_array_index(link_coords_copy);
          // Next link is U_\nu (x - \nu)
          link_coords_copy[num_dims] = planes[i];
          layout.sanitise_site_coords(link_coords_copy);
          links_[site_index][6 * i + 5]
            = layout.get_array_index(link_coords_copy);
        }
      }
    }

    template <typename Real, int Nc>
    typename Action<Real, Nc>::GaugeLink
    WilsonAction<Real, Nc>::compute_staples(
      const typename Action<Real, Nc>::GaugeField& gauge_field,
      const Int site_index) const
    {
      auto ret = Action<Real, Nc>::GaugeLink::Zero().eval();
      auto temp_colour_mat = ret;

      auto num_dims = gauge_field.layout().num_dims() - 1;

      for (unsigned i = 0; i < 6 * (num_dims - 1); i += 6) {
        temp_colour_mat = gauge_field[links_[site_index][i]];
        temp_colour_mat *= gauge_field[links_[site_index][i + 1]].adjoint();
        temp_colour_mat *= gauge_field[links_[site_index][i + 2]].adjoint();

        ret += temp_colour_mat;

        temp_colour_mat = gauge_field[links_[site_index][i + 3]].adjoint();
        temp_colour_mat *= gauge_field[links_[site_index][i + 4]].adjoint();
        temp_colour_mat *= gauge_field[links_[site_index][i + 5]];

        ret += temp_colour_mat;
      }

      return ret;
    }

    template <typename Real, int Nc>
    Real WilsonAction<Real, Nc>::local_action(
      const typename Action<Real, Nc>::GaugeField& gauge_field,
      const Int site_index) const
    {
      // Compute the contribution to the action from the specified site
      auto staple = compute_staples(gauge_field, site_index);
      auto link = gauge_field(site_index);
      return -this->beta() * (link * staple).trace().real() / Nc;
    }
  }
}

#endif // PYQCD_WILSON_ACTION_HPP