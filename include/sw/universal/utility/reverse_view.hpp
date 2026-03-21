#pragma once
// reverse_view.hpp: wrapper function to reverse a container iteration for range based loops
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iterator>
#include <type_traits>
#include <utility>

namespace sw { namespace universal {

#if NO_XVALUE_CONTAINER_REQUIRED

	// simpler condition where we have an rvalue-based container that the
	// ReverseContainerView can iterate over.

	// Reverse container view
	template<typename Container>
	class ReverseContainerView {
	public:
		explicit ReverseContainerView(Container& c)
			: m_container{ c }
		{}
		auto begin() {
			return std::rbegin(m_container);
		}
		auto end() {
			return std::rend(m_container);
		}
	private:
		Container& m_container;
	};

	template<typename Container>
	auto reverse(Container& c) {
		return ReverseContainerView<Container>(c);
	}

	template<typename Container>
	auto reverse(const Container& cc) {
		return ReverseContainerView<const Container>(cc);
	}

#else
	// for cases where the container is an xvalue, we need to copy the contents
	// if we want to apply a reverse view on it
	template<typename Ty, bool CopyValue = !std::is_lvalue_reference<Ty>::value >
	struct ContainerContainer;

	// lvalue ContainerContainer: contains a reference to the container
	template<typename Ty>
	struct ContainerContainer<Ty, false> {
		Ty& c;
		explicit ContainerContainer(Ty& c)
			: c{ c }  // for lvalues store the reference to the container
		{}
	};

	// rvalue ContainerContainer: we first need to copy the contents into the ContainerContainer
	// so we can provide the ReverseContainerView access to something to iterate over
	template<typename Ty>
	struct ContainerContainer<Ty, true> {	
		const Ty c;
		explicit ContainerContainer(Ty c)
			: c{ std::move(c) }  // move will construct a new copy of the container
		{}
	};

	// ReverseContainerView for range based loops
	template<typename Container>
	class ReverseContainerView : ContainerContainer<Container> {
		using Base = ContainerContainer<Container>;
	public:
		explicit ReverseContainerView(Container&& c)
			: Base{ std::forward<Container>(c) }
		{}
		auto begin() {
			return std::rbegin(Base::c);
		}
		auto end() {
			return std::rend(Base::c);
		}
	};

	template<typename Container>
	auto reverse(Container&& c) {
		return ReverseContainerView<Container>(std::forward<Container>(c));
	}

#endif // NO_XVALUE_CONTAINER_REQUIRED

}} // namespace sw::universal
