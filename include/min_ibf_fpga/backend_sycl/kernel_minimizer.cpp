		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor queries(queries_buffer, handler, sycl::read_only);
			sycl::accessor querySizes(querySizes_buffer, handler, sycl::read_only);

			sycl::stream out(65536, 256, handler); // DEBUG
			handler.single_task<MinimizerKernel>([=]() [[intel::kernel_args_restrict]]
			{
				using QueryIndex = types::QueryIndex;

				// Prefetching requires pointer
				auto querySizes_ptr = querySizes.get_pointer();
				auto queries_ptr = queries.get_pointer();

				QueryIndex queryOffset = queriesOffset;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					const QueryIndex querySize = PrefetchingLSU::load(querySizes_ptr + static_cast<size_t>(querySizesOffset) + static_cast<size_t>(queryIndex));

					const QueryIndex localQueryOffset = queryOffset;
					queryOffset += querySize;

					minimizer_kernel_t::compute_minimizer(querySize, [&](QueryIndex const iteration){
						return PrefetchingLSU::load(queries_ptr + static_cast<size_t>(localQueryOffset + iteration));
					}, [&](auto && data)
					{
						MinimizerToIBFPipes::PipeAt<id>::write(std::forward<decltype(data)>(data));
					});
				}
			});
		});