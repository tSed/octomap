// $Id$

/**
* Octomap:
* A  probabilistic, flexible, and compact 3D mapping library for robotic systems.
* @author K. M. Wurm, A. Hornung, University of Freiburg, Copyright (C) 2009.
* @see http://octomap.sourceforge.net/
* License: GNU GPL v2, http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <math.h>
#include <cassert>

namespace octomap {


  template <class NODE>
  AbstractOcTree<NODE>::AbstractOcTree(double _resolution) :
    itsRoot(NULL), tree_depth(16), tree_max_val(32768), resolution(_resolution), tree_size(0) {
    this->setResolution(_resolution);
    for (unsigned i = 0; i< 3; i++){
      maxValue[i] = -1e6;
      minValue[i] = 1e6;
    }
    sizeChanged = true;
  }


  template <class NODE>
  AbstractOcTree<NODE>::~AbstractOcTree(){
  }

  template <class NODE>
  void AbstractOcTree<NODE>::setResolution(double r) {
    resolution = r;
    resolution_factor = 1. / resolution;
    tree_center(0) = tree_center(1) = tree_center(2) = ((double) tree_max_val) / resolution_factor;
  }


  template <class NODE>
  bool AbstractOcTree<NODE>::genKey(double val, unsigned short int& key) const{

    // scale to resolution and shift center for tree_max_val
    int scaled_val =  ((int) floor(resolution_factor * val)) + tree_max_val;

    // key within range of tree?
    if (( scaled_val > 0) && (((unsigned int) scaled_val) < (2*tree_max_val))) {
      key = scaled_val;
      return true;
    }
    else {
      return false;
    }
  }


  template <class NODE>
  bool AbstractOcTree<NODE>::genVal(unsigned short int& key, double& val) const{

    val = 0.0;

    if (key >= 2*tree_max_val)
      return false;

    val = ((double(key) - tree_max_val) + 0.5) * resolution;

    return true;
  }


  template <class NODE>
  unsigned int AbstractOcTree<NODE>::genPos(unsigned short int key[], int i) const {

    unsigned int retval = 0;
    if (key[0] & (1 << i)) retval += 1;
    if (key[1] & (1 << i)) retval += 2;
    if (key[2] & (1 << i)) retval += 4;
    return retval;
  }


  template <class NODE>
  bool AbstractOcTree<NODE>::search(const point3d& value, NODE*& result) const {

    result = NULL;

    // Search is a variant of insert which aborts if
    // it had to insert nodes

    unsigned short int key[3];

    for (unsigned int i=0; i<3; i++) {
      if ( !genKey( value(i), key[i]) ) {
        return false;
      }
    }

    NODE* curNode = itsRoot;

    // follow nodes down to last level...
    for (int i=(tree_depth-1); i>=0; i--) {

      unsigned int pos = genPos(key, i);

      if (curNode->childExists(pos)) {
        curNode = static_cast<NODE*>( curNode->getChild(pos) );
      }
      else {
        // we expected a child but did not get it
        // is the current node a leaf already?
        if (!curNode->hasChildren()) {
          result = curNode;
          return true;
        }
        else {
          // it is not, search failed
          return false;
        }
      }
    } // end for
    result = curNode;
    return true;
  }


  template <class NODE>
  void AbstractOcTree<NODE>::getLeafNodes(unsigned int max_depth, std::list<OcTreeVolume>& nodes) const{
    assert(itsRoot);
    if (tree_size <= 1) return; // A tree with only the root is an empty tree (by definition)
    getLeafNodesRecurs(itsRoot, 0, max_depth, tree_center, nodes);
  }


  template <class NODE>
  void AbstractOcTree<NODE>::getLeafNodesRecurs(NODE* node, unsigned int depth, unsigned int max_depth,
					const point3d& parent_center, std::list<OcTreeVolume>& nodes) const{

    if ((depth <= max_depth) && (node != NULL) ) {

      if (node->hasChildren() && (depth != max_depth)) {

        double center_offset = tree_center(0) / pow( 2., (double) depth+1);
        point3d search_center;

        for (unsigned int i=0; i<8; i++) {
          if (node->childExists(i)) {

            // x-axis
            if (i & 1)	search_center(0) = parent_center(0) + center_offset;
            else  	search_center(0) = parent_center(0) - center_offset;

            // y-axis
            if (i & 2)	search_center(1) = parent_center(1) + center_offset;
            else	search_center(1) = parent_center(1) - center_offset;
            // z-axis
            if (i & 4)	search_center(2) = parent_center(2) + center_offset;
            else	search_center(2) = parent_center(2) - center_offset;

            getLeafNodesRecurs(static_cast<NODE*>(node->getChild(i)), depth+1, max_depth, search_center, nodes);

          } // GetChild
        }
      }
      else {    // node is a leaf node or max depth reached
        double voxelSize = resolution * pow(2., double(tree_depth - depth));
        nodes.push_back(std::make_pair<point3d, double>(parent_center - tree_center, voxelSize));
      }
    }

  }


  template <class NODE>
  void AbstractOcTree<NODE>::getVoxels(unsigned int max_depth, std::list<OcTreeVolume>& voxels) const{
    assert(itsRoot);

    getVoxelsRecurs(itsRoot, 0, max_depth, tree_center, voxels);
  }



  template <class NODE>
  void AbstractOcTree<NODE>::getVoxelsRecurs(NODE* node, unsigned int depth, unsigned int max_depth,
			 const point3d& parent_center, std::list<OcTreeVolume>& voxels) const{

    if ((depth <= max_depth) && (node != NULL) ) {
      if (node->hasChildren() && (depth != max_depth)) {

        double center_offset = tree_center(0) / pow(2., (double) depth + 1);
        point3d search_center;

        for (unsigned int i = 0; i < 8; i++) {
          if (node->childExists(i)) {

            // x-axis
            if (i & 1)
              search_center(0) = parent_center(0) + center_offset;
            else
              search_center(0) = parent_center(0) - center_offset;

            // y-axis
            if (i & 2)
              search_center(1) = parent_center(1) + center_offset;
            else
              search_center(1) = parent_center(1) - center_offset;
            // z-axis
            if (i & 4)
              search_center(2) = parent_center(2) + center_offset;
            else
              search_center(2) = parent_center(2) - center_offset;

            getVoxelsRecurs(static_cast<NODE*>(node->getChild(i)), depth + 1, max_depth, search_center, voxels);

          } else{ // GetChild
            double voxelSize = resolution * pow(2., double(tree_depth - depth));
            voxels.push_back(std::make_pair<point3d, double>(parent_center - tree_center, voxelSize));
          }
        } // depth
      }
      // lowest level (= OcTree cells) is not drawn
    }
  }


}