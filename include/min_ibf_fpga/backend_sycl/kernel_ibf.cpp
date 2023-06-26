		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor ibfData(ibfData_buffer, handler, sycl::read_only);
			sycl::accessor thresholds(thresholds_buffer, handler, sycl::read_only);
			sycl::accessor result(result_buffer, handler, sycl::write_only);

			handler.single_task<IbfKernel>([=]() [[intel::kernel_args_restrict]]
			{
				using QueryIndex = typename types::QueryIndex;
				using HostSizeType = typename types::HostSizeType;
				using Counter = typename types::Counter;
				using Chunk = typename types::Chunk;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					ibf_kernel_t::compute_ibf(
						ibfData,
						binSize,
						hashShift,
					[&](const QueryIndex localNumberOfHashes){
						//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
						// manual inline of getThreshold() because of thresholds accessor
						const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

						HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
						index = index < maximalIndex? index : maximalIndex;

						Counter threshold = (thresholds[static_cast<size_t>(index)] + 2).to_uint();
						return threshold;
					}, [&]() -> MinimizerToIBFData {
						return MinimizerToIBFPipes::PipeAt<id>::read();
					}, [&](size_t const chunkIndex, Chunk const & localResult) {
						size_t result_idx = (size_t)queryIndex * constants::chunks + chunkIndex;
						result[result_idx] = localResult;
					});
				}
			});
		});