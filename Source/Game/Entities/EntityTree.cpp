#include "StdAfx.h"
#include "Game/Entities/EntityTree.h"
#include "Game/Entities/Entity.h"

/// A recyclable temporally-coherent stack
template<class Type>
class Stack
{
public:
  Stack() : iStackFront(0), iStackEnd(0) {}

  /// Returns number of items in the stack
  inline unsigned int Count(void) const { return iStackEnd-iStackFront; }
  /// Returns true if stack is empty
  inline bool Empty(void) const { return iStackEnd == iStackFront; }

  /// Returns first item in the stack
  inline const Type &Front(void) const { return Array[iStackFront]; }
  /// Returns the last item in the stack
  inline const Type &Back(void) const { return Array[iStackEnd-1]; }
  /// Returns Nth item from the beginning of the stack
  inline const Type &operator[](int i) const { Array[i + iStackFront]; }

  /// Returns first item in the stack
  inline Type &Front(void) { return Array[iStackFront]; }
  /// Returns the last item in the stack
  inline Type &Back(void) { return Array[iStackEnd-1]; }
  /// Returns Nth item from the beginning of the stack
  inline Type &operator[](int i) { return Array[i + iStackFront]; }


  /// Clears the stack (maintains allocation)
  inline void Clear(void)
  {
    iStackFront = 0;
    iStackEnd = 0;
  }

  /// Pops items from the front of the stack
  inline void Pop(unsigned int iCount = 1)
  {
    iStackFront += iCount;
  }

  /// Pushes an item to the back of the stack
  inline void Push(const Type &Item)
  {
    if(iStackEnd >= Array.size())
      Array.resize(iStackEnd+1);
    Array[iStackEnd++] = Item;
  }

  /// Prepares space for pushing items to the back of the stack, allows calling PushFast
  inline void PreparePush(unsigned int iCount)
  {
    if(iCount != 0 && iStackEnd+iCount-1 >= Array.size())
      Array.resize(iStackEnd+iCount);
  }

  /// Pushes an item to the back of the stack, NOTE: assumes there is enough space
  inline void PushFast(const Type &Item)
  {
    Array[iStackEnd++] = Item;
  }

public:
  std::vector<Type> Array;
  unsigned int iStackFront;
  unsigned int iStackEnd;
};

/// Static copy of node stack for temporal coherency
static Stack<EntityTree::QuadTreeNode *> _NodeStack;

EntityTree::EntityTree()
{
  m_pRoot = new QuadTreeNode(NULL, 0);
  m_pRoot->m_fSize = 1024.0f;
  m_pRoot->m_vCorner = Vector2D(-1024.0f, 0);
  m_fSmallestCellSize = 32.0f;
}

EntityTree::~EntityTree()
{
  delete m_pRoot;
}

EntityTree::QuadTreeNode::QuadTreeNode(QuadTreeNode *pParent, int iIndexInParent)
{
  memset(m_pChildren, 0, sizeof(QuadTreeNode*) * 8);
  m_pParent = pParent;
  if(pParent != NULL)
  {
    m_fSize = pParent->m_fSize * 0.5f;

    int iIndexZ = iIndexInParent / 2;
    iIndexInParent -= iIndexZ * 2;
    int iIndexX = iIndexInParent;
    m_vCorner = pParent->m_vCorner + Vector2D(iIndexX * m_fSize, iIndexZ * m_fSize);
  }
}

EntityTree::QuadTreeNode::~QuadTreeNode()
{
  for(int i = 0; i < 4; i++)
    delete m_pChildren[i];
}

inline int EntityTree::QuadTreeNode::GetChildNodeIndex(const Vector3D &vPosition)
{
  float fInvHalfSize = 2.0f / m_fSize;
  int iIndexX = (int)((vPosition.x - m_vCorner.x) * fInvHalfSize);
  int iIndexZ = (int)((vPosition.z - m_vCorner.y) * fInvHalfSize);
  return iIndexX + iIndexZ * 2;
}

void EntityTree::QuadTreeNode::AddEntity(Entity *pEntity)
{
  m_Entities.push_back(pEntity);
  pEntity->m_pTreeNode = this;
}

