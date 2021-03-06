////////////////////////////////////////////////////////////
//
// MIT License
//
// DonerComponents
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

#include <donercomponents/CDonerComponentsSystems.h>
#include <donercomponents/component/CComponent.h>
#include <donercomponents/component/CComponentFactoryManager.h>
#include <donercomponents/handle/CHandle.h>

#include <gtest/gtest.h>

namespace DonerComponents
{
	namespace ComponentHandleTestInternal
	{
		class CCompFoo : public CComponent
		{};

		class CCompBar : public CComponent
		{};
        
        class CCompUnregistered : public CComponent
        {};
	}

	class CComponentHandleTest : public ::testing::Test
	{
	public:
		CComponentHandleTest()
			: m_componentFactoryManager(nullptr)
		{
			m_componentFactoryManager = CDonerComponentsSystems::CreateInstance()->Init().GetComponentFactoryManager();
			ADD_COMPONENT_FACTORY("foo", ComponentHandleTestInternal::CCompFoo, 1);
			ADD_COMPONENT_FACTORY("bar", ComponentHandleTestInternal::CCompBar, 1);
		}

		~CComponentHandleTest()
		{
			CDonerComponentsSystems::DestroyInstance();
		}

		CComponentFactoryManager *m_componentFactoryManager;
	};

