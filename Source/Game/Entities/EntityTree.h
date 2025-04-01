#pragma once

/// Loose quadtree that contains all entities
class EntityTree
{
public:
  EntityTree();
  ~EntityTree();

  /// Adds an entity to the tree
  void AddEntity(Entity *pEntity);
  /// Removes entity from tree
  void RemoveEntity(Entity *pEntity);
  /// Updates entity position in tree after size or position change
  void UpdateEntity(Entity *pEntity);

  /// Returns entities in given area
  void GetEntities(const Vector3D &vStart, const Vector3D &vEnd, std::vector<Entity *> &Result);

  /// Clears all empty nodes
  void ClearEmptyNodes(void);

public:
  /// A quadtree node
  class QuadTreeNode
  {
  public:
    QuadTreeNode(QuadTreeNode *pParent, int iIndexInParent);
    ~QuadTreeNode();

    /// Returns index for child node array for the given location
    inline int GetChildNodeIndex(const Vector3D &vPosition);

    /// Adds an entity in the array
    inline void AddEntity(Entity *pEntity);
    /// Removes an entity from the array
    inline void RemoveEntity(Entity *pEntity);

  public:
    /// Length of the cell (not including looseness)
    float m_fSize;
    /// The eight child nodes, may be NULL
    QuadTreeNode *m_pChildren[4];
    /// The parent node, NULL for root node
    QuadTreeNode *m_pParent;
    /// Entities contained in this node
    std::vector<Entity *> m_Entities;
    /// World space corner position of the node (not including looseness)
    Vector2D m_vCorner;
  };

  /// Root node
  QuadTreeNode *m_pRoot;
  /// Smallest allowed cell width to limit tree depth
  float m_fSmallestCellSize;
};