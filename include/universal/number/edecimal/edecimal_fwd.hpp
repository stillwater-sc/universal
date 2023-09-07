#pragma once
// edecimal_fwd.hpp: forward definition of adaptive precision edecimal integer data type
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Forward references
	class edecimal;

	struct edecintdiv;

	edecimal quotient(const edecimal&, const edecimal&);
	edecimal remainder(const edecimal&, const edecimal&);

	int findMsd(const edecimal&);

}} // namespace sw::universal
