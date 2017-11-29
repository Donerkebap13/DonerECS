////////////////////////////////////////////////////////////
//
// MIT License
//
// DonerECS - A Tweaked Entity-Component System
// Copyright(c) 2017 Donerkebap13
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////

#include <donerecs/entity/CEntity.h>
#include <donerecs/entity/CEntityManager.h>
#include <donerecs/component/CComponentFactoryManager.h>

#include <cassert>

namespace DonerECS
{
	CEntity::CEntity()
		: m_componentFactoryManager(*CComponentFactoryManager::Get())
		, m_tagsManager(*CTagsManager::Get())
		, m_numDeactivations(1)
		, m_initialized(false)
		, m_destroyed(false)
		, m_initiallyActive(true)
	{
		m_components.resize(m_componentFactoryManager.GetRegisteredComponentsAmount());
	}

	CEntity::~CEntity()
	{}

	CEntity::operator CHandle()
	{
		CHandle handle;

		handle.m_elementType = CHandle::EElementType::Entity;
		handle.m_componentIdx = 0;
		handle.m_elementPosition = CEntityManager::Get()->GetPositionForElement(this);
		handle.m_version = GetVersion();

		return handle;
	}

	const CEntity* CEntity::operator=(const CHandle& rhs)
	{
		if (rhs.m_elementType == CHandle::EElementType::Entity)
		{
			*this = CEntityManager::Get()->GetElementByIdxAndVersion(rhs.m_elementPosition, rhs.m_version);
			return this;
		}
		return nullptr;
	}

	void CEntity::SetParent(CEntity* newParent)
	{
		CEntity* oldParent = m_parent;
		m_parent = newParent;
		if (oldParent)
		{
			oldParent->RemoveChild(this);
		}
	}

	bool CEntity::AddChild(CHandle newChild)
	{
		if (newChild && !HasChild(newChild))
		{
			CEntity* entity = newChild;
			entity->SetParent(this);
			m_children.emplace_back(newChild);
			return true;
		}
		return false;
	}

	bool CEntity::RemoveChild(CHandle child)
	{
		auto it = std::find(m_children.begin(), m_children.end(), child);
		if (it != m_children.end())
		{
			CEntity* entity = (*it);
			m_children.erase(it);
			if (entity->GetParent() == *this)
			{
				entity->SetParent(nullptr);
			}
			return true;
		}
		return false;
	}

	bool CEntity::HasChild(CHandle child) const
	{
		return std::find(m_children.begin(), m_children.end(), child) != m_children.end();
	}

	CHandle CEntity::GetChildByName(const std::string& name)
	{
		for (CEntity* child : m_children)
		{
			if (child && child->GetName() == name)
			{
				return child;
			}
		}
		return CHandle();
	}

	CHandle CEntity::GetChildByIndex(std::size_t index)
	{
		if (index >= 0 && index < m_children.size())
		{
			return m_children[index];
		}
		return CHandle();
	}

	CHandle CEntity::AddComponent(CStrID nameId)
	{
		CComponent* component = m_componentFactoryManager.AddComponent(nameId, m_components);
		if (component)
		{
			component->SetOwner(this);
		}
		return component;
	}

	bool CEntity::RemoveComponent(CStrID nameId)
	{
		int componentIdx = m_componentFactoryManager.GetFactoryIndexByName(nameId);
		if (componentIdx >= 0 && m_components[componentIdx])
		{
			return m_componentFactoryManager.DestroyComponent(&m_components[componentIdx]);
		}
		return false;
	}

	CHandle CEntity::GetComponent(CStrID nameId)
	{
		int componentIdx = m_componentFactoryManager.GetFactoryIndexByName(nameId);
		if (componentIdx >= 0)
		{
			return m_components[componentIdx];
		}
		return CHandle();
	}

	void CEntity::Init()
	{
		if (!m_initialized)
		{
			for (CComponent* component : m_components)
			{
				if (component)
				{
					component->Init();
				}
			}
			for (CEntity* child : m_children)
			{
				if (child)
				{
					child->Init();
				}
			}

			m_initialized = true;
		}
	}

	void CEntity::Destroy()
	{
		if (!m_destroyed)
		{
			for (CComponent* component : m_components)
			{
				if (component)
				{
					m_componentFactoryManager.DestroyComponent(&component);
				}
			}

			CEntityManager* entityManager = CEntityManager::Get();
			for (CEntity* child : m_children)
			{
				if (child)
				{
					entityManager->DestroyEntity(&child);
				}
			}

			m_destroyed = true;
		}
	}

	void CEntity::Activate()
	{
		if (m_initialized && m_numDeactivations > 0)
		{
			m_numDeactivations = 0;
			ActivateInternal();
		}
	}

	void CEntity::Deactivate()
	{
		if (m_initialized)
		{
			if (m_numDeactivations == 0)
			{
				for (CComponent* component : m_components)
				{
					if (component)
					{
						component->Deactivate();
					}
				}
				++m_numDeactivations;
			}
			for (CEntity* child : m_children)
			{
				if (child)
				{
					child->Deactivate();
				}
			}
		}
	}

	void CEntity::ActivateFromParent()
	{
		if (m_numDeactivations > 0)
		{
			--m_numDeactivations;
			if (m_numDeactivations == 0)
			{
				ActivateInternal();
			}
		}
	}

	void CEntity::ActivateInternal()
	{
		for (CComponent* component : m_components)
		{
			if (component)
			{
				component->ActivateFromParent();
			}
		}
		for (CEntity* child : m_children)
		{
			if (child)
			{
				child->ActivateFromParent();
			}
		}
	}

	void CEntity::CheckFirstActivation()
	{
		if (m_initialized)
		{
			for (CEntity* child : m_children)
			{
				if (child)
				{
					child->CheckFirstActivationInternal();
				}
			}

			for (CComponent* component : m_components)
			{
				if (component)
				{
					component->CheckFirstActivation();
				}
			}

			if (GetIsInitiallyActive())
			{
				m_numDeactivations = 1;
				ActivateFromParent();
			}
			else
			{
				m_numDeactivations = 0;
				Deactivate();
			}
		}
	}

	void CEntity::CheckFirstActivationInternal()
	{
		if (m_initialized)
		{
			if (!GetIsInitiallyActive())
			{
				++m_numDeactivations;
			}

			for (CComponent* component : m_components)
			{
				if (component)
				{
					component->CheckFirstActivation();
				}
			}

			for (CEntity* child : m_children)
			{
				if (child)
				{
					child->CheckFirstActivationInternal();
				}
			}
		}
	}

	void CEntity::CloneFrom(CEntity* entity)
	{
		if (entity)
		{
			SetParent(nullptr);
			SetName(entity->GetName());

			m_tags = entity->m_tags;

			m_numDeactivations = 1;
			m_initialized = false;
			m_destroyed = false;
			m_initiallyActive = true;

			m_componentFactoryManager.CloneComponents(entity->m_components, m_components);

			CEntityManager* entityManager = CEntityManager::Get();
			for (CEntity *child : entity->m_children)
			{
				if (child)
				{
					CEntity* newChild = entityManager->GetNewElement();
					if (newChild)
					{
						newChild->CloneFrom(child);
						AddChild(newChild);
					}
					else
					{
						assert(false || "CEntityManager doesn't have enough free entities to perform a full copy");
					}
				}
			}
		}
	}
}