void EntityTree::QuadTreeNode::RemoveEntity(Entity *pEntity)
{
  for(unsigned int i = 0; i < m_Entities.size(); i++)
  {
    if(m_Entities[i] == pEntity)
    {
      m_Entities[i] = m_Entities.back();
      m_Entities.pop_back();
      pEntity->m_pTreeNode = NULL;
      break;
    }
  }
}

void EntityTree::AddEntity(Entity *pEntity)
{
  // get center
  Vector3D vCenter = pEntity->m_vPosition;
  Vector3D vHalfSize = pEntity->m_vSize * 0.5f;

  // outside tree
  if(vCenter.x < m_pRoot->m_vCorner.x || vCenter.x >= m_pRoot->m_vCorner.x + m_pRoot->m_fSize ||
    vCenter.z < m_pRoot->m_vCorner.y || vCenter.z >= m_pRoot->m_vCorner.y + m_pRoot->m_fSize)
  {
    // add to root
    m_pRoot->AddEntity(pEntity);
    return;
  }

  // get largest half-size of object
  float fHalfSize = vHalfSize.z > vHalfSize.x ? vHalfSize.z : vHalfSize.x;

  // find proper node
  QuadTreeNode *pNode = m_pRoot;
  while(true)
  {
    // can object fit to child
    float fHalfLooseSizeInChild = pNode->m_fSize * 0.125f;
    if(fHalfSize < fHalfLooseSizeInChild
      && pNode->m_fSize > m_fSmallestCellSize) // don't create nodes too deep
    {
      // choose child based on center point
      int iIndex = pNode->GetChildNodeIndex(vCenter);

      // create if doesn't exist yet
      if(pNode->m_pChildren[iIndex] == NULL)
        pNode->m_pChildren[iIndex] = new QuadTreeNode(pNode, iIndex);

      pNode = pNode->m_pChildren[iIndex];
      continue;
    }

    // otherwise, this node is a perfect fit
    pNode->AddEntity(pEntity);
    break;
  }
}

void EntityTree::RemoveEntity(Entity *pEntity)
{
  // get node
  QuadTreeNode *pNode = (QuadTreeNode *)pEntity->m_pTreeNode;
  if(pNode != NULL)
  {
    // remove from entities
    pNode->RemoveEntity(pEntity);
    pEntity->m_pTreeNode = NULL;
  }
}

void EntityTree::UpdateEntity(Entity *pEntity)
{
  // get node
  QuadTreeNode *pNode = (QuadTreeNode *)pEntity->m_pTreeNode;
  QuadTreeNode *pOriginalNode = pNode;
  if(pNode == NULL)
  {
    AddEntity(pEntity);
  }
  else
  {
    // get center
    Vector3D vCenter = pEntity->m_vPosition;
    Vector3D vHalfSize = pEntity->m_vSize * 0.5f;
    Vector2D vCenter2D(vCenter.x, vCenter.z);

    // outside tree
    if(vCenter.x < m_pRoot->m_vCorner.x || vCenter.x >= m_pRoot->m_vCorner.x + m_pRoot->m_fSize ||
       vCenter.z < m_pRoot->m_vCorner.y || vCenter.z >= m_pRoot->m_vCorner.y + m_pRoot->m_fSize)
    {
      // add to root
      if(pNode != m_pRoot)
      {
        pNode->RemoveEntity(pEntity);
        m_pRoot->AddEntity(pEntity);
      }
      return;
    }

    // get largest half-size of object
    float fHalfSize = vHalfSize.z > vHalfSize.x ? vHalfSize.z : vHalfSize.x;

    // move upward in tree
    while(true)
    {
      Vector2D vDiff = vCenter2D - pNode->m_vCorner;

      float fHalfLooseSize = pNode->m_fSize * 0.25f;

      if(pNode->m_pParent != NULL &&
        // does not fit in node
        (fHalfSize > fHalfLooseSize
        // or outside node limits
        || vDiff.x < 0.0f || vDiff.x > pNode->m_fSize
        || vDiff.y < 0.0f || vDiff.y > pNode->m_fSize))
      {
        // go to parent
        pNode = pNode->m_pParent;
      }
      else
      {
        // it's good right here
        break;
      }
    }

    // move downward in tree
    while(true)
    {
      float fHalfLooseSizeInChild = pNode->m_fSize * 0.125f;

      // can object fit to child
      if(fHalfSize < fHalfLooseSizeInChild
        && pNode->m_fSize > m_fSmallestCellSize) // don't create nodes too deep
      {
        // choose child based on center point
        int iIndex = pNode->GetChildNodeIndex(vCenter);

        // create if doesn't exist yet
        if(pNode->m_pChildren[iIndex] == NULL)
          pNode->m_pChildren[iIndex] = new QuadTreeNode(pNode, iIndex);

        // go to child
        pNode = pNode->m_pChildren[iIndex];
      }
      else
      {
        // it's good right here
        break;
      }
    }

    // has chosen a new node
    if(pOriginalNode != pNode)
    {
      pOriginalNode->RemoveEntity(pEntity);
      pNode->AddEntity(pEntity);
    }
  }
}