	TEST_F(CComponentHandleTest, get_new_component_pointer)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompFoo>();
		EXPECT_NE(nullptr, component);
		ComponentHandleTestInternal::CCompFoo* foo = dynamic_cast<ComponentHandleTestInternal::CCompFoo*>(component);
		EXPECT_NE(nullptr, foo);
		ComponentHandleTestInternal::CCompBar* bar = dynamic_cast<ComponentHandleTestInternal::CCompBar*>(component);
		EXPECT_EQ(nullptr, bar);
	}

	TEST_F(CComponentHandleTest, get_new_component_pointer_by_name)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent("foo");
		EXPECT_NE(nullptr, component);
		ComponentHandleTestInternal::CCompFoo* foo = dynamic_cast<ComponentHandleTestInternal::CCompFoo*>(component);
		EXPECT_NE(nullptr, foo);
		ComponentHandleTestInternal::CCompBar* bar = dynamic_cast<ComponentHandleTestInternal::CCompBar*>(component);
		EXPECT_EQ(nullptr, bar);
	}

	TEST_F(CComponentHandleTest, get_new_component_as_handle)
	{
		CHandle fooHandle = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompFoo>();
		EXPECT_EQ(CHandle::EElementType::Component, fooHandle.m_elementType);
		EXPECT_EQ(0, fooHandle.m_componentIdx);
		EXPECT_EQ(0, fooHandle.m_elementPosition);
		EXPECT_EQ(0, fooHandle.m_version);
		EXPECT_TRUE(static_cast<bool>(fooHandle));

		CHandle barHandle = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();
		EXPECT_EQ(CHandle::EElementType::Component, barHandle.m_elementType);
		EXPECT_EQ(1, barHandle.m_componentIdx);
		EXPECT_EQ(0, barHandle.m_elementPosition);
		EXPECT_EQ(0, barHandle.m_version);
		EXPECT_TRUE(static_cast<bool>(barHandle));
	}

	TEST_F(CComponentHandleTest, cast_handle_to_generic_component)
	{
		CHandle fooHandle = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompFoo>();

		CComponent* component = fooHandle;
		EXPECT_NE(nullptr, component);

		int pos = m_componentFactoryManager->GetPositionForElement(component);
		EXPECT_EQ(pos, fooHandle.m_elementPosition);
		EXPECT_EQ(component->GetVersion(), fooHandle.m_version);
		CComponent* component2 = m_componentFactoryManager->GetByIdxAndVersion<ComponentHandleTestInternal::CCompFoo>(pos, component->GetVersion());
		EXPECT_EQ(component2, component);
	}

	TEST_F(CComponentHandleTest, cast_handle_to_specific_component)
	{
		CHandle fooHandle = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompFoo>();

		ComponentHandleTestInternal::CCompFoo* component = fooHandle;
		EXPECT_NE(nullptr, component);

		int pos = m_componentFactoryManager->GetPositionForElement(component);
		EXPECT_EQ(pos, fooHandle.m_elementPosition);
		EXPECT_EQ(component->GetVersion(), fooHandle.m_version);
		ComponentHandleTestInternal::CCompFoo* component2 =
			static_cast<ComponentHandleTestInternal::CCompFoo*>(
				m_componentFactoryManager->GetByIdxAndVersion<ComponentHandleTestInternal::CCompFoo>(
					pos, component->GetVersion()));
		EXPECT_EQ(component2, component);
	}
    
    TEST_F(CComponentHandleTest, cast_invalid_handle_to_component_fails)
    {
        static constexpr int invalidPos = 123;
        static constexpr int invalidVersion = 321;

        CComponent* component = m_componentFactoryManager->GetByIdxAndVersion<ComponentHandleTestInternal::CCompFoo>(invalidPos, invalidVersion);
        EXPECT_EQ(nullptr, component);
    }
    
    TEST_F(CComponentHandleTest, cast_handle_to_invalid_component_fails)
    {
        CHandle fooHandle = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompFoo>();
        
        CComponent* component = fooHandle;
        EXPECT_NE(nullptr, component);
        
        int pos = m_componentFactoryManager->GetPositionForElement(component);
        EXPECT_EQ(pos, fooHandle.m_elementPosition);
        EXPECT_EQ(component->GetVersion(), fooHandle.m_version);
        CComponent* component2 = m_componentFactoryManager->GetByIdxAndVersion<ComponentHandleTestInternal::CCompUnregistered>(pos, component->GetVersion());
        EXPECT_EQ(nullptr, component2);
    }

	TEST_F(CComponentHandleTest, cast_component_to_handle)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();

		CHandle compHandle = component;
		EXPECT_EQ(CHandle::EElementType::Component, compHandle.m_elementType);
		EXPECT_EQ(1, compHandle.m_componentIdx);
		EXPECT_EQ(0, compHandle.m_elementPosition);
		EXPECT_EQ(0, compHandle.m_version);
		EXPECT_TRUE(static_cast<bool>(compHandle));
	}

	TEST_F(CComponentHandleTest, invalidate_handle)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();

		CHandle compHandle = component;
		EXPECT_TRUE(static_cast<bool>(compHandle));
		m_componentFactoryManager->DestroyComponent(&component);
		EXPECT_FALSE(static_cast<bool>(compHandle));
	}

	TEST_F(CComponentHandleTest, null_component_from_invalid_handle)
	{
		CHandle handle;
		EXPECT_FALSE(static_cast<bool>(handle));
		CComponent* component = handle;
		EXPECT_EQ(nullptr, component);
	}

	TEST_F(CComponentHandleTest, invalid_handle_from_null_component)
	{
		CComponent* component = nullptr;
		CHandle handle = component;
		EXPECT_FALSE(static_cast<bool>(handle));
	}

	TEST_F(CComponentHandleTest, invalid_cast_to_handle_after_destroy)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();
		CHandle handle = component;
		EXPECT_TRUE(static_cast<bool>(handle));

		m_componentFactoryManager->DestroyComponent(&component);

		EXPECT_FALSE(static_cast<bool>(handle));
		component = handle;
		EXPECT_EQ(nullptr, component);
	}

	TEST_F(CComponentHandleTest, invalid_handle_because_of_version)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();
		CHandle handle = component;
		EXPECT_TRUE(static_cast<bool>(handle));

		m_componentFactoryManager->DestroyComponent(&component);

		CComponent* component2 = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();
		EXPECT_EQ(1, component2->GetVersion());

		EXPECT_FALSE(static_cast<bool>(handle));
	}

	TEST_F(CComponentHandleTest, destroy_component_through_handle)
	{
		CComponent* component = m_componentFactoryManager->CreateComponent<ComponentHandleTestInternal::CCompBar>();
		CHandle handle = component;

		handle.Destroy();
		EXPECT_TRUE(component->IsDestroyed());
		EXPECT_FALSE(static_cast<bool>(handle));
	}
}
