#ifndef OCTOMAP_COUNTING_OCTREE_HH
#define OCTOMAP_COUNTING_OCTREE_HH

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

#include "AbstractOcTree.h"

namespace octomap {

  /*!
   * An OcTreeNode which stores an internal counter per node / cell
   * count is recursive, parent nodes have the summed count of their
   * children.
   */
  class CountingOcTreeNode {

  public:

    CountingOcTreeNode();
    ~CountingOcTreeNode();

    CountingOcTreeNode* getChild(unsigned int i);
    void setChild(unsigned int i, CountingOcTreeNode* child);
    bool childExists(unsigned int i) const;
    bool hasChildren() const;

    unsigned int getCount() const { return count; }
    void increaseCount() { count++; }

  protected:
    CountingOcTreeNode* itsChildren[8];
    unsigned int count;
  };


  /*!
   * OcTree implementation storing the hit counts for single cells
   */
  class CountingOcTree : public AbstractOcTree <CountingOcTreeNode> {

  public:

    CountingOcTree(double resolution = 0.1);
    ~CountingOcTree();

    CountingOcTreeNode* updateNode(const point3d& value);

  protected:

    void traverseNode(CountingOcTreeNode* traversedNode);

  };


}


#endif