void EntityTree::GetEntities(const Vector3D &vStart, const Vector3D &vEnd, std::vector<Entity *> &Result)
{
  Result.clear();

  _NodeStack.Clear();
  _NodeStack.Push(m_pRoot);

  // BFS search with stack
  while(!_NodeStack.Empty())
  {
    // pop node
    QuadTreeNode *pNode = _NodeStack.Front();
    _NodeStack.Pop();

    // calculate loose bounding box
    float fHalfLoose = pNode->m_fSize * 0.25f;
    float fSizeAndHalfLoose = pNode->m_fSize + fHalfLoose;
    Vector2D vNodeStart = pNode->m_vCorner - Vector2D(fHalfLoose, fHalfLoose);
    Vector2D vNodeEnd = pNode->m_vCorner + Vector2D(fSizeAndHalfLoose, fSizeAndHalfLoose);
    
    // test if intersects
    if(vStart.x <= vNodeEnd.x && vEnd.x >= vNodeStart.x &&
       vStart.z <= vNodeEnd.y && vEnd.z >= vNodeStart.y)
    {
      unsigned int iCount = pNode->m_Entities.size();
      if(iCount > 0)
      {
        // add objects from node
        Result.reserve(Result.size() + iCount);
        for(unsigned int i = 0; i < iCount; i++)
        {
          Entity *pEntity = pNode->m_Entities[i];
          Vector3D vEntityStart = pEntity->m_vPosition - pEntity->m_vSize * 0.5f;
          Vector3D vEntityEnd = pEntity->m_vPosition + pEntity->m_vSize * 0.5f;

          // test if intersects
          if(vStart.x <= vEntityEnd.x && vEnd.x >= vEntityStart.x &&
             vStart.y <= vEntityEnd.y && vEnd.y >= vEntityStart.y &&
             vStart.z <= vEntityEnd.z && vEnd.z >= vEntityStart.z)
          {
            Result.push_back(pEntity);
          }
        }
      }

      // add children to stack
      _NodeStack.PreparePush(4);
      for(int i = 0; i < 4; i++)
      {
        if(pNode->m_pChildren[i] != NULL)
          _NodeStack.PushFast(pNode->m_pChildren[i]);
      }
    }
  }
}

void EntityTree::ClearEmptyNodes(void)
{
  _NodeStack.Clear();
  _NodeStack.Push(m_pRoot);

  // BFS search with stack
  while(!_NodeStack.Empty())
  {
    // pop node
    QuadTreeNode *pNode = _NodeStack.Front();
    _NodeStack.Pop();

    // add children to stack
    _NodeStack.PreparePush(4);
    for(int i = 0; i < 4; i++)
    {
      if(pNode->m_pChildren[i] != NULL)
        _NodeStack.PushFast(pNode->m_pChildren[i]);
    }
  }

  // go from bottom to top
  for(int i = _NodeStack.iStackEnd - 1; i >= 0; i--)
  {
    QuadTreeNode *pNode = _NodeStack.Array[i];

    // all entities must be empty
    if(!pNode->m_Entities.empty())
      continue;

    // all children must be empty
    if(pNode->m_pChildren[0] != NULL ||
      pNode->m_pChildren[1] != NULL ||
      pNode->m_pChildren[2] != NULL ||
      pNode->m_pChildren[3] != NULL)
    {
      continue;
    }

    // find index in parent
    if(pNode->m_pParent != NULL)
    {
      for(int j = 0; j < 4; j++)
      {
        if(pNode->m_pParent->m_pChildren[j] == pNode)
        {
          pNode->m_pParent->m_pChildren[j] = NULL;
          delete pNode;
          break;
        }
      }
    }
  }
}