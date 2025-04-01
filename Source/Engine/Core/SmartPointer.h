#pragma once

/// @short Reference counted smart pointer class
/// @details Target class must implement AddReference and RemoveReference functions.
template <class Type> 
class SmartPointer
{
public:
  // constructors
  inline SmartPointer() : m_pTarget(NULL) {};
  inline ~SmartPointer() { if(m_pTarget) m_pTarget->RemoveReference(); };

  inline SmartPointer(const SmartPointer &other): m_pTarget(other.m_pTarget)
  {
    if(m_pTarget) m_pTarget->AddReference();
  };

  inline SmartPointer(Type *pTarget) : m_pTarget(pTarget)
  {
    if(m_pTarget) m_pTarget->AddReference();
  };

  // set
  inline const SmartPointer &operator=(Type *pTarget)
  {
    if(pTarget) pTarget->AddReference();
    if(m_pTarget) m_pTarget->RemoveReference();
    m_pTarget = pTarget;
    return *this;
  }

  // copy
  inline const SmartPointer &operator=(const SmartPointer &other)
  {
    if(other.m_pTarget) other.m_pTarget->AddReference();
    if(m_pTarget) m_pTarget->RemoveReference();
    m_pTarget = other.m_pTarget;
    return *this;
  }

  // access
  inline Type * operator->(void) const { return m_pTarget; }
  inline operator Type*(void) const { return m_pTarget; }
  inline Type& operator*(void) const { return *m_pTarget; }
  inline Type* Get(void) { return m_pTarget; }

  Type *m_pTarget;
};