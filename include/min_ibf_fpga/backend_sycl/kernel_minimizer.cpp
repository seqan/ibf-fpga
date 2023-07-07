		queue.submit([&](sycl::handler &handler)
		{
			sycl_minimizer_kernel_t minimizer_kernel{
				.queries{queries_buffer, handler, sycl::read_only},
				.queriesOffset{queriesOffset},
				.querySizes{querySizes_buffer, handler, sycl::read_only},
				.querySizesOffset{querySizesOffset},
				.numberOfQueries{numberOfQueries}
			};
			handler.single_task<MinimizerKernel>([minimizer_kernel, id]() [[intel::kernel_args_restrict]]
			{
				minimizer_kernel.execute<id>();
			});
		});